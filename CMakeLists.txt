cmake_minimum_required(VERSION 4.0)
project(chip8emu 
        VERSION 1.0
        DESCRIPTION "CHIP-8/SuperCHIP Emulator"
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find SDL2 package
find_package(SDL2 REQUIRED)

# Define the executable
add_executable(${PROJECT_NAME}
    main.cpp
    Chip8.cpp
    SuperChip.cpp
)

# Include directories using modern CMake
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${SDL2_INCLUDE_DIRS}
)

# Link libraries using modern CMake
target_link_libraries(${PROJECT_NAME} PRIVATE
    ${SDL2_LIBRARIES}
)

# Enable warnings
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()
