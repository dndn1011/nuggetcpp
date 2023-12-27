#include <filesystem>

#include "debug.h"
#include "system.h"
#include "notice.h"
#include "png.h"

namespace fs = std::filesystem;

namespace nugget::asset {
    using namespace identifier;

    void CollectFiles(const fs::path& directory) {
        for (const auto& entry : fs::recursive_directory_iterator(directory)) {
            if (fs::is_regular_file(entry.path())) {
                output("{}\n", entry.path().string());
                PNGImage png;
                bool r = LoadPNG(entry.path().string(), png);

            }
        }
    }

    void Init() {
        std::string textureDir = Notice::GetString(ID("properties.assets.textures"));
        CollectFiles(textureDir);
    }



    size_t init_dummy = nugget::system::RegisterModule([]() {
        Init();
        return 0;
        },100);


}