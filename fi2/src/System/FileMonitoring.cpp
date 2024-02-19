#include <windows.h>
#include <iostream>
#include <vector>
#include <thread>
#include <functional>
#include <string>
#include "system.h"
#include "debug.h"
#include "utils/utils.h"

namespace nugget::system::files {

    struct WideString {
        APPLY_RULE_OF_MINUS_5(WideString);
        WideString(const std::string& input) {
            int bufferSize = MultiByteToWideChar(CP_UTF8, 0, input.c_str(), -1, nullptr, 0);
            wstr = new wchar_t[bufferSize];
            MultiByteToWideChar(CP_UTF8, 0, input.c_str(), -1, wstr, bufferSize);
        }
        LPCWSTR GetWStr() const {
            return wstr;
        }
        ~WideString() {
            delete[] wstr;
        }
    private:
        LPWSTR wstr = nullptr;
    };

    std::string WStringToString(const std::wstring& wstr)
    {
        if (wstr.empty()) return std::string();

        int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
        std::string strTo(sizeNeeded, 0);
        WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], sizeNeeded, NULL, NULL);
        return strTo;
    }

    void ProcessChanges(const char* buffer, DWORD bytesReturned, const std::wstring& directoryPath, std::function<void(std::string)> callback) {
        auto notifyInfo = reinterpret_cast<const FILE_NOTIFY_INFORMATION*>(buffer);
        do {
            // Calculate the length of the file name
            DWORD nameLength = notifyInfo->FileNameLength / sizeof(WCHAR);
            // Construct a wstring from the FileName member
            std::wstring fileName(notifyInfo->FileName, nameLength);
            // Create the full path for the changed file
            std::wstring fullPath = directoryPath + L"\\" + fileName;

            // Invoke the callback with the full path of the changed file
            callback(WStringToString(fullPath));

            // Move to the next record if available
            if (notifyInfo->NextEntryOffset == 0) break;
            notifyInfo = reinterpret_cast<const FILE_NOTIFY_INFORMATION*>(
                reinterpret_cast<const char*>(notifyInfo) + notifyInfo->NextEntryOffset);
        } while (true);
    }
        

    void MonitorDirectory(std::stop_token stopToken,std::string directoryPath, std::function<void(std::string)> callback) {
        LPCSTR directoryToMonitor = directoryPath.c_str();

        WideString widePath(directoryPath);

        HANDLE dirHandle = CreateFileW(
            widePath.GetWStr(),
            FILE_LIST_DIRECTORY,
            FILE_SHARE_WRITE | FILE_SHARE_READ | FILE_SHARE_DELETE,
            NULL,
            OPEN_EXISTING,
            FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
            NULL
        );
        
        check(dirHandle != INVALID_HANDLE_VALUE, "Failed to open directory handle.");

        char buffer[1024];
        DWORD bytesReturned;
        OVERLAPPED overlapped = {};
        overlapped.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

        while (true) {
            BOOL result = ReadDirectoryChangesW(
                dirHandle,
                &buffer,
                sizeof(buffer),
                TRUE, // Monitor the directory and subdirectories
                FILE_NOTIFY_CHANGE_LAST_WRITE,
                &bytesReturned,
                &overlapped, // Using overlapped I/O
                NULL
            );

            auto r = WaitForSingleObject(overlapped.hEvent, 500);
            if (stopToken.stop_requested()) {
                break;
            }
            if (WAIT_OBJECT_0 == r) {
                if (result) {
                    ProcessChanges(buffer, bytesReturned, widePath.GetWStr(), callback);
                } else {
                    // An error occurred
                    std::cerr << "Failed to monitor directory changes." << std::endl;
                    break;
                }
            }
        }

        CloseHandle(dirHandle);
    }

    struct MonitorInfo {
        std::jthread thread;
        std::string path;
        std::function<void(const std::string&)> func;
    };

    std::vector<MonitorInfo> monitors;

    void Monitor(const std::string &path, std::function<void(const std::string &)> func) {
        monitors.emplace_back(
            MonitorInfo{
                std::jthread(MonitorDirectory, path, func),
                path,
                func
            });
     
    }

    void Exit() {
        for (auto&& x : monitors) {
            x.thread.request_stop();
        }
    }

    static size_t init_dummy[] =
    {
        {
            // frame begin
            nugget::system::RegisterModule([]() {
                Exit();
                return 0;
            }, 200, ID("shutdown"))
        },
    };

}
