#include <windows.h>
#include <iostream>
#include <chrono>

using namespace std;

const int NUM_READERS = 3;
const int NUM_WRITERS = 2;

const int METHOD = 4;
/*
1 - монопольная запись, параллельное чтение
2 - параллельная запись, параллельное чтение
3 - монопольная запись, грязное чтение
4 - монопольная запись, монопольное чтение
5 - параллельная запись, грязное чтение
*/

CRITICAL_SECTION cs;
HANDLE semaphore;
int read_count = 0;

int active_readers = 0;
int active_writers = 0;
HANDLE readers_turn;
HANDLE writers_turn;

int memory = 0;

DWORD WINAPI reader(LPVOID lpParam)
{
    int id = *(int*)lpParam;

    if (METHOD == 1)
    {
        while (true)
        {
            EnterCriticalSection(&cs);
            if (read_count == 0) {
                WaitForSingleObject(semaphore, INFINITE);
            }
            read_count++;
            LeaveCriticalSection(&cs);

            cout << "Reader " << id << " is reading. Value: " << memory << endl;
            Sleep(rand() % 1000);

            EnterCriticalSection(&cs);
            read_count--;
            if (read_count == 0) {
                ReleaseSemaphore(semaphore, 1, NULL);
            }
            LeaveCriticalSection(&cs);

            Sleep(rand() % 1000);
        }
    }
    else if (METHOD == 2)
    {
        while (true)
        {
            WaitForSingleObject(readers_turn, INFINITE);

            EnterCriticalSection(&cs);
            active_readers++;
            if (active_readers == 1) {
                WaitForSingleObject(writers_turn, INFINITE);
            }
            LeaveCriticalSection(&cs);

            ReleaseSemaphore(readers_turn, 1, NULL);

            cout << "Reader " << id << " is reading. Value: " << memory << endl;
            Sleep(rand() % 1000);

            EnterCriticalSection(&cs);
            active_readers--;
            if (active_readers == 0) {
                ReleaseSemaphore(writers_turn, 1, NULL);
            }
            LeaveCriticalSection(&cs);

            Sleep(rand() % 1000);
        }
    }
    else if (METHOD == 3)
    {
        while (true)
        {
            EnterCriticalSection(&cs);
            active_readers++;
            if (active_readers == 1) {
                WaitForSingleObject(writers_turn, INFINITE);
            }
            LeaveCriticalSection(&cs);

            cout << "Reader " << id << " is reading. Value: " << memory << endl;
            Sleep(rand() % 1000);

            EnterCriticalSection(&cs);
            active_readers--;
            if (active_readers == 0) {
                ReleaseSemaphore(writers_turn, 1, NULL);
            }
            LeaveCriticalSection(&cs);

            Sleep(rand() % 1000);
        }
    }
    else if (METHOD == 4)
    {
        while (true) {
            WaitForSingleObject(semaphore, INFINITE);

            cout << "Reader " << id << " is reading. Value: " << memory << endl;
            Sleep(rand() % 1000);

            ReleaseSemaphore(semaphore, 1, NULL);

            Sleep(rand() % 1000);
        }
    }
    else if (METHOD == 5)
    {
        while (true)
        {
            cout << "Reader " << id << " is reading. Value: " << memory << endl;
            Sleep(rand() % 1000);

            Sleep(rand() % 1000);
        }
    }
    return 0;
}

DWORD WINAPI writer(LPVOID lpParam)
{
    if (METHOD == 1)
    {
        int id = *(int*)lpParam;

        while (true)
        {
            WaitForSingleObject(semaphore, INFINITE);

            cout << "Writer " << id << " is writing." << endl;
            memory++;
            Sleep(rand() % 1000);

            ReleaseSemaphore(semaphore, 1, NULL);

            Sleep(rand() % 1000);
        }

    }
    else if (METHOD == 2)
    {
        int id = *(int*)lpParam;

        while (true)
        {
            EnterCriticalSection(&cs);
            active_writers++;
            if (active_writers == 1) {
                WaitForSingleObject(readers_turn, INFINITE);
            }
            LeaveCriticalSection(&cs);

            cout << "Writer " << id << " is writing." << endl;
            memory++;
            Sleep(rand() % 1000);

            EnterCriticalSection(&cs);
            active_writers--;
            if (active_writers == 0) {
                ReleaseSemaphore(readers_turn, 1, NULL);
            }
            LeaveCriticalSection(&cs);

            Sleep(rand() % 1000);
        }
    }
    else if (METHOD == 3)
    {
        int id = *(int*)lpParam;

        while (true)
        {
            WaitForSingleObject(writers_turn, INFINITE);

            cout << "Writer " << id << " is writing." << endl;

            Sleep(rand() % 1000);
            memory++;
            ReleaseSemaphore(writers_turn, 1, NULL);

            Sleep(rand() % 1000);
        }
    }
    else if (METHOD == 4)
    {
        int id = *(int*)lpParam;

        while (true) {
            WaitForSingleObject(semaphore, INFINITE);

            cout << "Writer " << id << " is writing." << endl;
            memory++;
            Sleep(rand() % 1000);

            ReleaseSemaphore(semaphore, 1, NULL);

            Sleep(rand() % 1000);
        }
    }
    else if (METHOD == 5)
    {
        int id = *(int*)lpParam;

        while (true)
        {
            cout << "Writer " << id << " is writing." << endl;
            int new_value = memory + 1;
            Sleep(rand() % 1000);
            memory = new_value;
            cout << "Writer " << id << " finished writing. New value: " << memory << endl;

            Sleep(rand() % 1000);
        }
    }
    return 0;
}

int main()
{
    HANDLE readers[NUM_READERS];
    HANDLE writers[NUM_WRITERS];
    int reader_ids[NUM_READERS];
    int writer_ids[NUM_WRITERS];

    if (METHOD == 1)
    {
        InitializeCriticalSection(&cs);
        semaphore = CreateSemaphore(NULL, 1, 1, NULL);

        for (int i = 0; i < NUM_READERS; i++) {
            reader_ids[i] = i;
            readers[i] = CreateThread(NULL, 0, reader, &reader_ids[i], 0, NULL);
            if (readers[i] == NULL) {
                cerr << "Failed to create reader thread " << i << endl;
                return 1;
            }
        }

        for (int i = 0; i < NUM_WRITERS; i++) {
            writer_ids[i] = i;
            writers[i] = CreateThread(NULL, 0, writer, &writer_ids[i], 0, NULL);
            if (writers[i] == NULL) {
                cerr << "Failed to create writer thread " << i << endl;
                return 1;
            }
        }

        WaitForMultipleObjects(NUM_READERS, readers, TRUE, INFINITE);
        WaitForMultipleObjects(NUM_WRITERS, writers, TRUE, INFINITE);

        DeleteCriticalSection(&cs);
        CloseHandle(semaphore);
    }
    else if (METHOD == 2)
    {
        InitializeCriticalSection(&cs);
        readers_turn = CreateSemaphore(NULL, 1, 1, NULL);
        writers_turn = CreateSemaphore(NULL, 1, 1, NULL);

        for (int i = 0; i < NUM_READERS; i++) {
            reader_ids[i] = i;
            readers[i] = CreateThread(NULL, 0, reader, &reader_ids[i], 0, NULL);
            if (readers[i] == NULL) {
                cerr << "Failed to create reader thread " << i << endl;
                return 1;
            }
        }

        for (int i = 0; i < NUM_WRITERS; i++) {
            writer_ids[i] = i;
            writers[i] = CreateThread(NULL, 0, writer, &writer_ids[i], 0, NULL);
            if (writers[i] == NULL) {
                cerr << "Failed to create writer thread " << i << endl;
                return 1;
            }
        }

        WaitForMultipleObjects(NUM_READERS, readers, TRUE, INFINITE);
        WaitForMultipleObjects(NUM_WRITERS, writers, TRUE, INFINITE);

        DeleteCriticalSection(&cs);
        CloseHandle(readers_turn);
        CloseHandle(writers_turn);
    }
    else if (METHOD == 3)
    {
        InitializeCriticalSection(&cs);
        writers_turn = CreateSemaphore(NULL, 1, 1, NULL);

        for (int i = 0; i < NUM_READERS; i++) {
            reader_ids[i] = i;
            readers[i] = CreateThread(NULL, 0, reader, &reader_ids[i], 0, NULL);
            if (readers[i] == NULL) {
                cerr << "Failed to create reader thread " << i << endl;
                return 1;
            }
        }

        for (int i = 0; i < NUM_WRITERS; i++) {
            writer_ids[i] = i;
            writers[i] = CreateThread(NULL, 0, writer, &writer_ids[i], 0, NULL);
            if (writers[i] == NULL) {
                cerr << "Failed to create writer thread " << i << endl;
                return 1;
            }
        }

        WaitForMultipleObjects(NUM_READERS, readers, TRUE, INFINITE);
        WaitForMultipleObjects(NUM_WRITERS, writers, TRUE, INFINITE);

        DeleteCriticalSection(&cs);
        CloseHandle(writers_turn);
    }
    else if (METHOD == 4)
    {
        semaphore = CreateSemaphore(NULL, 1, 1, NULL);

        for (int i = 0; i < NUM_READERS; i++) {
            reader_ids[i] = i;
            readers[i] = CreateThread(NULL, 0, reader, &reader_ids[i], 0, NULL);
            if (readers[i] == NULL) {
                cerr << "Failed to create reader thread " << i << endl;
                return 1;
            }
        }

        for (int i = 0; i < NUM_WRITERS; i++) {
            writer_ids[i] = i;
            writers[i] = CreateThread(NULL, 0, writer, &writer_ids[i], 0, NULL);
            if (writers[i] == NULL) {
                cerr << "Failed to create writer thread " << i << endl;
                return 1;
            }
        }

        WaitForMultipleObjects(NUM_READERS, readers, TRUE, INFINITE);
        WaitForMultipleObjects(NUM_WRITERS, writers, TRUE, INFINITE);

        CloseHandle(semaphore);
    }
    else if (METHOD == 5)
    {
        

        for (int i = 0; i < NUM_READERS; i++) {
            reader_ids[i] = i;
            readers[i] = CreateThread(NULL, 0, reader, &reader_ids[i], 0, NULL);
            if (readers[i] == NULL) {
                cerr << "Failed to create reader thread " << i << endl;
                return 1;
            }
        }

        for (int i = 0; i < NUM_WRITERS; i++) {
            writer_ids[i] = i;
            writers[i] = CreateThread(NULL, 0, writer, &writer_ids[i], 0, NULL);
            if (writers[i] == NULL) {
                cerr << "Failed to create writer thread " << i << endl;
                return 1;
            }
        }

        WaitForMultipleObjects(NUM_READERS, readers, TRUE, INFINITE);
        WaitForMultipleObjects(NUM_WRITERS, writers, TRUE, INFINITE);
    }

    for (int i = 0; i < NUM_READERS; i++) {
        CloseHandle(readers[i]);
    }
    for (int i = 0; i < NUM_WRITERS; i++) {
        CloseHandle(writers[i]);
    }

    return 0;
}
