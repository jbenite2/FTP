#!/usr/bin/env bash

rm -rf /autograder

cp -r autograder /autograder

apt-get install -y psmisc python python-pip python-dev libboost-all-dev

pip install subprocess32 gradescope-utils

(git clone https://github.com/cawka/gradescope-utils /root/utils; cd /root/utils; python setup.py install)


