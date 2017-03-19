.PHONY: check clean
.PRECIOUS: %.cpp.xcodeml %.c.xcodeml
.DELETE_ON_ERROR:

all: check

ROOTDIR = ../..
CXXTOXMLDIR = $(ROOTDIR)/CXXtoXML/src
XCODEMLTOCXXDIR = $(ROOTDIR)/XcodeMLtoCXX
CC = /usr/local/bin/clang
CFLAGS = -pedantic -fsyntax-only
CXX = /usr/local/bin/clang++
CXXFLAGS = -pedantic -fsyntax-only
CTOXML = $(ROOTDIR)/bin/CXXtoXcodeML
CXXTOXML = $(ROOTDIR)/bin/CXXtoXcodeML
CTOXMLFLAGS = --
CXXTOXMLFLAGS = --
XCODEMLTOCXX = $(XCODEMLTOCXXDIR)/XcodeMLtoCXX
CTESTCASES = $(wildcard *.src.c)
CTESTOBJECTS = $(CTESTCASES:.src.c=.dst.c)
CXXTESTCASES = $(wildcard *.src.cpp)
CXXTESTOBJECTS = $(CXXTESTCASES:.src.cpp=.dst.cpp)
$(CXXTOXML):
	$(MAKE) -C $(CXXTOXMLDIR)

$(XCODEMLTOCXX):
	$(MAKE) -C $(XCODEMLTOCXXDIR)

%.c.xcodeml: %.src.c $(CTOXML)
	$(CTOXML) $< $(CTOXMLFLAGS) \
		> $@

%.dst.c: %.c.xcodeml $(XCODEMLTOCXX)
	$(XCODEMLTOCXX) $< > $@

%.cpp.xcodeml: %.src.cpp $(CXXTOXML)
	$(CXXTOXML) $< $(CXXTOXMLFLAGS) \
		> $@

%.dst.cpp: %.cpp.xcodeml $(XCODEMLTOCXX)
	$(XCODEMLTOCXX) $< > $@

check: $(CXXTOXML) $(XCODEMLTOCXX) $(CXXTESTOBJECTS) $(CTESTOBJECTS)
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
