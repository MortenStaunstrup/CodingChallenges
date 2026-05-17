#!/usr/bin/env bash

set -u

if [ "$#" -ne 2 ]; then
    echo "Usage: $0 ./calc.exe Tests.txt"
    exit 1
fi

CALCULATOR="$1"
TEST_FILE="$2"

if [ ! -f "$CALCULATOR" ]; then
    echo "Error: calculator '$CALCULATOR' not found"
    exit 1
fi

if [ ! -f "$TEST_FILE" ]; then
    echo "Error: test file '$TEST_FILE' not found"
    exit 1
fi

passed=0
failed=0
test_num=1

trim() {
    sed 's/\r$//' | sed 's/^[[:space:]]*//;s/[[:space:]]*$//'
}

is_number() {
    [[ "$1" =~ ^-?[0-9]+([.][0-9]+)?$ ]]
}

numbers_equal() {
    awk -v a="$1" -v b="$2" 'BEGIN {
        diff = a - b
        if (diff < 0) diff = -diff
        exit(diff <= 0.000000001 ? 0 : 1)
    }'
}

while IFS= read -r expression || [ -n "$expression" ]; do
    expected=""

    if ! IFS= read -r expected; then
        if [ -z "$expected" ]; then
            echo "Error: missing expected result after expression:"
            echo "  $expression"
            exit 1
        fi
    fi

    expression=$(printf "%s" "$expression" | trim)
    expected=$(printf "%s" "$expected" | trim)

    output=$("$CALCULATOR" "$expression" 2>&1)
    status=$?

    actual=$(printf "%s\n" "$output" | tail -n 1 | trim)

    passed_this_test=false

    if [ "$expected" = "fail" ]; then
        if [ "$status" -ne 0 ]; then
            passed_this_test=true
        elif echo "$actual" | grep -Eiq "fail|error|division|zero"; then
            passed_this_test=true
        fi
    elif is_number "$expected" && is_number "$actual"; then
        if numbers_equal "$expected" "$actual"; then
            passed_this_test=true
        fi
    else
        if [ "$actual" = "$expected" ]; then
            passed_this_test=true
        fi
    fi

    if [ "$passed_this_test" = true ]; then
        echo "[PASS] Test $test_num"
        passed=$((passed + 1))
    else
        echo "[FAIL] Test $test_num"
        echo "  Expression: $expression"
        echo "  Expected:   $expected"
        echo "  Got:        $actual"
        failed=$((failed + 1))
    fi

    test_num=$((test_num + 1))
done < "$TEST_FILE"

echo
echo "Passed: $passed"
echo "Failed: $failed"

if [ "$failed" -ne 0 ]; then
    exit 1
fi