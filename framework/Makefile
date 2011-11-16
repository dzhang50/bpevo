# Author: Hongliang Gao;   Created: Jan 27 2011
# Description: Makefile for building cbp3.
# DO NOT MODIFY. THIS FILE WILL NOT BE SUBMITTED WITH YOUR PREDICTOR

# 32 for 32-bit system and 64 for 64-bit system
FORMAT = 64

CXXFLAGS = -O3 -Wall -Ireaderlib
CXX = g++

objects = cbp3_framework.o predictor.o

cbp3 : $(objects)
	$(CXX) -o cbp3 $(objects) -Lreaderlib -lcbp3_trace.$(FORMAT)
cbp3_framework.o: cbp3_framework.h ./readerlib/cbp3_reader.h ./readerlib/cbp3_def.h predictor.h
predictor.o : ./readerlib/cbp3_def.h cbp3_framework.h predictor.h

.PHONY : clean
clean :
	rm -f cbp3 $(objects)

