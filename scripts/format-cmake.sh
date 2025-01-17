#!/bin/bash

# get script directory
DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

cd $DIR/../

find . \( -path ./.build -o -path ./third_party \) -prune -o -type f -name CMakeLists.txt -print -exec cmake-format -i {} \;
# find . \( -path ./.build -o -path ./third_party \) -prune -o -type f -name "*.cmake" -exec cmake-format -i {} \;
