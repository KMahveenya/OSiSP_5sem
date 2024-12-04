#include <windows.h>
#include <iostream>
#include <vector>
#include <sstream>

using namespace std;

int bubblesort(vector<int>* mass, int n)
{
    int opCount = 0;
    for (int i = 1; i < n; ++i)
    {
        for (int r = 0; r < n - i; r++)
        {
            if ((*mass)[r] > (*mass)[r + 1])
            {
                int temp = (*mass)[r];
                (*mass)[r] = (*mass)[r + 1];
                (*mass)[r + 1] = temp;
            }
            opCount++;
        }
    }
    return opCount;
}

int main(int argc, char* argv[])
{
    setlocale(LC_ALL, "rus");

    int n = stoi(argv[1]);
    string namedPipeName = "\\\\.\\pipe\\MyPipe" + to_string(n);

    HANDLE hPipe = CreateFileA(
        namedPipeName.c_str(),          // ��� ������
        GENERIC_READ | GENERIC_WRITE,   // ������ � ������
        0,                              // ��� ����������� �������
        NULL,                           // ��� ������
        OPEN_EXISTING,                  // ������� ������������ �����
        0,                              // �������� �� ���������
        NULL                            // ��� �������
    );

    char buffer[512];
    DWORD bytesRead;

    vector<int> vec(11);

    ReadFile(hPipe, vec.data(), vec.size() * sizeof(int) + 1, &bytesRead, NULL);

    cout << "���������� ������: \n";

    for (int i = 0; i < vec.size(); i++) {

        cout << vec[i] << ' ';
    }
    cout << '\n';

    bubblesort(&vec, vec.size());

    DWORD bytesWritten;
    WriteFile(hPipe, vec.data(), vec.size() * sizeof(int) + 1, &bytesWritten, NULL);

    cout << "\n������������ ������: \n";

    for (int i = 0; i < vec.size(); i++) {

        cout << vec[i] << ' ';
    }
    cout << '\n';

    CloseHandle(hPipe);
    int a;
    cin >> a;
    return 0;
}