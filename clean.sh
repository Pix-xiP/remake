#!/usr/bin/sh

set -ex

SRC="./src"
PWD=$(pwd)

cd "$SRC"
rm ./*.o
cd "$PWD"
