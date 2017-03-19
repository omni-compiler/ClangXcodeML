.SUFFIXES: .xcodeml
.PHONY: check clean install-cc
.DELETE_ON_ERROR:

all: check

ROOTDIR = ../..
CXXTOXML = $(ROOTDIR)/bin/CXXtoXcodeML
CXXTOXMLFLAGS = --
XCODEMLTOCXXDIR = $(ROOTDIR)/XcodeMLtoCXX
XCODEMLTOCXX = $(XCODEMLTOCXXDIR)/XcodeMLtoCXX
CXXTESTCASES = $(wildcard *.src.cpp)
CXXDECOMPTESTOBJECTS = $(CXXTESTCASES:.src.cpp=.dst.cpp)
CXXFLAGS = -pedantic -fsyntax-only

%.xcodeml: %.src.cpp
	$(CXXTOXML) $< $(CXXTOXMLFLAGS) \
		> $@

%.dst.cpp: %.xcodeml
	$(XCODEMLTOCXX) $< > $@

install-cc:
	for file in *.cc; do \
		mv $$file $${file%.cc}.src.cpp; \
	done

check: $(CXXDECOMPTESTOBJECTS)
	set -e; \
	for testobj in $(CXXDECOMPTESTOBJECTS); do  \
		$(CXX) $(CXXFLAGS) $$testobj; \
	done

clean:
	rm -f *.xml *.dst.cpp *.xcodeml
