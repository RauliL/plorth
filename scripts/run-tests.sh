#!/usr/bin/env bash

set -ev

mkdir -p build
cd build
env CXXFLAGS="-Wall -Werror" cmake ..
make
for file in ../tests/test-*.plorth
do
  echo $file
  ./plorth-cli/plorth $file
done
