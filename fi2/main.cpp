#include <iostream>
#include <thread>
#include <chrono>

#include "src/propertytree.h"
#include "src/UIEntities.h"
#include "src/identifier.h"
#include "src/notice.h"
#include "src/ui.h"

#include <filesystem>

#include <array>

#include "src/debug.h"
#include "nuggetgl/gl.h"

#include "system.h"

using namespace nugget;
using namespace nugget::identifier;

struct FileWatcher {
    FileWatcher(const std::string& filenameIn) : filename(filenameIn) {
        mtime = std::filesystem::last_write_time(filename);
    }

    bool Check() {
        try {
            auto newtime = std::filesystem::last_write_time(filename);
            if (newtime != mtime) {
                mtime = newtime;
                return true;
            } else {
                return false;
            }
        }
        catch (const std::exception& e) {
            (void)e;
            return false;
        }
    }

    std::string filename;
    std::filesystem::file_time_type mtime;
};

void ReloadPt(std::string filename) {
    //            Notice::LockNotifications();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    Notice::Remove(IDR("properties2"));
    auto result = properties::LoadPropertyTree("properties2", filename);
    if (result.successful) {
        nugget::ui::entity::UpdateEntity(IDR("properties"), IDR("properties2"));
        nugget::ui::entity::CreateEntity(IDR("properties.main"));
        nugget::ui::entity::ManageGeometry(IDR("properties.main"));
    } else {
        auto absfilename = std::filesystem::absolute(filename);

        output("Parse state:\n{}({}): {}\n", absfilename.string(), result.lineNumber, result.description);
        output("Could not load propertries for {}: {}\n", filename.c_str(), result.description.c_str());
    }
    //            Notice::UnlockNotifications();
}


static const std::string filename = "config.pt";
int main(int argc, char* argv[]) {

    std::string file = filename;
    if (argc > 1) {
        file = std::string(argv[1]);
    }
    for (int i = 0; i<1; i++) {
        auto result = properties::LoadPropertyTree("properties",file);
        if (!result.successful) {
            output("Could not load propertries for {}:\n", file);
            auto absfilename = std::filesystem::absolute(file);

            outputAlways("Parse state:\n{}({}): {}\n", absfilename.string(), result.lineNumber, result.description);
            return 1;
        }
    }   

    nugget::system::Init();

//    auto v = Notice::GetValueAny(IDR("properties.test.value"));
//    output("RESULT: {} : {}\n", v.GetTypeAsString(), v.GetValueAsString());

//    PrintHashTree(IDR("properties"));

#if 0
    auto a = nugget::Notice::GetString(IDR("properties.test.a"));
    auto b = nugget::Notice::GetInt64(IDR("properties.test.b"));
    auto c = nugget::Notice::GetFloat(IDR("properties.test.c"));
    auto d = nugget::Notice::GetValueAsString(IDR("properties.test.d"));

    output("a = %s\n", a.c_str());
    output("b = %lld\n", b);
    output("c = %f\n", c);
    output("d = %s\n", d.c_str());
#endif

    nugget::ui::entity::CreateEntity(IDR("properties.main"));
    nugget::ui::entity::ManageGeometry(IDR("properties.main"));

//    nugget::ui::entity::CreateEntity(ID("properties.circle"));

    FileWatcher watcher(filename);

#if 0 // test of button events
    auto updateLambda = [](IDType changeId) {
        int32_t count = Notice::GetInt32(changeId);
        Notice::Set(IDR(GetParent(changeId), ID("text")), std::to_string(count));
        };

    std::vector<IDType> children;
    Notice::GetChildrenWithNodeOfValue(ID("properties.main.sub"), ID("class"), ID("ui::Button"), children/*fill*/);
    for (auto& x : children) {
        Notice::RegisterHandler(Notice::Handler{
            IDR(x,ID("pressEventCount")),
            updateLambda
            });
    }
#endif

#if 0
    nugget::ui::entity::RegisterFunction(IDR("ExitClick"), [](IDType id) {
        exit(0);
        });
#endif

    nugget::ui::Init();


    nugget::ui::Exec([&watcher]() {
        if (watcher.Check()) {
            ReloadPt(filename);
        }
        });

    getchar();

    return 0;
}

