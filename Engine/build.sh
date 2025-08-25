#!/bin/bash

CC="gcc"
GOC="gccgo"
LD="gccgo"

CFLAGS="-O2 -Wall -Wextra -g -lgo -lgcc_s -lpthread -ldl -lm"
GOFLAGS="-fgo-pkgpath=sqlparser"
LFLAGS=""

SRC=src
OBJ=obj
BUILD=build

SRCS=($(find "$SRC" -name "*.c" -o -path "$SRC" -name "*.c") $(find "$SRC" -name "*.go" -o -path "$SRC" -name "*.go"))
OBJSK=()

for src in "${SRCS[@]}"; do
    obj="${OBJ}/$(basename $src.o)"
    OBJSK+=("$obj")
done

OUTPUT=$BUILD/db

function build {
    mkdir -p $OBJ
    mkdir -p $BUILD

    for i in "${!SRCS[@]}"; do
        src="${SRCS[$i]}"
        obj="${OBJSK[$i]}"

        case "$src" in
        *.c) compc $src $obj ;;
        *.go) compgo $src $obj ;;
        *) echo "Unkown file type: $src" ;;
        esac
    done

    $LD $LFLAGS "${OBJSK[@]}" -o $OUTPUT
}

function compc {
    $CC $CFLAGS -c $1 -o $2
}

function compgo {
    $GOC $CFLAGS -c $1 -o $2
}

function clean {
    clear

    rm -rf $OBJ
    rm -rf $BUILD
}

for arg in "$@"; do
    if [ "$arg" = "build" ]; then
        clean
        build
    elif [ "$arg" = "clean" ]; then
        clean
    fi
done
