#include "shared_memory.h"
#include <thread>
#include <chrono>

int main() {
    try {
        SharedMemory shm("SharedMemoryExample", sizeof(Data), false);
        Data* data = shm.getData();

        while (true) {
            shm.lockReceiver();  // Lock before reading
            std::cout << "Received: Data: ";
            for (float val : data->values) {
                std::cout << val << " ";
            }
            std::cout<< "Time: " << data->time;
            std::cout << std::endl;
            shm.unlockReceiver();  // Unlock after reading

            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}
