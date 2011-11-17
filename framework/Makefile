# Author: Hongliang Gao;   Created: Jan 27 2011
# Description: Makefile for building cbp3.
# DO NOT MODIFY. THIS FILE WILL NOT BE SUBMITTED WITH YOUR PREDICTOR

# 32 for 32-bit system and 64 for 64-bit system
FORMAT = 64

CXXFLAGS = -O3 -Wall -Ireaderlib
CXX = g++

objects = cbp3_framework.o predictor.o
SRCDIR = predictors
OUTDIR = bin
HEADERS = cbp3_framework.h ./readerlib/cbp3_reader.h ./readerlib/cbp3_def.h predictors.h

all : $(notdir $(wildcard $(SRCDIR)/*))

% : $(SRCDIR)/%/predictor.cc $(SRCDIR)/%/predictor.h
	mkdir -p $(OUTDIR)
	cp $(SRCDIR)/$@/predictor.cc ./
	cp $(SRCDIR)/$@/predictor.h ./
	make cbp3
	mv cbp3 $(OUTDIR)/$@

cbp3 :
	$(CXX) -c $(CXXFLAGS) -o cbp3_framework.o cbp3_framework.cc
	$(CXX) -c $(CXXFLAGS) -o predictor.o predictor.cc
	$(CXX) -o cbp3 cbp3_framework.o predictor.o -Lreaderlib -lcbp3_trace.$(FORMAT)


.PHONY : clean
clean :
	rm -f $(tmp_out_name) $(objects)
	rm -f $(OUTDIR)/*
	rm -f $(src_file_name).cc
	rm -f $(src_file_name).h

