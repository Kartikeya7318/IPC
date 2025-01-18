#include "shared_memory.h"
#include <thread>
#include <chrono>

#ifdef _WIN32
// Constructor for Windows
SharedMemory::SharedMemory(const char* name, size_t size, bool isCreator)
    : name(name), size(size), isCreator(isCreator) {
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

    // Create or open mutexes for sender and receiver
    hMutexSender = CreateMutex(nullptr, FALSE, (std::string(name) + "_Sender").c_str());
    hMutexReceiver = CreateMutex(nullptr, FALSE, (std::string(name) + "_Receiver").c_str());
    if (hMutexSender == nullptr || hMutexReceiver == nullptr) {
        throw std::runtime_error("Failed to create/open mutexes.");
    }
}
#else
// Constructor for Linux/Unix
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

    // Map the shared memory into the process's address space
    shared_mem = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shared_mem == MAP_FAILED) {
        throw std::runtime_error("Failed to map shared memory.");
    }

    // Create or open lock files for sender and receiver
    lock_fd_sender = open((std::string(name) + "_sender.lock").c_str(), O_CREAT | O_RDWR, 0666);
    lock_fd_receiver = open((std::string(name) + "_receiver.lock").c_str(), O_CREAT | O_RDWR, 0666);
    if (lock_fd_sender == -1 || lock_fd_receiver == -1) {
        throw std::runtime_error("Failed to create/open lock files.");
    }
}
#endif

// Destructor
SharedMemory::~SharedMemory() {
#ifdef _WIN32
    UnmapViewOfFile(pBuf);
    CloseHandle(hMapFile);
    CloseHandle(hMutexSender);
    CloseHandle(hMutexReceiver);
#else
    munmap(shared_mem, size);
    close(lock_fd_sender);
    close(lock_fd_receiver);
    if (isCreator) {
        shm_unlink(name);
    }
#endif
}

// Get the data pointer
Data* SharedMemory::getData() {
#ifdef _WIN32
    return static_cast<Data*>(pBuf);
#else
    return static_cast<Data*>(shared_mem);
#endif
}

// Lock the sender
void SharedMemory::lockSender() {
    int retries = 0;
    const int maxRetries = 10;
    const int retryDelayMs = 20;

    while (retries < maxRetries) {
        // std::cout << "Sender attempting to lock mutex (Attempt " << retries + 1 << ")..." << std::endl;

#ifdef _WIN32
        DWORD dwWaitResult = WaitForSingleObject(hMutexSender, INFINITE);
        if (dwWaitResult == WAIT_FAILED) {
            std::cerr << "Mutex lock failed with error: " << GetLastError() << std::endl;
        } else {
            // std::cout << "Sender mutex locked." << std::endl;
            return;
        }
#else
        struct flock fl;
        memset(&fl, 0, sizeof(fl));
        fl.l_type = F_WRLCK;

        if (fcntl(lock_fd_sender, F_SETLKW, &fl) == -1) {
            std::cerr << "Sender failed to acquire lock: " << strerror(errno) << std::endl;
        } else {
            std::cout << "Sender lock acquired." << std::endl;
            return;
        }
#endif

        retries++;
        std::this_thread::sleep_for(std::chrono::milliseconds(retryDelayMs));
    }
    std::cerr << "Sender failed to lock mutex after " << maxRetries << " attempts." << std::endl;
}

// Unlock the sender
void SharedMemory::unlockSender() {
#ifdef _WIN32
    if (!ReleaseMutex(hMutexSender)) {
        throw std::runtime_error("Failed to release mutex.");
    }
#else
    struct flock fl;
    memset(&fl, 0, sizeof(fl));
    fl.l_type = F_UNLCK;

    if (fcntl(lock_fd_sender, F_SETLK, &fl) == -1) {
        throw std::runtime_error("Failed to release lock.");
    }
#endif
}

// Lock the receiver
void SharedMemory::lockReceiver() {
    int retries = 0;
    const int maxRetries = 10;
    const int retryDelayMs = 20;

    while (retries < maxRetries) {
        // std::cout << "Receiver attempting to lock mutex (Attempt " << retries + 1 << ")..." << std::endl;

#ifdef _WIN32
        DWORD dwWaitResult = WaitForSingleObject(hMutexReceiver, INFINITE);
        if (dwWaitResult == WAIT_FAILED) {
            std::cerr << "Mutex lock failed with error: " << GetLastError() << std::endl;
        } else {
            // std::cout << "Receiver mutex locked." << std::endl;
            return;
        }
#else
        struct flock fl;
        memset(&fl, 0, sizeof(fl));
        fl.l_type = F_WRLCK;

        if (fcntl(lock_fd_receiver, F_SETLKW, &fl) == -1) {
            std::cerr << "Receiver failed to acquire lock: " << strerror(errno) << std::endl;
        } else {
            std::cout << "Receiver lock acquired." << std::endl;
            return;
        }
#endif

        retries++;
        std::this_thread::sleep_for(std::chrono::milliseconds(retryDelayMs));
    }
    std::cerr << "Receiver failed to lock mutex after " << maxRetries << " attempts." << std::endl;
}

// Unlock the receiver
void SharedMemory::unlockReceiver() {
#ifdef _WIN32
    if (!ReleaseMutex(hMutexReceiver)) {
        throw std::runtime_error("Failed to release mutex.");
    }
#else
    struct flock fl;
    memset(&fl, 0, sizeof(fl));
    fl.l_type = F_UNLCK;

    if (fcntl(lock_fd_receiver, F_SETLK, &fl) == -1) {
        throw std::runtime_error("Failed to release lock.");
    }
#endif
}
