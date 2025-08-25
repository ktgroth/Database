#!/bin/bash

GO="go build"

GOFLAGS="-buildmode=c-shared"

SRC=sqlparser
BUILD=build

OUTPUT=$BUILD/libsqlparser.so

function build {
    mkdir -p $BUILD
    comp
}

function comp {
    $GO $GOFLAGS -o $OUTPUT
}

function clean {
    clear

    rm -rf $BUILD
}

for arg in "$@"; do
    if [ "$arg" = "build" ]; then
        build
    elif [ "$arg" = "clean" ]; then
        clean
    fi
done
