#!/usr/bin/env bash

set -ev

mkdir -p build
cd build
cmake ..
make
for file in ../tests/test-*.plorth
do
  echo $file
  ./cli/plorth $file
done
