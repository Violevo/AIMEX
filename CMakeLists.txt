cmake_minimum_required(VERSION 3.16)

# Change project name to AIMEX
project(AIMEX VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Dependencies
find_package(OpenCV REQUIRED)
find_package(Boost REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(httplib REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBUSB REQUIRED IMPORTED_TARGET libusb-1.0)

# NDI SDK (Windows only)
if(WIN32)
    set(NDI_SDK_PATH "C:/Program Files/NDI/NDI 6 SDK" CACHE PATH "Path to NDI SDK")
    find_library(NDI_LIBRARY
        NAMES Processing.NDI.Lib.x64
        PATHS "${NDI_SDK_PATH}/Lib/x64"
        REQUIRED
    )
    set(NDI_INCLUDE_DIR "${NDI_SDK_PATH}/Include")
endif()

# Source files
add_executable(AIMEX
    src/main.cpp
    src/ndi_receiver.cpp
    src/color_filter.cpp
    src/vector_calculator.cpp
    src/usb_sender.cpp
    src/web_server.cpp
    src/makcu.cpp
    src/serialport.cpp
)

# Includes
target_include_directories(AIMEX PRIVATE
    ${OpenCV_INCLUDE_DIRS}
    ${NDI_INCLUDE_DIR}
    src/
)

# Link libraries
target_link_libraries(AIMEX PRIVATE
    ${OpenCV_LIBS}
    ${NDI_LIBRARY}
    PkgConfig::LIBUSB
    nlohmann_json::nlohmann_json
    httplib::httplib
)

# Windows-specific
if(WIN32)
    target_link_libraries(AIMEX PRIVATE
        ws2_32
        winmm
    )
endif()

# Compiler flags
target_compile_definitions(AIMEX PRIVATE
    $<$<CONFIG:Debug>:DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
)

# Copy web files
file(COPY web/ DESTINATION ${CMAKE_BINARY_DIR}/web/)

# Installation
install(TARGETS AIMEX DESTINATION bin)
install(DIRECTORY web/ DESTINATION share/AIMEX/web)

# Packaging
set(CPACK_PACKAGE_NAME "AIMEX")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Real-time video processing from NDI input, with colour filtering and MACKU output")
set(CPACK_PACKAGE_VENDOR "Violevo")
if(WIN32)
    set(CPACK_GENERATOR "NSIS;ZIP")
    set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
endif()
include(CPack)
