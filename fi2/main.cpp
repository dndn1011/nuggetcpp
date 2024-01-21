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
using namespace properties;

namespace nugget::properties {
    Notice::Board gProps;
    static Notice::Board gNoticeBack;
}

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
    //            gNotice.LockNotifications();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    gNoticeBack.Clear();

    auto result = properties::LoadPropertyTree(gNoticeBack, filename);
    if (result.successful) {
        nugget::ui::entity::UpdateEntity(gProps, gNoticeBack);
        nugget::ui::entity::CreateEntity(gProps,IDR("main"));
        nugget::ui::entity::ManageGeometry(gProps, IDR("main"));
    } else {
        auto absfilename = std::filesystem::absolute(filename);

        output("Parse state:\n{}({}): {}\n", absfilename.string(), result.lineNumber, result.description);
        output("Could not load propertries for {}: {}\n", filename.c_str(), result.description.c_str());
    }
    //            gNotice.UnlockNotifications();
}


static const std::string filename = "config.pt";
int main(int argc, char* argv[]) {

    std::string file = filename;
    if (argc > 1) {
        file = std::string(argv[1]);
    }
    for (int i = 0; i<1; i++) {
        auto result = properties::LoadPropertyTree(gProps /*fill*/,file);
        if (!result.successful) {
            output("Could not load propertries for {}:\n", file);
            auto absfilename = std::filesystem::absolute(file);

            outputAlways("Parse state:\n{}({}): {}\n", absfilename.string(), result.lineNumber, result.description);
            return 1;
        }

    }   

    auto result = gProps.GetVector4f(ID("result"));


    nugget::system::Init();

    nugget::ui::entity::CreateEntity(gProps, IDR("main"));
    nugget::ui::entity::ManageGeometry(gProps, IDR("main"));

//    nugget::ui::entity::CreateEntity(ID("circle"));

    FileWatcher watcher(filename);

#if 0 // test of button events
    auto updateLambda = [](IDType changeId) {
        int32_t count = gProps.GetInt32(changeId);
        gProps.Set(IDR(GetParent(changeId), ID("text")), std::to_string(count));
        };

    std::vector<IDType> children;
    gProps.GetChildrenWithNodeOfValue(ID("main.sub"), ID("class"), ID("ui::Button"), children/*fill*/);
    for (auto& x : children) {
        gProps.RegisterHandler(gProps.Handler{
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

