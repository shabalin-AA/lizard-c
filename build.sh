#!/bin/bash
set -xe
gcc -std=c17 -Wall -Wextra -DDEBUG=1 ./src/*.c -o ./bin/liz