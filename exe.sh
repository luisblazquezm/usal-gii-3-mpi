#!/bin/bash

mpicc fixing.c crypting.h -lcrypt -o crypt && mpirun -np 2 crypt

exit 0