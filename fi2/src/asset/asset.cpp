#include <filesystem>

#include "debug.h"
#include "system.h"
#include "notice.h"
#include "png.h"
#include "database/db.h"

namespace fs = std::filesystem;

namespace nugget::asset {
    using namespace identifier;

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
                output("{}\n", entry.path().string());
                PNGImage* png = new PNGImage();
                bool r = LoadPNG(entry.path().string(), *png);
                assert(r);
                db::AddAsset(entry.path().string(), MakeValidIdentifier(entry.path().string()), "texture");
            }
        }
    }

    void Init() {
        std::string textureDir = Notice::GetString(ID("properties.assets.config.textures"));
        CollectFiles(textureDir);
    }



    size_t init_dummy = nugget::system::RegisterModule([]() {
        Init();
        return 0;
        },200);


}