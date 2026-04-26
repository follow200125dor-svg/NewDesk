cmake_minimum_required(VERSION 3.16)
project(RemoteDesk VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Network Gui)

# Protobuf
find_package(Protobuf REQUIRED)

# libsodium
find_package(sodium REQUIRED)

# zlib-ng (через стандартный FindZLIB)
find_package(ZLIB REQUIRED)

# Генерация protobuf файлов
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS proto/remote.proto)

add_executable(RemoteDesk
    src/main.cpp
    src/server.cpp
    src/client.cpp
    ${PROTO_SRCS}
)

target_include_directories(RemoteDesk PRIVATE
    ${CMAKE_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${Protobuf_INCLUDE_DIRS}
    ${sodium_INCLUDE_DIRS}
    ${ZLIB_INCLUDE_DIRS}
)

target_link_libraries(RemoteDesk PRIVATE
    Qt6::Widgets
    Qt6::Network
    Qt6::Gui
    ${Protobuf_LIBRARIES}
    ${sodium_LIBRARIES}
    ${ZLIB_LIBRARIES}
)
