#ifndef SHARED_MEMORY_H
#define SHARED_MEMORY_H

#include <iostream>
#include <vector>
#include <cstring>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/mman.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <fcntl.h>  // For O_CREAT and O_RDWR
#endif

// Define a shared data structure
struct Data {
    float values[10];
};

class SharedMemory {
public:
    SharedMemory(const char* name, size_t size, bool isCreator = false);
    ~SharedMemory();
    Data* getData();

    void lockSender();
    void lockReceiver();
    void unlockSender();
    void unlockReceiver();

private:
    const char* name;
    size_t size;
    bool isCreator;

#ifdef _WIN32
    HANDLE hMapFile;
    LPVOID pBuf;
    HANDLE hMutexSender;  // Handle for sender mutex
    HANDLE hMutexReceiver;  // Handle for receiver mutex
#else
    int shm_fd;
    void* shared_mem;
    int lock_fd_sender;    // File descriptor for sender locking
    int lock_fd_receiver;  // File descriptor for receiver locking
#endif
};

#endif // SHARED_MEMORY_H
