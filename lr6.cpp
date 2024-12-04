#include <windows.h>
#include <iostream>
#include <string>
#include <vector>

void PrintRegValue(HKEY hKey, const std::string& subKey, const std::string& valueName, DWORD type, const BYTE* data, DWORD dataSize) {
    std::cout << "[" << subKey << "]\n";
    std::cout << "\"" << valueName << "\"=";

    if (type == REG_SZ) {
        std::cout << "\"" << reinterpret_cast<const char*>(data) << "\"";
    }
    else if (type == REG_DWORD) {
        DWORD value = *reinterpret_cast<const DWORD*>(data);
        std::cout << "dword:" << std::hex << value;
    }
    else if (type == REG_BINARY) {
        std::cout << "hex:";
        for (DWORD i = 0; i < dataSize; ++i) {
            printf("%02x", data[i]);
            if (i < dataSize - 1) {
                std::cout << ",";
            }
        }
    }
    else {
        std::cout << "Unsupported value type";
    }
    std::cout << "\n\n";
}

void SearchRegistryKey(HKEY hRootKey, const std::string& subKey, const std::string& searchKeyName) {
    HKEY hKey;
    if (RegOpenKeyExA(hRootKey, subKey.c_str(), 0, KEY_READ, &hKey) != ERROR_SUCCESS) {
        return;
    }

    char keyName[256];
    DWORD keyNameSize;
    DWORD index = 0;

    while (true) {
        keyNameSize = sizeof(keyName);
        LONG result = RegEnumKeyExA(hKey, index, keyName, &keyNameSize, nullptr, nullptr, nullptr, nullptr);
        if (result == ERROR_NO_MORE_ITEMS) {
            break;
        }
        else if (result == ERROR_SUCCESS) {
            std::string currentKey = subKey + "\\" + keyName;
            if (searchKeyName.empty() || currentKey.find(searchKeyName) != std::string::npos) {
                std::cout << "[" << currentKey << "]\n\n";
            }
            SearchRegistryKey(hRootKey, currentKey, searchKeyName);
        }
        ++index;
    }
    RegCloseKey(hKey);
}

void SearchRegistryValue(HKEY hRootKey, const std::string& subKey, const std::string& searchValueName, const std::string& searchValueData = "", DWORD searchValueDWORD = 0) {
    HKEY hKey;
    if (RegOpenKeyExA(hRootKey, subKey.c_str(), 0, KEY_READ, &hKey) != ERROR_SUCCESS) {
        return;
    }

    DWORD index = 0;
    char valueName[256];
    DWORD valueNameSize;
    BYTE data[512];
    DWORD dataSize;
    DWORD type;

    while (true) {
        valueNameSize = sizeof(valueName);
        dataSize = sizeof(data);
        LONG result = RegEnumValueA(hKey, index, valueName, &valueNameSize, nullptr, &type, data, &dataSize);

        if (result == ERROR_NO_MORE_ITEMS) {
            break;
        }
        else if (result == ERROR_SUCCESS) {
            std::string valueStr(valueName);

            bool matchName = searchValueName.empty() || valueStr.find(searchValueName) != std::string::npos;
            bool matchData = true;


            if (type == REG_SZ && !searchValueData.empty()) {

                std::string dataStr(reinterpret_cast<const char*>(data), dataSize - 1);
                matchData = dataStr.find(searchValueData) != std::string::npos;
            }
            else if (type == REG_DWORD && searchValueDWORD != 0) {

                DWORD dwordValue = *reinterpret_cast<const DWORD*>(data);
                matchData = (dwordValue == searchValueDWORD);
            }
            else if (type == REG_BINARY && !searchValueData.empty()) {

                std::string hexData;
                for (DWORD i = 0; i < dataSize; ++i) {
                    char hexByte[3];
                    sprintf_s(hexByte, "%02x", data[i]);
                    hexData += hexByte;
                }
                matchData = hexData.find(searchValueData) != std::string::npos;
            }

            if (matchName && matchData) {
                PrintRegValue(hKey, subKey, valueStr, type, data, dataSize);
            }
        }
        ++index;
    }
    RegCloseKey(hKey);
}

/*
Control Panel\\Desktop              папка с настройками рабочего стола
Software\\Microsoft\\Windows\\DWM   папка с цветом системы
*/

int main() {
    std::string searchKey = "Control Panel\\Desktop";
    std::string searchValueName = "";
    std::string searchValueData = "";
    DWORD searchValueDWORD = 0;

    std::cout << "=== Search by Key ===\n";
    SearchRegistryKey(HKEY_CURRENT_USER, searchKey, searchKey);

    std::cout << "\n=== Search by Value ===\n";
    SearchRegistryValue(HKEY_CURRENT_USER, searchKey, searchValueName, searchValueData, searchValueDWORD);

    return 0;
}
