#!/bin/bash

set -e  # stop bij fouten

BUILD_DIR=build
OUT=client
CFLAGS="-Wall -Wextra -DDEBUG"

mkdir -p "$BUILD_DIR"

gcc $CFLAGS main.c tnfs.c netw.c -o "$BUILD_DIR/$OUT"

echo
echo "[RUN] Starting $BUILD_DIR/$OUT"
echo "----------------------------------------"
"./$BUILD_DIR/$OUT"
