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

namespace nugget::asset {
    using namespace identifier;
    using namespace properties;
    namespace fs = std::filesystem;

    static const size_t MaxTextures = 100;
    std::unordered_map<IDType, TextureData> textures;
    std::unordered_map<IDType, ModelData> models;

    static volatile int32_t needToReconcile = 0;
    static int32_t lastReconciled = 0;

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
    

    void CollectFiles(const std::string &table,const fs::path& directory) {
        for (const auto& entry : fs::recursive_directory_iterator(directory)) {
            if (fs::is_regular_file(entry.path())) {
                std::string ppath = entry.path().string();
                    std::replace(ppath.begin(), ppath.end(), '\\', '/');
//                    output("{}\n", ppath);
                    auto ftime = fs::last_write_time(entry.path().string());

                    // Convert file_time_type to time_point of system_clock
                    auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                        ftime - fs::file_time_type::clock::now() + std::chrono::system_clock::now()
                    );
                    // Now convert to UNIX timestamp using system_clock's to_time_t
                    auto epoch = std::chrono::system_clock::to_time_t(sctp);

                    db::AddAsset(table, ppath, MakeValidIdentifier(ppath), "", epoch);
            }
        }
    }

    void CollectAssetMetadata(IDType node) {
        std::vector<IDType> children;
        gProps.GetChildrenWithNodeExisting(node, ID("path"), children);
        for (auto&& x : children) {
            IDType idPath = IDR(x, "path");
            IDType idType = IDR(x, "type");
            IDType idDescription = IDR(x, "description");
            const std::string &path = gProps.GetString(idPath);
            const std::string &type = gProps.GetString(idType);
            const std::string &description = gProps.GetString(idDescription);
            db::AddAssetMeta(IDToString(GetLeaf(x)), path, type, description);
        }
    }

    void ScanAssets(bool update=false) {
        std::string table = update ? "asset_changes" : "assets";
        
        db::StartTransaction();

        if (update) {
            db::ClearTable(table);
        }
        std::string textureDir = gProps.GetString(ID("assets.config.textures"));
        CollectFiles(table,textureDir);
        std::string modelDir = gProps.GetString(ID("assets.config.models"));
        CollectFiles(table,modelDir);

        if (update) {
            db::ReconcileAssetChanges();
        }
        db::CommitTransaction();
    }

    void Init() {
        std::string root = gProps.GetString(ID("assets.config.root"));
        ScanAssets();
     
        system::files::Monitor(root, [](const std::string& path) {
            ScanAssets(true);
            needToReconcile++;
            });

        CollectAssetMetadata(ID("assets.meta"));

    }

    void Update() {
        if (needToReconcile > lastReconciled) {
            db::ReconcileInfo info;
            if (db::GetNextToReconcile(info /*fill*/)) {
                if (info.type == "modified") {
                    output("@@@ {} {} \n", info.path, info.type);
                }
                db::MarkReconciled(info.id);
            } else {
                lastReconciled = needToReconcile;
            }
        }
    }

    const TextureData& GetTexture(IDType id) {
        if (textures.contains(id)) {
            return textures.at(id);
        } else {
            auto re = textures.emplace(std::piecewise_construct, std::forward_as_tuple(id), std::forward_as_tuple());
            TextureData& t = re.first->second;
            // look up the texture

            std::string path;
            bool r = db::LookupAsset(id, path);
            if (!r) {
                check(0, "No information available on asset '{}'\n", IDToString(id));
                return t;
            }
            bool rl = LoadPNG(path, t);
            assert(rl);  
            return t;
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
            bool rl = gltf::LoadModel(path, t);
            assert(rl);
            return t;
        }
    }

    TextureData::~TextureData() {
        if (data) {
            stbi_image_free(data);
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
            }, 200)
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


