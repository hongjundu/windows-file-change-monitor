// main.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <stdio.h>
#include <assert.h>
#include <tchar.h>
#include <Windows.h>

void WatchDirectory(LPTSTR lpDir);

DWORD WINAPI WatchDirectoryThread(LPVOID lpParam) {
    LPTSTR lpDir = (LPTSTR)lpParam;
    WatchDirectory(lpDir);
    return 0;
}

int main() {
    const wchar_t* path = _T("D:\\tmp");
    printf("watching %s for changes...\n", path);

    HANDLE hThread = CreateThread(
        NULL,                   // default security attributes
        0,                      // use default stack size  
        WatchDirectoryThread,   // thread function name
        (LPVOID)path,           // argument to thread function 
        0,                      // use default creation flags 
        NULL);                  // returns the thread identifier 

    // Check the return value for success.
    if (hThread == NULL) {
        printf("CreateThread failed, error: %d\n", GetLastError());
        return 1;
    }

    // Wait for the thread to finish.
    WaitForSingleObject(hThread, INFINITE);

    // Close thread handle.
    CloseHandle(hThread);

    return 0;
}

void WatchDirectory(LPTSTR lpDir) {
    HANDLE file = CreateFile(lpDir,
        FILE_LIST_DIRECTORY,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
        NULL);
    assert(file != INVALID_HANDLE_VALUE);

    OVERLAPPED overlapped = { 0 };
    overlapped.hEvent = CreateEvent(NULL, FALSE, 0, NULL);

    uint8_t change_buf[1024];
    BOOL success = ReadDirectoryChangesW(
        file, change_buf, 1024, TRUE,
        FILE_NOTIFY_CHANGE_FILE_NAME |
        FILE_NOTIFY_CHANGE_DIR_NAME |
        FILE_NOTIFY_CHANGE_LAST_WRITE,
        NULL, &overlapped, NULL);

    while (true) {
        DWORD result = WaitForSingleObject(overlapped.hEvent, INFINITE);

        if (result == WAIT_OBJECT_0) {
            DWORD bytes_transferred;
            GetOverlappedResult(file, &overlapped, &bytes_transferred, FALSE);

            FILE_NOTIFY_INFORMATION* event = (FILE_NOTIFY_INFORMATION*)change_buf;

            for (;;) {
                DWORD name_len = event->FileNameLength / sizeof(wchar_t);

                switch (event->Action) {
                case FILE_ACTION_ADDED: {
                    wprintf(L"       Added: %.*s\n", name_len, event->FileName);
                } break;

                case FILE_ACTION_REMOVED: {
                    wprintf(L"     Removed: %.*s\n", name_len, event->FileName);
                } break;

                case FILE_ACTION_MODIFIED: {
                    wprintf(L"    Modified: %.*s\n", name_len, event->FileName);
                } break;

                case FILE_ACTION_RENAMED_OLD_NAME: {
                    wprintf(L"Renamed from: %.*s\n", name_len, event->FileName);
                } break;

                case FILE_ACTION_RENAMED_NEW_NAME: {
                    wprintf(L"          to: %.*s\n", name_len, event->FileName);
                } break;

                default: {
                    printf("Unknown action!\n");
                } break;
                }

                if (event->NextEntryOffset) {
                    *((uint8_t**)&event) += event->NextEntryOffset;
                }
                else {
                    break;
                }
            }

            success = ReadDirectoryChangesW(
                file, change_buf, 1024, TRUE,
                FILE_NOTIFY_CHANGE_FILE_NAME |
                FILE_NOTIFY_CHANGE_DIR_NAME |
                FILE_NOTIFY_CHANGE_LAST_WRITE,
                NULL, &overlapped, NULL);
        }
    }
}



/*
int main() {
    const wchar_t *path = _T("D:\\tmp");
    printf("watching %s for changes...\n", path);

    HANDLE file = CreateFile(path,
        FILE_LIST_DIRECTORY,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
        NULL);
    assert(file != INVALID_HANDLE_VALUE);
    OVERLAPPED overlapped;
    overlapped.hEvent = CreateEvent(NULL, FALSE, 0, NULL);

    uint8_t change_buf[1024];
    BOOL success = ReadDirectoryChangesW(
        file, change_buf, 1024, TRUE,
        FILE_NOTIFY_CHANGE_FILE_NAME |
        FILE_NOTIFY_CHANGE_DIR_NAME |
        FILE_NOTIFY_CHANGE_LAST_WRITE,
        NULL, &overlapped, NULL);

    while (true) {
        DWORD result = WaitForSingleObject(overlapped.hEvent, 0);

        if (result == WAIT_OBJECT_0) {
            DWORD bytes_transferred;
            GetOverlappedResult(file, &overlapped, &bytes_transferred, FALSE);

            FILE_NOTIFY_INFORMATION* event = (FILE_NOTIFY_INFORMATION*)change_buf;

            for (;;) {
                DWORD name_len = event->FileNameLength / sizeof(wchar_t);

                switch (event->Action) {
                case FILE_ACTION_ADDED: {
                    wprintf(L"       Added: %.*s\n", name_len, event->FileName);
                } break;

                case FILE_ACTION_REMOVED: {
                    wprintf(L"     Removed: %.*s\n", name_len, event->FileName);
                } break;

                case FILE_ACTION_MODIFIED: {
                    wprintf(L"    Modified: %.*s\n", name_len, event->FileName);
                } break;

                case FILE_ACTION_RENAMED_OLD_NAME: {
                    wprintf(L"Renamed from: %.*s\n", name_len, event->FileName);
                } break;

                case FILE_ACTION_RENAMED_NEW_NAME: {
                    wprintf(L"          to: %.*s\n", name_len, event->FileName);
                } break;

                default: {
                    printf("Unknown action!\n");
                } break;
                }

                // Are there more events to handle?
                if (event->NextEntryOffset) {
                    *((uint8_t**)&event) += event->NextEntryOffset;
                }
                else {
                    break;
                }
            }

            // Queue the next event
            BOOL success = ReadDirectoryChangesW(
                file, change_buf, 1024, TRUE,
                FILE_NOTIFY_CHANGE_FILE_NAME |
                FILE_NOTIFY_CHANGE_DIR_NAME |
                FILE_NOTIFY_CHANGE_LAST_WRITE,
                NULL, &overlapped, NULL);

        }

        // Do other loop stuff here...
    }
}

*/
