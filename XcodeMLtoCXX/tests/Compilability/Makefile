.SUFFIXES: .CXXasXcodeML
.PHONY: test clean
.PRECIOUS: %.CXXasXcodeML %.CasXcodeML

all: test

ROOTDIR = ../../..
CTOXCODEMLDIR = $(ROOTDIR)/CtoXcodeML/src
XCODEMLTOCXXDIR = $(ROOTDIR)/XcodeMLtoCXX
CC = /usr/local/bin/clang
CFLAGS = -pedantic -fsyntax-only
CXX = /usr/local/bin/clang++
CXXFLAGS = -pedantic -fsyntax-only
CTOXCODEML = $(CTOXCODEMLDIR)/CtoXcodeML
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
	$(CTOXCODEML) $< -- > $@

%.dst.c: %.CasXcodeML $(XCODEMLTOCXX)
	$(XCODEMLTOCXX) $< > $@

%.CXXasXcodeML: %.src.cpp $(CTOXCODEML)
	$(CTOXCODEML) $< -- > $@

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
	rm -f *.dst.cpp *.XcodeML
