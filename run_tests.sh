#!/bin/sh
set -eu
passed=0
total=0
for input in tests/T??.in; do
    [ -f "$input" ] || continue
    total=$((total + 1))
    expected="${input%.in}.out"
    actual=$(mktemp)
    ./acaderp < "$input" > "$actual"
    if diff -u "$expected" "$actual"; then
        echo "PASS $(basename "$input")"
        passed=$((passed + 1))
    else
        echo "FAIL $(basename "$input")"
    fi
    rm -f "$actual" bonus_roundtrip.erp bonus_roundtrip.erp.tmp
done
echo "$passed/$total tests passed"
[ "$passed" -eq "$total" ]
