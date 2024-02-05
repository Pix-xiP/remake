#!/bin/bash

LIBS="-I./lua-5.4.4/src -L./lua-5.4.4/src -llua -lm"
FLAGS="-g  -fPIC"
CMD="gcc -o pb  $FLAGS ./src/*.c $LIBS"

exec $CMD
