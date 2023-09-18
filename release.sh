#!/bin/bash
set -xe
gcc -std=c17 -O3 -DDEBUG=0 ./src/*.c -o ./bin/liz