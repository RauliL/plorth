#!/usr/bin/env bash

set -ev

mkdir -p build
cd build
env CXXFLAGS="-Wall -Werror" cmake ..
make
cd ../tests
for file in test-*.plorth
do
  echo $file
  ../build/plorth-cli $file
done
