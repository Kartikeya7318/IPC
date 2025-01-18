#include "shared_memory.h"

#ifdef _WIN32
SharedMemory::SharedMemory(const char* name, size_t size, bool isCreator)
    : name(name), size(size), isCreator(isCreator) {
    std::string mutexName = std::string(name) + "_mutex";

    if (isCreator) {
        // Create a file mapping object
        hMapFile = CreateFileMapping(
            INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0, size, name);
        if (hMapFile == nullptr) {
            throw std::runtime_error("Failed to create shared memory.");
        }
    } else {
        // Open an existing file mapping object
        hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, name);
        if (hMapFile == nullptr) {
            throw std::runtime_error("Failed to open shared memory.");
        }
    }

    // Map the shared memory into the process's address space
    pBuf = MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, size);
    if (pBuf == nullptr) {
        CloseHandle(hMapFile);
        throw std::runtime_error("Failed to map shared memory.");
    }

    // Create or open the mutex for locking
    hMutex = CreateMutex(nullptr, FALSE, mutexName.c_str());
    if (hMutex == nullptr && GetLastError() == ERROR_ACCESS_DENIED) {
        hMutex = OpenMutex(SYNCHRONIZE, FALSE, mutexName.c_str());
    }
    if (hMutex == nullptr) {
        throw std::runtime_error("Failed to create/open mutex.");
    }
}

SharedMemory::~SharedMemory() {
    UnmapViewOfFile(pBuf);
    CloseHandle(hMapFile);
    CloseHandle(hMutex);
}

Data* SharedMemory::getData() {
    return static_cast<Data*>(pBuf);
}

void SharedMemory::lock() {
    DWORD dwWaitResult = WaitForSingleObject(hMutex, INFINITE);
    if (dwWaitResult != WAIT_OBJECT_0) {
        throw std::runtime_error("Failed to lock mutex.");
    }
}

void SharedMemory::unlock() {
    if (!ReleaseMutex(hMutex)) {
        throw std::runtime_error("Failed to release mutex.");
    }
}
#else
SharedMemory::SharedMemory(const char* name, size_t size, bool isCreator)
    : name(name), size(size), isCreator(isCreator) {
    int flags = isCreator ? (O_CREAT | O_RDWR) : O_RDWR;
    shm_fd = shm_open(name, flags, 0666);
    if (shm_fd == -1) {
        throw std::runtime_error("Failed to create/access shared memory.");
    }

    if (isCreator) {
        if (ftruncate(shm_fd, size) == -1) {
            throw std::runtime_error("Failed to set shared memory size.");
        }
    }

    shared_mem = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shared_mem == MAP_FAILED) {
        throw std::runtime_error("Failed to map shared memory.");
    }

    lock_fd = open("/tmp/shared_memory_lock", O_CREAT | O_RDWR, 0666);
    if (lock_fd == -1) {
        throw std::runtime_error("Failed to create/open lock file.");
    }
}

SharedMemory::~SharedMemory() {
    munmap(shared_mem, size);
    close(lock_fd);
    if (isCreator) {
        shm_unlink(name);
        unlink("/tmp/shared_memory_lock");
    }
}

Data* SharedMemory::getData() {
    return static_cast<Data*>(shared_mem);
}

void SharedMemory::lock() {
    struct flock fl;
    memset(&fl, 0, sizeof(fl));
    fl.l_type = F_WRLCK;
    fl.l_whence = SEEK_SET;

    if (fcntl(lock_fd, F_SETLKW, &fl) == -1) {
        throw std::runtime_error("Failed to acquire lock.");
    }
}

void SharedMemory::unlock() {
    struct flock fl;
    memset(&fl, 0, sizeof(fl));
    fl.l_type = F_UNLCK;
    fl.whence = SEEK_SET;

    if (fcntl(lock_fd, F_SETLK, &fl) == -1) {
        throw std::runtime_error("Failed to release lock.");
    }
}
#endif
