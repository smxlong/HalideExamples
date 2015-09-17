#!/bin/bash

SCRIPT_PATH=`dirname $0`
cd $SCRIPT_PATH
SCRIPT_PATH=`pwd`
SRC_ROOT=$SCRIPT_PATH

BUILD_ROOT=$SRC_ROOT/build-dir
CMAKE_BUILD_ROOT=$BUILD_ROOT/cmake-build

rm -rf $BUILD_ROOT && mkdir -p $CMAKE_BUILD_ROOT && cd $CMAKE_BUILD_ROOT && cmake $* $SRC_ROOT && make
