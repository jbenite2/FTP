#!/usr/bin/env bash
rm -rf /autograder/submission/*
chmod -R 777 /autograder/source/reference-implementation/client
chmod -R 777 /autograder/source/reference-implementation/server
cp -r submission/* /autograder/submission/
bash /autograder/run_autograder
# cp /autograder/results/* /vagrant/results/
#rm /autograder/results/*
rm -rf /autograder/testing*
