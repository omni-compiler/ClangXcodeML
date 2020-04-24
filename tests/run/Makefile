.PHONY: check clean clangxml
.PRECIOUS: \
	%.c.xcodeml \
	%.c.xml \
	%.cpp.xml \
	%.cpp.xcodeml \
	%.dst.cpp
.DELETE_ON_ERROR:

all: check

ROOTDIR = ../..
XCODEMLTOCXXDIR = $(ROOTDIR)/XcodeMLtoCXX
CXXTOXCODEML = $(ROOTDIR)/CXXtoXcodeML/src/CXXtoXcodeML
XCODEMLTOCXX = $(XCODEMLTOCXXDIR)/XcodeMLtoCXX
DURATION = 5s

%.txt: %.out
	timeout $(DURATION) ./$< > $@

# C tests
# .src.c -> .src.out -> .src.txt
# .src.c -> .c.xcodeml -> .dst.c -> .dst.out -> .dst.txt

CC = /usr/local/bin/clang90
CFLAGS = -pedantic -fsyntax-only
CTOXCODEMLFLAGS = -- -I.

CTESTCASES = $(wildcard *.src.c)
CTESTOBJECTS = $(CTESTCASES:.src.c=.dst.c)
CTESTANSWERS = $(CTESTCASES:.src.c=.src.txt)
CTESTOUTPUTTEXTS = $(CTESTCASES:.src.c=.dst.txt)

%.c.xcodeml: %.src.c
	$(CXXTOXCODEML) $< $(CTOXCODEMLFLAGS) \
		> $@

%.dst.c: %.c.xcodeml
	$(XCODEMLTOCXX) $< > $@

%.out: %.c
	$(CC) $< -o $@


# C++ tests
# .src.cpp -> .src.out -> .src.txt
# .src.cpp -> .cpp.xcodeml -> .dst.cpp -> .dst.out -> .dst.txt

CXX = /usr/local/bin/clang++90
CXXFLAGS = -pedantic -std=c++11
CXXTOXCODEMLFLAGS = -- -I. $(CXXFLAGS)

CXXTESTCASES = $(wildcard *.src.cpp)
CXXTESTOBJECTS = $(CXXTESTCASES:.src.cpp=.dst.cpp)
CXXTESTANSWERS = $(CXXTESTCASES:.src.cpp=.src.txt)
CXXTESTOUTPUTTEXTS = $(CXXTESTCASES:.src.cpp=.dst.txt)

%.cpp.xcodeml: %.src.cpp
	$(CXXTOXCODEML) $< $(CXXTOXCODEMLFLAGS) \
		> $@

%.dst.cpp: %.cpp.xcodeml
	$(XCODEMLTOCXX) $< > $@

%.out: %.cpp
	$(CXX) $(CXXFLAGS) $< -o $@


check: $(CXXTESTOUTPUTTEXTS) $(CTESTOUTPUTTEXTS) $(CXXTESTANSWERS) $(CTESTANSWERS)
	set -e; \
	for file in *.dst.txt; do \
		diff -u $$file $${file%.dst.txt}.src.txt; \
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
