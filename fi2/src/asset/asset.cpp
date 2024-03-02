#include <filesystem>

#include "debug.h"
#include "system.h"
#include "notice.h"
#include "png.h"
#include "database/db.h"
#include <unordered_map>
#include "../../external/stb_image.h"
#include "png.h"
#include "propertytree.h"
#include "gltf.h"
#include <chrono>
#include "system/FileMonitoring.h"
#include "windows.h"
#include "nuggetgl/texture.h"

namespace nugget::asset {
    using namespace identifier;
    using namespace properties;
    namespace fs = std::filesystem;

    static const size_t MaxTextures = 100;
    std::unordered_map<IDType, TextureData> textures;
    std::unordered_map<IDType, ModelData> models;

    static volatile int32_t needToReconcile = 0;
    static int32_t lastReconciled = 0;

    struct AssetSystemConfig {
        APPLY_RULE_OF_MINUS_4(AssetSystemConfig);

        AssetSystemConfig() {}

        Notice::HotValue rootDir = { gProps, ID("assets.config.root") };
        Notice::HotValue textureDir = { gProps, ID("assets.config.textures") };
        Notice::HotValue modelDir = { gProps, ID("assets.config.models") };
    };

    const AssetSystemConfig& assetSystemConfig()
    {
        static AssetSystemConfig config;
        return config;
    }


    std::string MakeValidIdentifier(const std::string& input) {
        // Ensure the first character is a letter or underscore
        if (!isalpha(input[0]) && input[0] != '_') {
            // If not, prepend an underscore
            return '_' + input;
        }

        // Replace invalid characters with underscores followed by ASCII value in hex
        std::string result;
        for (char ch : input) {
            if (isalnum(ch) || ch == '_') {
                result += ch;
            } else {
                // Replace invalid character with underscore and ASCII value in hex
                result += std::format("_{:02X}", ch);
            }
        }

        return result;
    }

    void CollectFiles(const std::string& table, const fs::path& directory, bool withoutRoot = false) {
        for (const auto& entry : fs::recursive_directory_iterator(directory)) {
            if (fs::is_regular_file(entry.path())) {
                std::string ppath = entry.path().string();
                //                    output("{}\n", ppath);
                std::replace(ppath.begin(), ppath.end(), '\\', '/');
                std::string rpath = withoutRoot ? fs::relative(ppath, directory).string() : ppath;
                auto ftime = fs::last_write_time(entry.path().string());

                // Convert file_time_type to time_point of system_clock
                auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                    ftime - fs::file_time_type::clock::now() + std::chrono::system_clock::now()
                );
                // Now convert to UNIX timestamp using system_clock's to_time_t
                auto epoch = std::chrono::system_clock::to_time_t(sctp);

                db::AddAsset(table, rpath, MakeValidIdentifier(ppath), "", epoch);
            }
        }
    }
    
    bool IsTextureOutOfDate(IDType id) {
        return !textures.contains(id) || textures.at(id).outOfDate;
    }
    void MarkTextureAsOutOfDate(IDType id) {
        if (textures.contains(id)) {
            textures.at(id).outOfDate = true;
        }
    }

    void UpdateFileEpoch(const std::string& table, const std::string& path) {
        check(fs::is_regular_file(path), "The path {} ois not for a regular file", path);

        std::string ppath = path;
        std::replace(ppath.begin(), ppath.end(), '\\', '/');

        auto ftime = fs::last_write_time(path);

        // Convert file_time_type to time_point of system_clock
        auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
            ftime - fs::file_time_type::clock::now() + std::chrono::system_clock::now()
        );

        // Now convert to UNIX timestamp using system_clock's to_time_t
        auto epoch = std::chrono::system_clock::to_time_t(sctp);

        db::UpdateAssetEpoch(table, ppath, epoch);

    }

    void CollectAssetMetadata(IDType node) {
        db::StartTransaction();
        std::vector<IDType> children;
        gProps.GetChildrenWithNodeExisting(node, ID("path"), children);
        for (auto&& x : children) {
            IDType idPath = IDR(x, "path");
            IDType idType = IDR(x, "type");
            IDType idDescription = IDR(x, "description");
            const std::string& path = gProps.GetString(idPath);
            const std::string& type = gProps.GetString(idType);
            const std::string& description = gProps.GetString(idDescription);
            db::AddAssetMeta(IDToString(GetLeaf(x)), path, type, description);
        }
        db::CommitTransaction();
    }

    void ScanAssets(bool update = false) {
        std::string table = update ? "asset_changes" : "assets";

        db::StartTransaction();

        if (update) {
            db::ClearTable(table);
        }
        CollectFiles(table, static_cast<std::string>(assetSystemConfig().textureDir));
        CollectFiles(table, static_cast<std::string>(assetSystemConfig().modelDir));

        if (update) {
            db::ReconcileAssetChanges();
        }
        db::CommitTransaction();
    }

#if 0
    void ScanCache() {
        std::string table = "cache_info";

        db::StartTransaction();

        db::ClearTable(table);


        std::string cacheDir = gProps.GetString(ID("assets.config.cache"));

        if (!fs::exists(cacheDir)) {
            std::error_code ec; // For error handling without exceptions
            if (!fs::create_directories(cacheDir, ec)) { // Automatically creates all directories in the path
                check(0, "Could not create cache directory");
            }
        } else {
            check(fs::is_directory(cacheDir), "The cache directory is a file!?");
        }

        CollectFiles(table, cacheDir, true);

        db::CommitTransaction();
    }
#endif

    std::string GetCachePathForAsset(IDType id) {
        fs::path cacheDir = gProps.GetString(ID("assets.config.cache"));
        fs::path cachePath = cacheDir / IDToString(id);
        return cachePath.string();
    }

    void Init() {

        ScanAssets(); 

        system::files::Monitor(assetSystemConfig().rootDir, [](const std::string& path) {
            // called by different thread
            ScanAssets(true);
            needToReconcile++;
            });

        CollectAssetMetadata(ID("assets.meta"));

    }


    void Update() {
  //      static Notice::HotValue foo(gProps, ID("haha.hehe"));

    //    int64_t fooValue = foo.Get();

    //    output("@@@ {}\n", fooValue);

        if (needToReconcile > lastReconciled) {
            db::ReconcileInfo info;
             if (db::GetNextToReconcile(info /*fill*/)) {
                if (info.type == "modified") {
                    // update the timestamp on the file so that it will be marked dirty
                    UpdateFileEpoch("assets", info.path);
                    std::vector<IDType> list;
                    db::ReverseLookupAsset(info.path,list);
                    for (auto&& x : list) {
                        MarkTextureAsOutOfDate(x);
                    }
                    renderer::TexturesAreDirty();
//                        const TextureData& texture = GetTexture(info.node);
                } else {
                    check(0, "Unhandled change type: {}",info.type);
                }
                db::MarkReconciled(info.id);
            }
            lastReconciled = needToReconcile;
        }
    }

    void SerialiseTexture(const TextureData& data, const std::string& outPath) {
        if (auto file = std::basic_ofstream<unsigned char>(outPath, std::ios::binary | std::ios::trunc)) {
            check(file, "stream invaklid: {}", GetLastError());
            file.write((unsigned char*)&data, sizeof(data));
            check(file, "write failure: {}", GetLastError());
            file.write(data.data, data.DataSize());
            check(file, "write failure {}", GetLastError());
        } else {
            check(0, "could not open {} for writing", outPath);
        }
    }

    void DeserialiseTexture(TextureData& data, const std::string& inPath) {
        if (auto file = std::ifstream(inPath, std::ios::binary)) {
            file.read(reinterpret_cast<char*>(&data), sizeof(data));
            data.data = new unsigned char[data.DataSize()];
            file.read(reinterpret_cast<char*>(data.data), data.DataSize());
            data.fromCache = true;
        }
    }

    const TextureData& GetTexture(IDType id) {
        if (!IsTextureOutOfDate(id)) {
            return textures.at(id);
        } else {
            auto re = textures.emplace(std::piecewise_construct, std::forward_as_tuple(id), std::forward_as_tuple());
            TextureData& t = re.first->second;

            if (!db::IsAssetCacheDirty(id)) {
                output("@@@@@@@@@@ USING CACHE!\n");
                // deserialise binary texture
                DeserialiseTexture(t, GetCachePathForAsset(id));
                return t;
            } else {
                // serialise

                std::string path;
                bool r = db::LookupAsset(id, path);
                if (!r) {
                    check(0, "No information available on asset '{}'\n", IDToString(id));
                    return t;
                }

                bool rl = LoadPNG(path, t);

                SerialiseTexture(t, GetCachePathForAsset(id));

                // path text,name text, type text, hash int64, epoch big integer
                db::AddAssetCache(path, IDToString(id), "texture", id);

                assert(rl);
                return t;
            }
        }
    }

    const ModelData& GetModel(IDType id) {
        static ModelData nullData = {};
        if (models.contains(id)) {
            return models.at(id);
        } else {
            auto re = models.emplace(std::piecewise_construct, std::forward_as_tuple(id), std::forward_as_tuple());
            ModelData& t = re.first->second;
            // look up the texture

            std::string path;
            bool r = db::LookupAsset(id, path /*fill*/);
            if (!r) {
                check(0, "No information available on asset '{}'\n", IDToString(id));
                return nullData;
            }
            output("Model Load...\n");
            bool rl = gltf::LoadModel(path, t);
            output("...ed\n");
            assert(rl);
            return t;
        }
    }

    TextureData::~TextureData() {
        if (fromCache) {
            delete data;
        } else {
            if (data) {
                stbi_image_free(data);
            }
        }
    }

    ModelData::~ModelData() {
        assert(0);   // check if we have to do somethin here?
    }

    static size_t init_dummy[] =
    {
        {
            nugget::system::RegisterModule([]() {
                Init();
                return 0;
            }, 200,identifier::ID("init"),__FILE__)
        },
        {
            // frame begin
            nugget::system::RegisterModule([]() {
                Update();
                return 0;
            }, 200, ID("update"))
        },
    };

}


