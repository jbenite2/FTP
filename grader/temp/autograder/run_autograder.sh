#!/usr/bin/env bash
set -x

# Show README
cat README.md

# Show Dockerfile if present
if [[ -f Dockerfile ]]; then
    cat Dockerfile
fi

# Build code
make all

cat /dev/urandom | head -c 499     > /autograder/file_1k
cat /dev/urandom | head -c 9979    > /autograder/file_10k
cat /dev/urandom | head -c 78017   > /autograder/file_100k
cat /dev/urandom | head -c 908101  > /autograder/file_1M
cat /dev/urandom | head -c 8940501 > /autograder/file_10M

mkdir /autograder/testing
cd /autograder/testing

# Run text-mode tests to see more detailed info about the run (will delay the run...)
# python /autograder/source/run_tests_text.py

python /autograder/source/run_tests.pyc > /autograder/results/"$STUDENT.json"

ls -alh .

cat /autograder/results/"$STUDENT.json"
