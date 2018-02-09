.PHONY: check clean clangxml
.PRECIOUS: %.cpp.xcodeml %.c.xcodeml %.cpp.xml %.c.xml
.DELETE_ON_ERROR:

all: check

ROOTDIR = ../..
XCODEMLTOCXXDIR = $(ROOTDIR)/XcodeMLtoCXX
CXXTOXCODEML = $(ROOTDIR)/scripts/CXXtoXcodeML
XCODEMLTOCXX = $(XCODEMLTOCXXDIR)/XcodeMLtoCXX

# C tests
# .src.c -> .c.xcodeml -> .dst.c -> .c.out -> .c.out

CC = /usr/local/bin/clang
CFLAGS = -pedantic -fsyntax-only
CTOXCODEMLFLAGS = -- -I.

CTESTCASES = $(wildcard *.src.c)
CTESTOBJECTS = $(CTESTCASES:.src.c=.dst.c)
CTESTEXECUTABLES = $(CTESTCASES:.src.c=.c.out)
CTESTOUTPUTTEXTS = $(CTESTCASES:.src.c=.c.txt)

%.c.xcodeml: %.src.c
	$(CXXTOXCODEML) $< $(CTOXCODEMLFLAGS) \
		> $@

%.dst.c: %.c.xcodeml
	$(XCODEMLTOCXX) $< > $@

%.c.out: %.dst.c
	$(CC) $< -o $@

%.c.txt: %.c.out
	./$< > $@


# C++ tests
# .src.cpp -> .cpp.xcodeml -> .dst.cpp -> .cpp.out -> .cpp.out

CXX = /usr/local/bin/clang++
CXXFLAGS = -pedantic -fsyntax-only
CXXTOXCODEMLFLAGS = -- -I.

CXXTESTCASES = $(wildcard *.src.cpp)
CXXTESTOBJECTS = $(CXXTESTCASES:.src.cpp=.dst.cpp)
CXXTESTEXECUTABLES = $(CXXTESTCASES:.src.cpp=.cpp.out)
CXXTESTOUTPUTTEXTS = $(CXXTESTCASES:.src.cpp=.cpp.txt)

%.cpp.xcodeml: %.src.cpp
	$(CXXTOXCODEML) $< $(CXXTOXCODEMLFLAGS) \
		> $@

%.dst.cpp: %.cpp.xcodeml
	$(XCODEMLTOCXX) $< > $@

%.cpp.out: %.dst.cpp
	$(CXX) $< -o $@

%.cpp.txt: %.cpp.out
	./$< > $@


check: $(CXXTESTOUTPUTTEXTS) $(CTESTOUTPUTTEXTS)
	set -e; \
	for file in *.txt; do \
		diff -u $$file $${file%.txt}.sample; \
	done

clean:
	rm -f *.dst.c *.dst.cpp *.out *.txt *.xcodeml *.xml

CLANGXMLOBJECTS = $(CTESTCASES:.src.c=.c.xml) $(CXXTESTCASES:.src.cpp=.cpp.xml)
CTOXMLFLAGS = --
CXXTOXMLFLAGS = --

%.c.xml: %.src.c
	../../CXXtoXML/src/CXXtoXML $< $(CTOXMLFLAGS) > $@

%.cpp.xml: %.src.cpp
	../../CXXtoXML/src/CXXtoXML $< $(CXXTOXMLFLAGS) > $@

clangxml: $(CLANGXMLOBJECTS)
