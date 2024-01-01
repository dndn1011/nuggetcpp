#include <filesystem>

#include "debug.h"
#include "system.h"
#include "notice.h"
#include "png.h"
#include "database/db.h"
#include <unordered_map>
#include "../../external/stb_image.h"

namespace nugget::asset {
    using namespace identifier;
    namespace fs = std::filesystem;

    static const size_t MaxTextures = 100;
    std::unordered_map<IDType,TextureData> textures;

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
    

    void CollectFiles(const fs::path& directory) {
        for (const auto& entry : fs::recursive_directory_iterator(directory)) {
            if (fs::is_regular_file(entry.path())) {
                std::string ppath = entry.path().string();
                    std::replace(ppath.begin(), ppath.end(), '\\', '/');
                    output("{}\n", ppath);
                db::AddAsset(ppath, MakeValidIdentifier(ppath), "texture");
            }
        }
    }

    void CollectAssetMetadata(IDType node) {
        std::vector<IDType> children;
        Notice::GetChildrenWithNodeExisting(node, ID("path"), children);
        for (auto&& x : children) {
            IDType idPath = IDR(x, "path");
            IDType idType = IDR(x, "type");
            IDType idDescription = IDR(x, "description");
            const std::string &path = Notice::GetString(idPath);
            const std::string &type = Notice::GetString(idType);
            const std::string &description = Notice::GetString(idDescription);
            db::AddAssetMeta(IDToString(GetLeaf(x)), path, type, description);
        }
    }

    void Init() {
        std::string textureDir = Notice::GetString(ID("properties.assets.config.textures"));
        CollectFiles(textureDir);
        CollectAssetMetadata(ID("properties.assets.meta"));
    }



    size_t init_dummy = nugget::system::RegisterModule([]() {
        Init();
        return 0;
        },200);


    const TextureData& GetTexture(IDType id) {
        if (textures.contains(id)) {
            return textures.at(id);
        } else {
            auto r = textures.emplace(id, TextureData{});
            TextureData& t = r.first->second;
//            png = new PNGImage();
//            bool r = LoadPNG(entry.path().string(), *png);
//            assert(r);  
        }
    }

    TextureData::~TextureData() {
        if (data) {
            stbi_image_free(data);
        }
    }


}