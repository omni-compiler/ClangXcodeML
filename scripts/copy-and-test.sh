#!/bin/sh

set -e
cd ../ # project root
cp ../CppTest/* tests/CCTest/
cd tests/CCTest
make install-cc
./check.sh
