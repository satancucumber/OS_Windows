#include <iostream>
#include <fileapi.h>
#include <windows.h>
#include <list>
#include <string>
#include <iomanip>
#include <conio.h>

using namespace std;

void DiskList(list<string> *disksList) {
    int n;
    string diskName;

    disksList->clear();

    DWORD dr = GetLogicalDrives();

    for (int i = 0; i < 32; i++) {
        n = ((dr>>i)&0x00000001);
        if( n == 1 )
        {
            diskName = char(65+i);
            diskName = diskName + ":";
            diskName = diskName + "\\";
            disksList->push_back(diskName);
        }
    }
}

string diskInf(list<string> *disksList) {
    string disk;
    TCHAR volumeName[MAX_PATH + 1] = { 0 };
    TCHAR fileSystemName[MAX_PATH + 1] = { 0 };
    DWORD serialNumber = 0;
    DWORD maxComponentLen = 0;
    DWORD fileSystemFlags = 0;
    DWORD SectorsPerCluster;
    DWORD BytesPerSector;
    DWORD NumberOfFreeClusters;
    DWORD TotalNumberOfClusters;
    string inf;
    DiskList(disksList);
    int k = 0;
    int index;
    cout << "Disks list:\n";
    for(list<string>::iterator it = disksList->begin(); it != disksList->end(); it++)
        cout << "'" << ++k << "' - " <<*it<<"\n";
    cout << "Choose the disk: ";
    cin >> index;
    k = 0;
    for(list<string>::iterator it = disksList->begin(); it != disksList->end(); it++)
        if (index == ++k) {
            disk = *it;
        }
    cout << "Information:\n";
    inf = inf + "- Drive type: ";
    switch (GetDriveType(disk.c_str())) {
            case 0 :
                inf = inf + "cannot be determined.";
                break;
            case 1:
                inf = inf +"the root path is invalid.";
                break;
            case 2:
                inf = inf +"removable media.";
                break;
            case 3:
                inf = inf +"fixed media.";
                break;
            case 4:
                inf = inf +"remote (network).";
                break;
            case 5:
                inf = inf +"CD-ROM.";
                break;
            case 6:
                inf = inf +"RAM disk.";
                break;
    }
    GetVolumeInformationA(
            disk.c_str(),
            volumeName,
            ARRAYSIZE(volumeName),
            &serialNumber,
            &maxComponentLen,
            &fileSystemFlags,
            fileSystemName,
            ARRAYSIZE(fileSystemName));

    inf = inf + "\n"
                "- Volume name: " +
          volumeName + "\n"
                       "- Serial number: " +
          to_string(serialNumber) + "\n"
                         "- Maximum component lenght: " +
          to_string(maxComponentLen) + "\n"
                            "- File system name: " +
          fileSystemName + "\n"
                           "- File system flags: ";

    struct FlagDefine {
        DWORD flag;
        const char* description;
    };

    static const FlagDefine file_sys_flags[] = {
            { FILE_CASE_SENSITIVE_SEARCH, "Case sensitive search" },
            { FILE_CASE_PRESERVED_NAMES, "Case preserved names" },
            { FILE_UNICODE_ON_DISK, "Unicode on disk" },
            { FILE_PERSISTENT_ACLS, "Persistent ACLs" },
            { FILE_FILE_COMPRESSION, "File compression" },
            { FILE_VOLUME_QUOTAS, "Volume quotas" },
            { FILE_SUPPORTS_SPARSE_FILES, "Supports sparse files" },
            { FILE_SUPPORTS_REPARSE_POINTS, "Supports reparse points" },
            { FILE_SUPPORTS_REMOTE_STORAGE, "Supports remote storage" },
            { FILE_VOLUME_IS_COMPRESSED, "Volume is compressed" },
            { FILE_SUPPORTS_OBJECT_IDS, "Supports object IDs" },
            { FILE_SUPPORTS_ENCRYPTION, "Supports encryption" },
            { FILE_NAMED_STREAMS, "Named streams" },
            { FILE_READ_ONLY_VOLUME, "Read-only volume" },
            { FILE_SEQUENTIAL_WRITE_ONCE, "Sequential write once" },
            { FILE_SUPPORTS_TRANSACTIONS, "Supports transactions" },
    };

    for (const auto& flag_def : file_sys_flags) {
        if (fileSystemFlags & flag_def.flag) {
            inf = inf + "\n  - " + flag_def.description;
        }
    }

    GetDiskFreeSpaceA(disk.c_str(),&SectorsPerCluster, &BytesPerSector, &NumberOfFreeClusters, &TotalNumberOfClusters);

    unsigned int freeSize;
    unsigned int fullSize;

    unsigned int clus_size = SectorsPerCluster * BytesPerSector / 1024;

    freeSize = NumberOfFreeClusters * clus_size / 1024;
    fullSize = TotalNumberOfClusters * clus_size / 1024;

    inf = inf + "\n"
                "- Free size: " +
                to_string(freeSize) + " Mb\n"
                           "- Full size: " +
                           to_string(fullSize) + " Mb\n";
    return inf;
}

void printLastErrorMessage() {
    DWORD error_code = GetLastError();

    if (error_code == 0) {
        std::wcout << L"No error." << std::endl;
        return;
    }

    LPWSTR message_buffer;
    DWORD buffer_length = FormatMessageW(
            FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS,
            nullptr,
            error_code,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPWSTR)&message_buffer,
            0,
            nullptr
    );

    if (buffer_length == 0) {
        std::wcout << L"Failed to retrieve the error message." << std::endl;
        return;
    }

    HANDLE console_output = GetStdHandle(STD_OUTPUT_HANDLE);

    if (console_output == INVALID_HANDLE_VALUE) {
        std::wcout << L"Failed to get console output handle." << std::endl;
        LocalFree(message_buffer);
        return;
    }

    DWORD written;
    WriteConsoleW(console_output, L"Error message: ", 15, &written, nullptr);
    WriteConsoleW(console_output, message_buffer, buffer_length, &written, nullptr);

    LocalFree(message_buffer);
}

void createDirect() {
    DWORD lastError;
    string direct;
    cout << "Create directory\n";
    cout << "Enter directory (D:\\folder): ";
    cin >> direct;
    if (CreateDirectoryA(direct.c_str(), NULL)){
        cout << "Directory was created successfully!\n";
    }
    else {
        printLastErrorMessage();
    }
}

void deleteDirect() {
    DWORD lastError;
    string direct;
    cout << "Remove directory\n";
    cout << "Enter directory (D:\\folder): ";
    cin >> direct;
    if (RemoveDirectoryA(direct.c_str())){
        cout << "Directory was deleted successfully!\n";
    }
    else {
        printLastErrorMessage();
    }
}

void createFile() {
    string file;
    cout << "Create file\n";
    cout << "Enter file path (D:\\folder\\file.txt): ";
    cin >> file;
    if (CreateFileA(file.c_str(),GENERIC_READ | GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,NULL)) {
        cout << "File was create successfully!";
    }
    else {
        printLastErrorMessage();
    }
}

void moveFiles() {
    string from;
    string where;
    cout << "Moving file\n";
    cout << "Enter moved file (D:\\folder\\file.txt): ";
    cin >> from;
    cout << "Enter new file name (D:\\folder\\file.txt): ";
    cin >> where;
    if (from == where) {
        cout << "Attempt to move file in the same place\n";
    } else {
        BOOL success = MoveFileA(from.c_str(), where.c_str());
        if (!success) {
            printLastErrorMessage();
        } else {
            cout << "File moved successfully!\n";
        }
    }
}

void copyFiles() {
    string from;
    string where;
    cout << "Copy file\n";
    cout << "Enter copied file (D:\\folder\\file.txt): ";
    cin >> from;
    cout << "Enter new file (D:\\folder\\file1.txt): ";
    cin >> where;
    if ( from == where) {
        cout << "Attempt to copy file in the same place\n";
    } else {
        BOOL success = CopyFileA(from.c_str(), where.c_str(), true);
        if (!success) {
            printLastErrorMessage();
        } else {
            cout << "File copied successfully!\n";
        }
    }
}

void print_filetime_in_DDMMYYYY(const FILETIME &filetime) {
    SYSTEMTIME systemTime;

    // Convert the FILETIME to SYSTEMTIME
    FileTimeToSystemTime(&filetime, &systemTime);

    // Print the date in the DDMMYYYY format
    cout << setw(2) << setfill('0') << systemTime.wDay
              << "/" << setw(2) << setfill('0') << systemTime.wMonth
              << "/" << systemTime.wYear << endl;
}

void getHandleFileAttributes(string file) {
    HANDLE fileHandle = CreateFile(
            file.c_str(),
            GENERIC_READ,
            FILE_SHARE_READ,
            nullptr,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            nullptr
    );

    if(fileHandle == INVALID_HANDLE_VALUE) {
        printLastErrorMessage();
    }

    // Call GetFileInformationByHandle
    BY_HANDLE_FILE_INFORMATION fileInfo;
    BOOL success = GetFileInformationByHandle(fileHandle, &fileInfo);

    if (!success) {
        printLastErrorMessage();
        CloseHandle(fileHandle);
    }

    // Print the retrieved file information
    cout << "File information:" << "\n";
    cout << "- File size: " << ((static_cast<__int64>(fileInfo.nFileSizeHigh) << 32)
                                     + fileInfo.nFileSizeLow) << " bytes\n";
    cout << "- Creation time: ";
    print_filetime_in_DDMMYYYY(fileInfo.ftCreationTime);


    cout << "- Last access time: ";
    print_filetime_in_DDMMYYYY(fileInfo.ftLastAccessTime);

    cout << "- Last write time: ";
    print_filetime_in_DDMMYYYY(fileInfo.ftLastWriteTime);


    cout << "- Number of links: " << fileInfo.nNumberOfLinks << "\n";
    cout << "- File attributes: " << fileInfo.dwFileAttributes << "\n";

    // Close the file handle after we're done
    CloseHandle(fileHandle);
}

void getFileAttributes(string file) {

    DWORD fileAttributes = GetFileAttributes(file.c_str());

    // Check if the call to GetFileAttributes was successful
    if (fileAttributes == INVALID_FILE_ATTRIBUTES) {
        printLastErrorMessage();
    }

    // Print the attributes of the file or directory
    cout << "File attributes:\n";

    if (fileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
        cout << "- Directory\n";
    } else {
        cout << "- File\n";
    }

    if (fileAttributes & FILE_ATTRIBUTE_READONLY) {
        cout << "- Read-only\n";
    }

    if (fileAttributes & FILE_ATTRIBUTE_HIDDEN) {
        cout << "- Hidden\n";
    }

    if (fileAttributes & FILE_ATTRIBUTE_SYSTEM) {
        cout << "- System\n";
    }

    if (fileAttributes & FILE_ATTRIBUTE_ARCHIVE) {
        cout << "- Archive\n";
    }
}

void getAllFileAttributes(string file) {
    getFileAttributes(file);
    getHandleFileAttributes(file);
}

void setCurrentTimeForFile(string file) {

    // Open the file with write access
    HANDLE fileHandle = CreateFile(
            file.c_str(),
            GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            nullptr,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            nullptr
    );

    if (fileHandle == INVALID_HANDLE_VALUE) {
        printLastErrorMessage();
    }

    // Set the new last write time to the current time
    FILETIME newLastWriteTime;
    SYSTEMTIME systemTime;
    GetSystemTime(&systemTime); // Get the current system time
    SystemTimeToFileTime(&systemTime, &newLastWriteTime); // Convert the SYSTEMTIME to FILETIME

    // Call SetFileTime to update the last write time
    BOOL success = SetFileTime(fileHandle, nullptr, nullptr, &newLastWriteTime);

    if (!success) {
        printLastErrorMessage();
        CloseHandle(fileHandle);
    }

    // Close the file handle after we're done
    CloseHandle(fileHandle);
    cout << "File time updated successfully." << "\n";

}

void showMenu() {
    cout << "'1' - Get disks list.\n"
            "'2' - Get disk inf and free size.\n"
            "'3' - Create and delete directories.\n"
            "'4' - Create files.\n"
            "'5' - Copying and moving files between directories.\n"
            "'6' - Analyzing and changing file attributes.\n"
            "'0' - Out.\n";
    cout << "Enter: ";
}

int main() {
    list<string> disksList;
    string inf;
    string input_string;
    bool b = TRUE;
    char ans;
    int k;
    char n;
    while(b) {
        showMenu();
        cin >> n;
        switch(n) {
            case '1':
                DiskList(&disksList);
                k = 0;
                cout << "Disks list:\n";
                for(list<string>::iterator it = disksList.begin(); it != disksList.end(); it++) {
                    cout << "'" << ++k << "' - " <<*it<<"\n";
                }
                getchar();
                break;
            case '2':
                cout << diskInf(&disksList);
                getchar();
                break;
            case '3':
                cout << "'1' - Create directory.\n"
                        "'2' - Delete directory\n"
                        "Enter: ";
                cin >> ans;
                switch (ans) {
                    case '1':
                        createDirect();
                        break;
                    case '2':
                        deleteDirect();
                        break;
                }
                getchar();
                break;
            case '4':
                createFile();
                getchar();
                break;
            case '5':
                cout << "'1' - Copy file.\n"
                        "'2' - Moving file\n"
                        "Enter: ";
                cin >> ans;
                switch (ans) {
                    case '1':
                        copyFiles();
                        break;
                    case '2':
                        moveFiles();
                        break;
                }
                getchar();
                break;
            case '6':
                cout << "'1' - Get file attributes.\n"
                        "'2' - Change file attributes.\n"
                        "Enter: ";
                cin >> ans;
                switch (ans) {
                    case '1':
                        cout << "Enter file (D:\\folder\\file.txt) :";
                        cin >> input_string;
                        getAllFileAttributes(input_string);
                        break;
                    case '2':
                        cout << "Enter file (D:\\folder\\file.txt): ";
                        cin >> input_string;
                        setCurrentTimeForFile(input_string);
                        break;
                }
                getchar();
                break;
            case '0':
                b = FALSE;
                break;
        }
        getchar();
    }

    return 0;
}
