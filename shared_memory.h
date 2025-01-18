#ifndef SHARED_MEMORY_H
#define SHARED_MEMORY_H

#include <iostream>
#include <stdexcept>
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

    void lock();
    void unlock();

private:
    const char* name;
    size_t size;
    bool isCreator;

#ifdef _WIN32
    HANDLE hMapFile;
    LPVOID pBuf;
    HANDLE hMutex;  // Handle for mutex
#else
    int shm_fd;
    void* shared_mem;
    int lock_fd;    // File descriptor for locking
#endif
};

#endif
