.PHONY: check clean
.PRECIOUS: %.cpp.xcodeml %.c.xcodeml
.DELETE_ON_ERROR:

all: check

ROOTDIR = ../..
XCODEMLTOCXXDIR = $(ROOTDIR)/XcodeMLtoCXX
CC = /usr/local/bin/clang
CFLAGS = -pedantic -fsyntax-only
CXX = /usr/local/bin/clang++
CXXFLAGS = -pedantic -fsyntax-only
CTOXCODEML = $(ROOTDIR)/bin/CXXtoXcodeML
CXXTOXCODEML = $(ROOTDIR)/bin/CXXtoXcodeML
CTOXCODEMLFLAGS = --
CXXTOXCODEMLFLAGS = --
XCODEMLTOCXX = $(XCODEMLTOCXXDIR)/XcodeMLtoCXX
CTESTCASES = $(wildcard *.src.c)
CTESTOBJECTS = $(CTESTCASES:.src.c=.dst.c)
CXXTESTCASES = $(wildcard *.src.cpp)
CXXTESTOBJECTS = $(CXXTESTCASES:.src.cpp=.dst.cpp)

%.c.xcodeml: %.src.c
	$(CTOXCODEML) $< $(CTOXCODEMLFLAGS) \
		> $@

%.dst.c: %.c.xcodeml
	$(XCODEMLTOCXX) $< > $@

%.cpp.xcodeml: %.src.cpp
	$(CXXTOXCODEML) $< $(CXXTOXCODEMLFLAGS) \
		> $@

%.dst.cpp: %.cpp.xcodeml
	$(XCODEMLTOCXX) $< > $@

check: $(CXXTESTOBJECTS) $(CTESTOBJECTS)
	set -e; \
	for testobj in $(CXXTESTOBJECTS); do  \
		$(CXX) $(CXXFLAGS) $$testobj; \
	done
	set -e; \
	for testobj in $(CTESTOBJECTS); do  \
		$(CC) $(CFLAGS) $$testobj; \
	done

clean:
	rm -f *.dst.c *.dst.cpp *.xcodeml
