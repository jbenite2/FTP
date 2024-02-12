.PHONY: all

all: setup.sh run_autograder run_tests.py tests/*
	zip -r autograder.zip *

clean:
	rm -Rf autograder.zip tests/*.pyc *.pyc
	make -C reference-implementation clean

