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
CTOXCODEML = $(ROOTDIR)/CXXtoXML/src/CXXtoXML
CXXTOXCODEML = $(ROOTDIR)/CXXtoXML/src/CXXtoXML
CTOXCODEMLFLAGS = --
CXXTOXCODEMLFLAGS = --
CTOXMLFLAGS = --
CXXTOXMLFLAGS = --
XCODEMLTOCXX = $(XCODEMLTOCXXDIR)/XcodeMLtoCXX
CTESTCASES = $(wildcard *.src.c)
CTESTOBJECTS = $(CTESTCASES:.src.c=.dst.c)
CXXTESTCASES = $(wildcard *.src.cpp)
CXXTESTOBJECTS = $(CXXTESTCASES:.src.cpp=.dst.cpp)
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

%.c.xml: %.src.c
	../../CXXtoXML/src/CXXtoXML $< $(CTOXMLFLAGS) > $@

%.cpp.xml: %.src.cpp
	../../CXXtoXML/src/CXXtoXML $< $(CXXTOXMLFLAGS) > $@

check: $(CXXTESTOBJECTS) $(CTESTOBJECTS)
	set -e; \
	for testobj in $(CXXTESTOBJECTS); do  \
		$(CXX) $(CXXFLAGS) $$testobj; \
	done
	set -e; \
	for testobj in $(CTESTOBJECTS); do  \
		$(CC) $(CFLAGS) $$testobj; \
	done

clangxml: $(CLANGXMLOBJECTS)

clean:
	rm -f *.dst.c *.dst.cpp *.xcodeml *.xml
