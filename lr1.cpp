#include <windows.h>
#include <iostream>

const int Thread_Number = 32;

const int number_of_elements = 1000000;

int** mas;
int queue = 1;

_int64 FileTimeToQuadWord(PFILETIME pft)
{
    return(Int64ShllMod32(pft->dwHighDateTime, 32) | pft->dwLowDateTime);
}

DWORD WINAPI ThreadFunction(LPVOID lpParam)
{
    //FILETIME ft, ft2, tt, q1, q2;
    //GetThreadTimes(GetCurrentThread(), &ft, &ft2, &tt, &tt);

    FILETIME ftKernelTimeStart, ftKernelTimeEnd;
    FILETIME ftUserTimeStart, ftUserTimeEnd;
    FILETIME ftDummy;

    _int64 qwKernelTimeElapsed, qwUserTimeElapsed, qwTotalTimeElapsed;

    GetThreadTimes(GetCurrentThread(), &ftDummy, &ftDummy,
        &ftKernelTimeStart, &ftUserTimeStart);

    int threadNumber = *static_cast<int*>(lpParam);
    std::cout << "Thread " << threadNumber << " starts" << std::endl;
    int* arr = new int[number_of_elements];

    for (int i = 0; i < Thread_Number; i++)
    {
        arr[i] = number_of_elements - i;
    }

    int n = number_of_elements;
    int opCount = 0;
    bool swapped;
    for (int i = 0; i < n - 1; ++i)
    {
        swapped = false;
        for (int j = 0; j < n - i - 1; ++j) {
            if (arr[j] > arr[j + 1])
            {
                std::swap(arr[j], arr[j + 1]);
                swapped = true;
                opCount++;
            }
        }
        if (!swapped)
        {
            break;
        }
    }

    //std::cout << "Thread " << threadNumber << " ended with result " << opCount << std::endl;
    mas[threadNumber][0] = queue;
    mas[threadNumber][1] = opCount;
    queue++;

    //GetThreadTimes(GetCurrentThread(), &q1, &q2, &tt, &tt);
    

    GetThreadTimes(GetCurrentThread(), &ftDummy, &ftDummy,
        &ftKernelTimeEnd, &ftUserTimeEnd);

    qwKernelTimeElapsed = FileTimeToQuadWord(&ftKernelTimeEnd) -
        FileTimeToQuadWord(&ftKernelTimeStart);

    qwUserTimeElapsed = FileTimeToQuadWord(&ftUserTimeEnd) -
        FileTimeToQuadWord(&ftUserTimeStart);

    qwTotalTimeElapsed = qwKernelTimeElapsed + qwUserTimeElapsed;

    std::cout << "Thread " << threadNumber << " ended with time " << qwTotalTimeElapsed / 10000 << "ms\n";

    return opCount;
}

void createThread(HANDLE* threads, int* p, DWORD* ids)
{
    for (int i = 0; i < Thread_Number; i++)
    {
            threads[i] = CreateThread(
            NULL,                   // Атрибуты безопасности (по умолчанию NULL)
            0,                      // Размер стека (по умолчанию 0)
            ThreadFunction,          // Указатель на функцию, которую выполняет поток
            &p[i],            // Параметры для переданной функции (в данном случае указатель на переменную)
            CREATE_SUSPENDED,                      // Флаги создания (0 означает немедленный запуск)
            &ids[i]              // Указатель на идентификатор потока
        );
    }
}

int setPriority(HANDLE* threads)
{
    for (int i = 0; i < Thread_Number - 1; i++)
    {
        if (!SetThreadPriority(threads[i], 0)) {
            std::cerr << "Failed to set thread priority." << std::endl;
            return 1;
        }
    }
    if (!SetThreadPriority(threads[Thread_Number - 1], 0)) {
        std::cerr << "Failed to set thread priority." << std::endl;
        return 1;
    }
    return 0;
}

int main()
{
    mas = new int* [Thread_Number];
    for (int i = 0; i < Thread_Number; i++)
    {
        mas[i] = new int[3];
        mas[i][2] = 0;
    }
    for (int k = 0; k < 10; k++)
    {
        
        HANDLE* hThreads = new HANDLE[Thread_Number];

        DWORD* dwThreadIds = new DWORD[Thread_Number];

        int* threadParams = new int[Thread_Number];

        setlocale(LC_ALL, "rus");

        for (int i = 0; i < Thread_Number; i++)
        {
            threadParams[i] = i;
            dwThreadIds[i] = i;
        }

        createThread(hThreads, threadParams, dwThreadIds);

        if (setPriority(hThreads))
        {
            return 1;
        }

        for (int i = 0; i < Thread_Number; i++)
        {
            ResumeThread(hThreads[i]);
        }

        WaitForMultipleObjects(Thread_Number, hThreads, true, INFINITE);

        // Закрытие дескриптора потока
        CloseHandle(hThreads);

        for (int i = 0; i < Thread_Number; i++)
        {
            std::cout << "Поток " << i << " закончил " << mas[i][0] << ". Количество операций " << mas[i][1] << '\n';
        }
        queue = 1;
        mas[mas[31][0] - 1][2]++;
    }
    for (int i = 0; i < Thread_Number; i++)
    {
        std::cout <</* i + 1 << " место - " << */ mas[i][2] << '\n';
    }
    return 0;
}
