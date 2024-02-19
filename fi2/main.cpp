#define WIN32_LEAN_AND_MEAN
#include <windows.h>

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
    static Notice::Board gPropsBack;
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
    
    gPropsBack.Clear();

    auto result = properties::LoadPropertyTree(gPropsBack, filename);
    if (result.successful) {
        nugget::ui::entity::UpdateEntity(gProps, gPropsBack);
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

    IDType commandLineHash = IDR("commandLine");
    IDType rawHash = IDR(commandLineHash, "raw");
    IDType assignmentHash = IDR(commandLineHash, "assignments");
    Notice::gBoard.SetAsParent(commandLineHash);
    Notice::gBoard.SetAsParent(rawHash);
    Notice::gBoard.SetAsParent(assignmentHash);
    for (int i = 0; i < argc; ++i) {
        std::string arg(argv[i]);
        Notice::gBoard.Set(IDR(rawHash, std::to_string(i)), arg);
        size_t pos = arg.find('=');
        if (pos != std::string::npos) {
            std::string l = arg.substr(0, pos);
            std::string r = arg.substr(pos + 1);
            Notice::gBoard.Set(IDR(assignmentHash, l), r);
        }
    }
    IDType exitAfterTestsOptionKey = ID("commandLine.assignments.ptfile");
    if (Notice::gBoard.KeyExists(exitAfterTestsOptionKey)) {
        file = Notice::gBoard.GetString(exitAfterTestsOptionKey);
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

    if (Notice::gBoard.KeyExists(ID("exit"))) {
        int32_t exitCode = Notice::gBoard.GetInt32(ID("exit"));
        system::Shutdown();
        exit(exitCode);
    }

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

    // Main loop        
    MSG msg = {};
    while (true) {
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                return 0;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        if (!nugget::system::Update()) {
            // nothing to update, so just exit now
            return 0;
        }

        if (watcher.Check()) {
            ReloadPt(filename);
        }

    }

    getchar();  

    return 0;
}

