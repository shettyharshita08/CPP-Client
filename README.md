# CPP-Client

--------Requirements---------
1. g++ compiler
2. MinGW for Windows
3. GCC for Linux/macOS
4. Node.js (v16.17.0 or higher)
5. nlohmann/json library

--------For Windows---------
1. Install MinGW or Visual Studio with C++ support.
2. Install nlohmann/json using vcpkg or manually.
3. Use Winsock2 instead of POSIX for socket operations in your C++ code.

--------For Linux/macOS Setups---------

Install dependencies:
1. sudo apt update
2. sudo apt install g++ nlohmann-json-dev

Compile the client:
1. g++ abx_client.cpp -o abx_client -I/usr/include/nlohmann

Run the ABX server:
1. node main.js

Execute the client:
1. ./abx_client
