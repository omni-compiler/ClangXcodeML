.PHONY: check clean clangxml
.PRECIOUS: %.cpp.xcodeml %.c.xcodeml %.cpp.xml %.c.xml
.DELETE_ON_ERROR:

all: check

ROOTDIR = ../..
XCODEMLTOCXXDIR = $(ROOTDIR)/XcodeMLtoCXX
CC = /usr/local/bin/clang
CFLAGS = -pedantic -fsyntax-only
CXX = /usr/local/bin/clang++
CXXFLAGS = -pedantic -fsyntax-only
CTOXCODEML = $(ROOTDIR)/scripts/CXXtoXcodeML
CXXTOXCODEML = $(ROOTDIR)/scripts/CXXtoXcodeML
CTOXCODEMLFLAGS = --
CXXTOXCODEMLFLAGS = -- -I.
CTOXMLFLAGS = --
CXXTOXMLFLAGS = --
XCODEMLTOCXX = $(XCODEMLTOCXXDIR)/XcodeMLtoCXX
CTESTCASES = $(wildcard *.src.c)
CTESTOBJECTS = $(CTESTCASES:.src.c=.dst.c)

# .src.cpp -> .cpp.xcodeml -> .dst.cpp -> .cpp.out -> .cpp.out
CXXTESTCASES = $(wildcard *.src.cpp)
CXXTESTOBJECTS = $(CXXTESTCASES:.src.cpp=.dst.cpp)
CXXTESTEXECUTABLES = $(CXXTESTCASES:.src.cpp=.cpp.out)
CXXTESTOUTPUTTEXTS = $(CXXTESTCASES:.src.cpp=.cpp.txt)
CLANGXMLOBJECTS = $(CTESTCASES:.src.c=.c.xml) $(CXXTESTCASES:.src.cpp=.cpp.xml)

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

%.cpp.out: %.dst.cpp
	$(CXX) $< -o $@

%.cpp.txt: %.cpp.out
	./$< > $@

%.c.xml: %.src.c
	../../CXXtoXML/src/CXXtoXML $< $(CTOXMLFLAGS) > $@

%.cpp.xml: %.src.cpp
	../../CXXtoXML/src/CXXtoXML $< $(CXXTOXMLFLAGS) > $@

check: $(CXXTESTOUTPUTTEXTS)
	set -e; \
	for file in *.txt; do \
		diff -u $$file $${file%.txt}.sample; \
	done

clangxml: $(CLANGXMLOBJECTS)

clean:
	rm -f *.dst.c *.dst.cpp *.xcodeml *.xml
