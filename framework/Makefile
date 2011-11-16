# Author: Hongliang Gao;   Created: Jan 27 2011
# Description: Makefile for building cbp3.
# DO NOT MODIFY. THIS FILE WILL NOT BE SUBMITTED WITH YOUR PREDICTOR

# 32 for 32-bit system and 64 for 64-bit system
FORMAT = 64

CXXFLAGS = -O3 -Wall -Ireaderlib
CXX = g++

objects = cbp3_framework.o $(src_file_name).o
SRCDIR = predictors
OUTDIR = bin

src_file_name = predictor
tmp_out_name = cbp3

all : $(notdir $(wildcard $(SRCDIR)/*))

% : $(SRCDIR)/%/$(src_file_name).cc $(SRCDIR)/%/$(src_file_name).h
	mkdir -p $(OUTDIR)
	cp -p $(SRCDIR)/$@/$(src_file_name).cc ./
	cp -p $(SRCDIR)/$@/$(src_file_name).h ./
	make $(tmp_out_name)
	mv $(tmp_out_name) $(OUTDIR)/$@
$(tmp_out_name) : $(objects)
	$(CXX) -o $(tmp_out_name) $(objects) -Lreaderlib -lcbp3_trace.$(FORMAT)
cbp3_framework.o: cbp3_framework.h ./readerlib/cbp3_reader.h ./readerlib/cbp3_def.h $(src_file_name).h
$(src_file_name).o : ./readerlib/cbp3_def.h cbp3_framework.h $(src_file_name).h

.PHONY : clean
clean :
	rm -f $(tmp_out_name) $(objects)
	rm -f $(OUTDIR)/*
	rm -f $(src_file_name).cc
	rm -f $(src_file_name).h

