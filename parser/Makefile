# Author: Hongliang Gao;   Created: Jan 27 2011
# Description: Makefile for building cbp3.
# DO NOT MODIFY. THIS FILE WILL NOT BE SUBMITTED WITH YOUR PREDICTOR

# 32 for 32-bit system and 64 for 64-bit system
FORMAT = 64

CXXFLAGS = -O3 -Wall -Ireaderlib -std=c++0x
CXX = g++

objects = cbp3_framework.o predictor.o
SRCDIR = predictors
OUTDIR = bin
HEADERS = cbp3_framework.h ./readerlib/cbp3_reader.h ./readerlib/cbp3_def.h predictors.h

all : $(notdir $(wildcard $(SRCDIR)/*))

% : $(SRCDIR)/%/predictor.cc
	mkdir -p $(OUTDIR)
	cp $(SRCDIR)/$@/predictor.cc ./
	make cbp3
	mv cbp3 $(OUTDIR)/$@

cbp3 :
	$(CXX) -c $(CXXFLAGS) -o cbp3_framework.o cbp3_framework.cc
	$(CXX) -c $(CXXFLAGS) -o predictor.o predictor.cc
	$(CXX) -o cbp3 cbp3_framework.o predictor.o -Lreaderlib -lcbp3_trace.$(FORMAT)

java:
	javac -cp . NodeType.java
	javac -cp . Node.java
	java -cp antlr-3.4-complete.jar org.antlr.Tool BPLang.g
	javac -cp .:antlr-3.4-complete.jar BPLangProg.java

gen:
	$(MAKE) java
	java -cp .:antlr-3.4-complete.jar BPLangProg gen 30 60 983
#	mkdir -p predictors/mate
#	java -cp .:antlr-3.4-complete.jar BPLangProg mate predictors/predictor_1/bplang predictors/predictor_3/bplang predictors/mate 1 42 

javaclean:
	-rm -rf *~
	-rm -rf *.class
	-rm -rf \#*
	-rm *Parser.java
	-rm *Lexer.java
	-rm *.tokens


.PHONY : clean
clean :
	rm -f $(objects)
	rm -f $(OUTDIR)/*
	rm -f predictor.cc
	-rm -f *.class
	-rm -f *.tokens
	-rm -f *Lexer*
	-rm -f *Parser*
	-rm -rf predictors
	-rm -f *~
