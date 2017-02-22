.SUFFIXES: .CXXasXcodeML
.PHONY: test clean
.PRECIOUS: %.CXXasXcodeML %.CasXcodeML
.DELETE_ON_ERROR:

all: test

ROOTDIR = ../../..
CTOXCODEMLDIR = $(ROOTDIR)/CtoXcodeML/src
XCODEMLTOCXXDIR = $(ROOTDIR)/XcodeMLtoCXX
CC = /usr/local/bin/clang
CFLAGS = -pedantic -fsyntax-only
CXX = /usr/local/bin/clang++
CXXFLAGS = -pedantic -fsyntax-only
CTOXCODEML = $(CTOXCODEMLDIR)/CtoXcodeML
CTOXCODEMLFLAGS = --
XCODEMLTOCXX = $(XCODEMLTOCXXDIR)/XcodeMLtoCXX
CTESTCASES = $(wildcard *.src.c)
CTESTOBJECTS = $(CTESTCASES:.src.c=.dst.c)
CXXTESTCASES = $(wildcard *.src.cpp)
CXXTESTOBJECTS = $(CXXTESTCASES:.src.cpp=.dst.cpp)

$(CTOXCODEML):
	$(MAKE) -C $(CTOXCODEMLDIR)

$(XCODEMLTOCXX):
	$(MAKE) -C $(XCODEMLTOCXXDIR)

%.CasXcodeML: %.src.c $(CTOXCODEML)
	$(CTOXCODEML) $< $(CTOXCODEMLFLAGS) > $@

%.dst.c: %.CasXcodeML $(XCODEMLTOCXX)
	$(XCODEMLTOCXX) $< > $@

%.CXXasXcodeML: %.src.cpp $(CTOXCODEML)
	$(CTOXCODEML) $< $(CTOXCODEMLFLAGS) > $@

%.dst.cpp: %.CXXasXcodeML $(XCODEMLTOCXX)
	$(XCODEMLTOCXX) $< > $@

check: $(CTOXCODEML) $(XCODEMLTOCXX) $(CXXTESTOBJECTS) $(CTESTOBJECTS)
	set -e; \
	for testobj in $(CXXTESTOBJECTS); do  \
		$(CXX) $(CXXFLAGS) $$testobj; \
	done
	set -e; \
	for testobj in $(CTESTOBJECTS); do  \
		$(CC) $(CFLAGS) $$testobj; \
	done

clean:
	rm -f *.dst.c *.dst.cpp *.CasXcodeML *.CXXasXcodeML
