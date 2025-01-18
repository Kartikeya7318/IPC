Shared Memory Inter-Process Communication (IPC) Example

Linux
mkdir build
cd build
cmake ..
make

Run:
Open two terminals:

Terminal 1: Start the sender process:

./sender

Terminal 2: Start the receiver process:

./receiver

Windows

"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
mkdir build
cd build
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release
ninja



