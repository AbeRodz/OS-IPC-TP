#!/bin/bash

# Create a build directory if it doesn't exist
mkdir -p build

cd build

# Run CMake to generate Makefiles
cmake ..

# Build
make

if [ "$#" -gt 0 ] && [ "$1" == "-r" ]; then
    # Run the writer with '-r' option
    echo -e "\nRunning Writer process..."
    ./writer
fi

# Return to the project root directory
cd ..