# Makefile

SRC   := cmd common rsp ril ublox
SRC_D := ../
SRC_H := $(addprefix ${SRC_D},$(addsuffix .h,${SRC}))
SRC_C := $(addprefix ${SRC_D},$(addsuffix .c,${SRC}))

all : test

test : ril.so
	python -m unittest discover -v

ril.so : ${SRC_H} ${SRC_C}
	gcc -fPIC -shared -Wall -O -o $@ ${SRC_C}

.PHONY : all test
