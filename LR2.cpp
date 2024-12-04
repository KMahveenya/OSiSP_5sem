#include <iostream>
#include <vector>
#include <cmath>
#include <chrono>
#include <windows.h>
#include <cstdlib>

struct regularInfoOfThread
{
    HANDLE file;
    long double sum;
    LARGE_INTEGER startOffset;
    unsigned long long length;
};

struct memoryMappingInfoOfThread
{
    int* start;
    long double sum;
    unsigned long long length;
};

DWORD WINAPI memoryMappingFunction(LPVOID param)
{
    memoryMappingInfoOfThread* data = static_cast<memoryMappingInfoOfThread*>(param);

    for (int i = 0; i < data->length; i++)
    {
        volatile int dummy = 0;
        for (int i = 0; i < 10; ++i)
        {
            dummy += i;
        }
        data->sum += data->start[i];
    }

    return 0;
}

void writeFileInfo(const char* filename, int numbersCount)
{
    int wideStrSize = MultiByteToWideChar(CP_UTF8, 0, filename, -1, NULL, 0);
    LPWSTR wideStr = new WCHAR[wideStrSize];
    MultiByteToWideChar(CP_UTF8, 0, filename, -1, wideStr, wideStrSize);

    HANDLE file = CreateFile(
        wideStr,
        GENERIC_WRITE,
        0,
        nullptr,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        nullptr);

    std::vector<int> data(numbersCount);
    for (size_t i = 0; i < numbersCount; i++)
    {
        data[i] = rand() % 100;
    }

    DWORD bytesWritten;
    WriteFile(file, data.data(), static_cast<DWORD>(numbersCount * 4), &bytesWritten, nullptr);
    CloseHandle(file);
}

int thread_number = 2;
const long long numberOfNumbers = 1000000000;

void memoryMapping(const char* filename)
{
    int wideStrSize = MultiByteToWideChar(CP_UTF8, 0, filename, -1, NULL, 0);
    LPWSTR wideStr = new WCHAR[wideStrSize];
    MultiByteToWideChar(CP_UTF8, 0, filename, -1, wideStr, wideStrSize);

    HANDLE file = CreateFile(
        wideStr,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        nullptr);

    DWORD fSize = GetFileSize(file, NULL);

    HANDLE mapFile = CreateFileMappingA(
        file,
        nullptr,
        PAGE_READWRITE,
        0,
        0,
        nullptr);

    int* data = static_cast<int*>(MapViewOfFile(
        mapFile,
        FILE_MAP_ALL_ACCESS,
        0,
        0,
        0));

    int sizeOfOneChunk = (fSize / 4) / thread_number;

    std::vector<HANDLE> threads(thread_number);
    std::vector<memoryMappingInfoOfThread> threadInfo(thread_number);

    for (int i = 0; i < thread_number; i++)
    {
        unsigned long long start = i * sizeOfOneChunk, length;
        if (i == thread_number - 1)
        {
            length = (fSize / 4) - start;
        }
        else
        {
            length = sizeOfOneChunk;
        }

        threadInfo[i] = { data + start, 0, length };

        threads[i] = CreateThread(
            nullptr,
            0,
            memoryMappingFunction,
            &threadInfo[i],
            0,
            nullptr);
    }

    WaitForMultipleObjects(static_cast<DWORD>(thread_number), threads.data(), TRUE, INFINITE);

    long double totalSum = 0;
    for (size_t i = 0; i < thread_number; i++) {
        totalSum += threadInfo[i].sum;
        CloseHandle(threads[i]);
    }

    //std::cout << "Полученное значение: " << totalSum / (fSize / 4) << '\n';

    UnmapViewOfFile(data);
    CloseHandle(mapFile);
    CloseHandle(file);
}

DWORD WINAPI regularFunction(LPVOID param)
{
    regularInfoOfThread* data = static_cast<regularInfoOfThread*>(param);

    std::vector<int> buffer(data->length);

    SetFilePointerEx(data->file, data->startOffset, NULL, FILE_BEGIN);

    DWORD bytesToRead = static_cast<DWORD>(data->length * sizeof(int));
    DWORD bytesRead;
    ReadFile(data->file, buffer.data(), bytesToRead, &bytesRead, NULL);

    for (int i = 0; i < data->length ; ++i)
    {
        volatile int dummy = 0;
        for (int i = 0; i < 10; ++i)
        {
            dummy += i;
        }
        data->sum += buffer[i];
    }

    SetFilePointerEx(data->file, data->startOffset, NULL, FILE_BEGIN);

    DWORD bytesToWrite = static_cast<DWORD>(data->length * sizeof(int));
    DWORD bytesWritten;
    //WriteFile(data->file, buffer.data(), bytesToWrite, &bytesWritten, NULL);

    CloseHandle(data->file);

    return 0;
}

void regular(const char* filename)
{
    int wideStrSize = MultiByteToWideChar(CP_UTF8, 0, filename, -1, NULL, 0);
    LPWSTR wideStr = new WCHAR[wideStrSize];
    MultiByteToWideChar(CP_UTF8, 0, filename, -1, wideStr, wideStrSize);

    HANDLE file = CreateFile(
        wideStr,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        nullptr);

    DWORD fSize = GetFileSize(file, NULL);

    unsigned long long sizeOfOneChunk = (fSize / 4) / thread_number;

    std::vector<HANDLE> threads(thread_number);
    std::vector<regularInfoOfThread> threadInfo(thread_number);

    for (int i = 0; i < thread_number; i++)
    {
        unsigned long long start = i * sizeOfOneChunk, length;
        if (i == thread_number - 1)
        {
            length = (fSize / 4) - start;
        }
        else
        {
            length = sizeOfOneChunk;
        }

        HANDLE threadFileHandle;
        DuplicateHandle(
            GetCurrentProcess(),
            file,
            GetCurrentProcess(),
            &threadFileHandle,
            0,
            FALSE,
            DUPLICATE_SAME_ACCESS);

        LARGE_INTEGER startOffset;
        startOffset.QuadPart = start * 4;

        threadInfo[i] = { threadFileHandle, 0, startOffset, length };

        threads[i] = CreateThread(
            nullptr,
            0,
            regularFunction,
            &threadInfo[i],
            0,
            nullptr);
    }

    WaitForMultipleObjects(static_cast<DWORD>(thread_number), threads.data(), TRUE, INFINITE);

    long double totalSum = 0;
    for (int i = 0; i < thread_number; i++)
    {
        totalSum += threadInfo[i].sum;
        CloseHandle(threads[i]);
    }
    //std::cout << "Полученное значение: " << totalSum / (fSize / 4) << '\n';

    CloseHandle(file);
}

int main()
{
    const char* filename = "file.dat";
    setlocale(LC_ALL, "rus");

    //writeFileInfo(filename, numberOfNumbers);

    auto start = std::chrono::high_resolution_clock::now();
    regular(filename);
    auto end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> regularTime = end - start;
    std::cout << "Время обычного способа в секундах: " << regularTime.count() << '\n';

    start = std::chrono::high_resolution_clock::now();
    memoryMapping(filename);
    end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> memoryTime = end - start;
    std::cout << "Время в отображения в секундах: " << memoryTime.count() << '\n';

    std::cout << "Отображение быстрее во столько раз: " << regularTime.count() / memoryTime.count() << '\n';

    std::cout << '\n';

    return 0;
}
