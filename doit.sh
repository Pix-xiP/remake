#!/usr/bin/bash

echo "Running PB"
bash ./clean.sh
./latest_pb

echo "Replacing old binary with latest compile."
mv pb ./latest_pb
