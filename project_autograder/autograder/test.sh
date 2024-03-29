#!/usr/bin/env bash
set -x

# Prepare student submission
ls -alh /autograder/submission/
export STUDENT=$(ls /autograder/submission/ | xargs basename -s .tar.gz)

cd /autograder/submission
tar zxf "$STUDENT.tar.gz"
if [[ $? != 0 ]]; then
    # There was a stupid bug in the Makefile... :(
    tar xf "$STUDENT.tar.gz"
fi
rm "$STUDENT.tar.gz"
ls -alh

# Show README
cat README.md

# Show Dockerfile if present
if [[ -f Dockerfile ]]; then
    cat Dockerfile
fi

# Build code
make

cat /dev/urandom | head -c 499     > /autograder/file_1k
cat /dev/urandom | head -c 9979    > /autograder/file_10k
cat /dev/urandom | head -c 78017   > /autograder/file_100k
cat /dev/urandom | head -c 908101  > /autograder/file_1M
cat /dev/urandom | head -c 8940501 > /autograder/file_10M

mkdir /autograder/testing
cd /autograder/testing

# Run text-mode tests to see more detailed info about the run (will delay the run...)
# python /autograder/source/run_tests_text.py

python /autograder/source/run_tests.py > /autograder/results/"$STUDENT.json"

ls -alh .

cat /autograder/results/"$STUDENT.json"
