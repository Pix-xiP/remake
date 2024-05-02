#!/bin/bash

LIBS="-I./lua-5.4.6/src -I ./mimalloc/include -L./lua-5.4.6/src -llua -lm"
ASAN="-fsanitize=address"
FLAGS="-ggdb -g3 -Og -fPIC -fno-omit-frame-pointer"
CMD="gcc -o pb  $FLAGS ./mimalloc/src/static.c ./src/*.c $LIBS"

exec $CMD
