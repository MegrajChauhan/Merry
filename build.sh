#!/bin/bash

cd build
rm -r *
cmake -DCMAKE_BUILD_TYPE=release ..
make
