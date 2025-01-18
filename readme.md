Shared Memory Inter-Process Communication (IPC) Example

Linux

g++ -o sender sender.cc shared_memory.cc -lpthread
g++ -o receiver receiver.cc shared_memory.cc -lpthread

Run:
Open two terminals:

Terminal 1: Start the sender process:

./sender

Terminal 2: Start the receiver process:

./receiver

Windows

"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"

cl /EHsc sender.cc shared_memory.cc
cl /EHsc receiver.cc shared_memory.cc
