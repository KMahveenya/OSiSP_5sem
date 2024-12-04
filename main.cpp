#include <windows.h>
#include <iostream>
#include <vector>
#include <string>

using namespace std;

const int number_of_workers = 2;

int main()
{
    setlocale(LC_ALL, "rus");
    HANDLE pipes[number_of_workers];
    PROCESS_INFORMATION processes[number_of_workers];

    for (int n = 0; n < number_of_workers; n++)
    {
        string namedPipeName = "\\\\.\\pipe\\MyPipe" + to_string(n);
        vector<int> vec;

        for (int i = 0; i <= 10; i++)
        {
            vec.push_back(rand() % 1000 - 500);
        }

        pipes[n] = CreateNamedPipeA(
            namedPipeName.c_str(),                                  // ��� ������
            PIPE_ACCESS_DUPLEX,                                     // ������ � ������
            PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,  // ��� ���������
            PIPE_UNLIMITED_INSTANCES,                               // ������������ ���������� ���������
            512,                                                    // �������� �����
            512,                                                    // ������� �����
            0,                                                      // ����-���
            NULL                                                    // ��� ������
        );

        string workerPath = "C:\\OSiSP\\x64\\Debug\\OSiSP_lr3_worker.exe";

        // ��������� ��� ���������� � �������� � ��� �������
        STARTUPINFO si;

        workerPath = workerPath + " " + to_string(n);
        wstring workerPathModified(workerPath.begin(), workerPath.end());

        // �������������� ��������� ������
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        ZeroMemory(&processes[n], sizeof(processes[n]));

        // ��������� �������
        CreateProcess(
            NULL,                               // ���� � ������������ �����
            &workerPathModified[0],             // ��������� ������
            NULL,                               // ��������� �� �������
            NULL,                               // ��������� �� �����
            FALSE,                              // ����������� �����������
            CREATE_NEW_CONSOLE,                 // ����� ��������
            NULL,                               // ���������� ���������
            NULL,                               // ������� �������
            &si,                                // ��������� ���������� � �������
            &processes[n]);

        cout << "�������� ����������� � �������..." << endl;

        BOOL result = ConnectNamedPipe(pipes[n], NULL);
        if (!result) {
            cerr << "ConnectNamedPipe failed. Error: " << GetLastError() << endl;
            CloseHandle(pipes[n]);
            return 1;
        }

        cout << "������ ���������.\n\n������������ ������:\n";

        for (int i = 0; i < vec.size(); i++) {

            cout << vec[i] << ' ';
        }
        cout << '\n';

        DWORD bytesWritten;
        WriteFile(pipes[n], vec.data(), vec.size() * sizeof(int) + 1, &bytesWritten, NULL);

        DWORD bytesRead;
        ReadFile(pipes[n], vec.data(), vec.size() * sizeof(int) + 1, &bytesRead, NULL);

        cout << "���������� ������:\n";

        for (int i = 0; i < vec.size(); i++) {

            cout << vec[i] << ' ';
        }

        cout << "\n\n";
    }

    HANDLE* processesHandles = (HANDLE*)malloc(sizeof(HANDLE) * number_of_workers);
    for (int i = 0; i < number_of_workers; i++)
    {
        processesHandles[i] = processes[i].hProcess;
    }

    WaitForMultipleObjects(number_of_workers, processesHandles, true, INFINITE);
    
    // �������� ������������ �������� � ������
    for (int i = 0; i < number_of_workers; i++)
    {
        CloseHandle(pipes[i]);
        CloseHandle(processes[i].hProcess);
        CloseHandle(processes[i].hThread);
    }


    return 0;
}
