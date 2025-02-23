#include <iostream>
#include <windows.h>
#include <cstdlib>
#include <ctime>
#include <climits>

using namespace std;

const int ARRAY_SIZE = 20;
const int PIPE_SIZE = 1024;

void fillArray(int arr[], int size) {
    srand(time(0));
    for (int i = 0; i < size; i++) {
        arr[i] = rand() % 100;
    }
}

int findMin(int arr[], int start, int end) {
    int minVal = INT_MAX;
    for (int i = start; i < end; i++) {
        if (arr[i] < minVal) {
            minVal = arr[i];
        }
    }
    return minVal;
}

int main() {
    int arr[ARRAY_SIZE];
    fillArray(arr, ARRAY_SIZE);

    cout << "Generated Array: ";
    for (int i = 0; i < ARRAY_SIZE; i++) {
        cout << arr[i] << " ";
    }
    cout << endl;

    HANDLE hReadPipe, hWritePipe;
    SECURITY_ATTRIBUTES saAttr;

    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;

    if (!CreatePipe(&hReadPipe, &hWritePipe, &saAttr, PIPE_SIZE)) {
        cerr << "Pipe creation failed!" << endl;
        return 1;
    }

    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    ZeroMemory(&pi, sizeof(pi));

    si.cb = sizeof(si);
    BOOL result = CreateProcess(
        NULL,
        (LPSTR)"child.exe",
        NULL,
        NULL,
        TRUE,
        0,
        NULL,
        NULL,
        &si,
        &pi
    );

    if (!result) {
        cerr << "CreateProcess failed!" << endl;
        return 1;
    }

    int parentMin = findMin(arr, 0, ARRAY_SIZE / 2);
    cout << "Parent Process (PID " << GetCurrentProcessId() << ") found min in first half: " << parentMin << endl;

    DWORD bytesWritten;
    int childMin = findMin(arr, ARRAY_SIZE / 2, ARRAY_SIZE);
    WriteFile(hWritePipe, &childMin, sizeof(childMin), &bytesWritten, NULL);

    CloseHandle(hWritePipe);

    WaitForSingleObject(pi.hProcess, INFINITE);

    DWORD bytesRead;
    ReadFile(hReadPipe, &childMin, sizeof(childMin), &bytesRead, NULL);
    cout << "Received from Child Process: Min in second half = " << childMin << endl;

    int overallMin = min(parentMin, childMin);
    cout << "Overall Minimum in the Array: " << overallMin << endl;

    CloseHandle(hReadPipe);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return 0;
}
