# Makefile for Linux


all: Exe/Echo1 Exe/Performance Exe/Test

INCDIR = ../Inc
API    = ../USBBULK_API_Linux/USBBULK_Linux.c
HDR    = $(INCDIR)/USBBULK.h
LIBS   = -ludev -lpthread

Exe/Echo1: Application/Echo1.c $(API) $(HDR)
	cc -Wall -O -I$(INCDIR) $^ -o $@ $(LIBS)

Exe/Performance: Application/Performance.c $(API) $(HDR)
	cc -Wall -O -I$(INCDIR) $^ -o $@ $(LIBS)

Exe/Test: Application/Test.c $(API) $(HDR)
	cc -Wall -O -I$(INCDIR) $^ -o $@ $(LIBS)
