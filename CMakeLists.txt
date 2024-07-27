cmake_minimum_required(VERSION 3.0)

project(SOPG-TP)

# Set C standard
set(CMAKE_C_STANDARD 11)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}  # Include the root directory
    ${CMAKE_SOURCE_DIR}/TP  # Include TP directory
)

# Compile executables separately
add_executable(writer ${CMAKE_SOURCE_DIR}/TP/writer.c ${CMAKE_SOURCE_DIR}/TP/logger.c)
add_executable(reader ${CMAKE_SOURCE_DIR}/TP/reader.c ${CMAKE_SOURCE_DIR}/TP/logger.c)
