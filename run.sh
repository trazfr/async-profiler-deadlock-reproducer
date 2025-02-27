#!/bin/sh

MALLOC_SIZE=$((300 * 1024 * 1024))
MARCH=$(uname -m)
BIN_OUTPUT=/tmp/test-tcmalloc

export TCMALLOC_STACKTRACE_METHOD_VERBOSE=true

while [ "$#" -ne 0 ]; do
case "$1" in
    --malloc-size)
        MALLOC_SIZE="$2"
        shift 2
        ;;
    --stacktrace)
        export TCMALLOC_STACKTRACE_METHOD="$2"
        shift 2
        ;;
    --bin-output)
        BIN_OUTPUT="$2"
        shift 2
        ;;
    *)
        echo "Unknown option: $1"
        exit 1
        ;;
esac
done

cd "$(dirname "$0")"

set -x

g++ -std=c++11 -Wall -Wextra -pedantic -g -o "$BIN_OUTPUT" main.cpp

echo "## Run without LD_PRELOAD"
"$BIN_OUTPUT" "$MALLOC_SIZE"

echo "## Run with LD_PRELOAD=libtcmalloc_minimal.so"
LD_PRELOAD="/usr/lib/$MARCH-linux-gnu/libtcmalloc_minimal.so.4" "$BIN_OUTPUT" "$MALLOC_SIZE"

echo "## Run with LD_PRELOAD=libtcmalloc.so"
LD_PRELOAD="/usr/lib/$MARCH-linux-gnu/libtcmalloc.so.4" "$BIN_OUTPUT" "$MALLOC_SIZE"
