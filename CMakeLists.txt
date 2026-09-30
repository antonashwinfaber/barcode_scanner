cmake_minimum_required(VERSION 3.20)
project(MobileBarcodeScanner CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# Fetch ZXing-C++ v2.2.1
FetchContent_Declare(
    zxing-cpp
    GIT_REPOSITORY https://github.com/zxing-cpp/zxing-cpp.git
    GIT_TAG        v2.2.1
)
set(BUILD_WRITERS OFF CACHE BOOL "" FORCE)
set(BUILD_READERS ON CACHE BOOL "" FORCE)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_BLACKBOX_TESTS OFF CACHE BOOL "" FORCE)
set(BUILD_UNIT_TESTS OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(zxing-cpp)

add_executable(MobileBarcodeScanner
    src/main.cpp
    src/BarcodeDecoder.h
    src/BarcodeDecoder.cpp
    src/KeyboardWedge.h
    src/KeyboardWedge.cpp
    src/SessionManager.h
    src/SessionManager.cpp
    src/WebServer.h
    src/WebServer.cpp
    src/MobileUI.h
    src/AppLogger.h
    src/CloudflareTunnel.h
    src/CloudflareTunnel.cpp
    src/third_party/qrcodegen.hpp
    src/third_party/qrcodegen.cpp
)

target_include_directories(MobileBarcodeScanner PRIVATE
    src
    src/third_party
    ${zxing-cpp_SOURCE_DIR}/core/src
)

target_link_libraries(MobileBarcodeScanner PRIVATE
    ZXing::ZXing
    ws2_32
    iphlpapi
)

if(MSVC)
    target_compile_options(MobileBarcodeScanner PRIVATE /W3 /utf-8 /EHsc /permissive-)
endif()

if(MINGW)
    target_link_options(MobileBarcodeScanner PRIVATE -static -static-libgcc -static-libstdc++)
endif()

