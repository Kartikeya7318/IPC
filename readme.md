Shared Memory Inter-Process Communication (IPC) Example

This project demonstrates how to use shared memory for inter-process communication (IPC) with locking mechanisms in both Windows and Linux. The shared memory is used to exchange data between a sender and a receiver process. The implementation ensures data consistency using platform-specific locking mechanisms.

Files

shared_memory.h: Header file for the SharedMemory class.

shared_memory.cpp: Implementation of the SharedMemory class with cross-platform locking.

sender.cpp: Sender process, which writes data to shared memory.

receiver.cpp: Receiver process, which reads data from shared memory.

Key Features

Cross-platform compatibility (Linux and Windows).

Uses locking mechanisms for thread-safe access:

Windows: Mutex (CreateMutex).

Linux: File-based locking (fcntl).

Demonstrates data exchange between two processes using shared memory.

Compilation Instructions

Linux

Install Required Libraries:
Ensure you have g++ installed:

sudo apt-get update
sudo apt-get install g++

Compile:
Run the following commands:

g++ -o sender sender.cpp shared_memory.cpp -lpthread
g++ -o receiver receiver.cpp shared_memory.cpp -lpthread

Run:
Open two terminals:

Terminal 1: Start the sender process:

./sender

Terminal 2: Start the receiver process:

./receiver

Windows

Install MinGW or MSVC:

For MinGW: Install it from MinGW.

For MSVC: Use Visual Studio with the "Desktop development with C++" workload installed.

Compile:

Using MinGW:

g++ -o sender.exe sender.cpp shared_memory.cpp -lws2_32
g++ -o receiver.exe receiver.cpp shared_memory.cpp -lws2_32

Using MSVC (from Developer Command Prompt):

cl /EHsc /Fe:sender.exe sender.cpp shared_memory.cpp
cl /EHsc /Fe:receiver.exe receiver.cpp shared_memory.cpp

Run:
Open two command prompts:

Command Prompt 1: Start the sender process:

sender.exe

Command Prompt 2: Start the receiver process:

receiver.exe

Notes

Ensure you have appropriate permissions for shared memory operations.

Linux uses /tmp/shared_memory_lock as a lock file for interprocess locking.

Both Windows and Linux use the same shared memory name (SharedMemoryExample).

The shared memory size and structure are customizable in the SharedMemory class.

Example Output

Sender:

Sent: 0 1 2 3 4 5 6 7 8 9 
Sent: 1 2 3 4 5 6 7 8 9 10 
...

Receiver:

Received: 0 1 2 3 4 5 6 7 8 9 
Received: 1 2 3 4 5 6 7 8 9 10 
...

