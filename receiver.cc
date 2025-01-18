#include "shared_memory.h"
#include <thread>
#include <chrono>

int main() {
    try {
        SharedMemory shm("SharedMemoryExample", sizeof(Data), false);
        Data* data = shm.getData();

        while (true) {
            shm.lock();  // Lock before reading
            std::cout << "Received: ";
            for (float val : data->values) {
                std::cout << val << " ";
            }
            std::cout << std::endl;
            shm.unlock();  // Unlock after reading

            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}
