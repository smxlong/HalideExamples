#!/bin/bash

SCRIPT_PATH=`dirname $0`
cd $SCRIPT_PATH
SCRIPT_PATH=`pwd`
SRC_ROOT=$SCRIPT_PATH

BUILD_ROOT=$SRC_ROOT/build-dir
CMAKE_BUILD_ROOT=$BUILD_ROOT/cmake-build

if ! [ -r $CMAKE_BUILD_ROOT/CMakeCache.txt ]; then
	if ! (rm -rf $BUILD_ROOT && mkdir -p $CMAKE_BUILD_ROOT); then
		echo ERROR: could not create build directory $CMAKE_BUILD_ROOT
		exit 1
	fi
fi

cd $CMAKE_BUILD_ROOT && cmake $* $SRC_ROOT && make install
