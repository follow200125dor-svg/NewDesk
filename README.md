# 🖥️ RemoteDesk

Fast remote desktop application built with C++ and Qt.

## Features
- Screen capture and streaming
- Mouse and keyboard input forwarding
- Encryption via libsodium
- Compression via zlib-ng
- Protobuf for fast serialization

## Build
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .

## Run
./RemoteDesk
