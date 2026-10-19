#!/usr/bin/env bash
set -euo pipefail

BIN=./build/prog

if [ ! -x "$BIN" ]; then
    echo "FAIL: $BIN not found or not executable"
    exit 1
fi

out="$("$BIN")"

check() {
    local needle="$1"
    if ! grep -q -- "$needle" <<<"$out"; then
        echo "FAIL: expected '$needle' in output"
        echo "---- actual output ----"
        echo "$out"
        echo "-----------------------"
        exit 1
    fi
}

check "add: 5"
check "sub: 6"
check "mul: 42"
check "sum: 15"

echo "PASS"
