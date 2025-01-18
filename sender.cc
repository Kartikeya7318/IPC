#include "shared_memory.h"
#include <thread>
#include <chrono>
#include <ctime>

int main() {
    try {
        SharedMemory shm("SharedMemoryExample", sizeof(Data), true);
        Data* data = shm.getData();

        float counter = 0.0f;
        while (true) {
            shm.lockSender();  // Lock before writing
            for (int i = 0; i < 10; ++i) {
                data->values[i] = counter + i;
            }
            auto epoch_time = std::chrono::system_clock::now();
            data->time=static_cast<long>(std::chrono::duration_cast<std::chrono::milliseconds>(epoch_time.time_since_epoch()).count());

            shm.unlockSender();  // Unlock after writing

            std::cout << "Sent: Data: ";
            for (float val : data->values) {
                std::cout << val << " ";
            }
            std::cout<< "Time: " << data->time;
            std::cout << std::endl;

            counter += 1.0f;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}
