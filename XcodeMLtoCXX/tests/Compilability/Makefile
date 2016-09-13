.SUFFIXES: .CXXasXcodeML
.PHONY: test clean
.PRECIOUS: %.CXXasXcodeML

all: test

ROOTDIR = ../../..
CTOXCODEMLDIR = $(ROOTDIR)/CtoXcodeML/src
XCODEMLTOCXXDIR = $(ROOTDIR)/XcodeMLtoCXX
CXX = /usr/local/bin/clang++
CXXFLAGS = -pedantic -fsyntax-only
CTOXCODEML = $(CTOXCODEMLDIR)/CtoXcodeML
XCODEMLTOCXX = $(XCODEMLTOCXXDIR)/XcodeMLtoCXX
CXXTESTCASES = $(wildcard *.src.cpp)
CXXTESTOBJECTS = $(CXXTESTCASES:.src.cpp=.dst.cpp)

$(CTOXCODEML):
	$(MAKE) -C $(CTOXCODEMLDIR)

$(XCODEMLTOCXX):
	$(MAKE) -C $(XCODEMLTOCXXDIR)

%.XcodeML: %.c $(CTOXCODEML)
	$(CTOXCODEML) $< -- > $@

%.CXXasXcodeML: %.src.cpp $(CTOXCODEML)
	$(CTOXCODEML) $< -- > $@

%.dst.cpp: %.CXXasXcodeML $(XCODEMLTOCXX)
	$(XCODEMLTOCXX) $< > $@

check: $(CTOXCODEML) $(XCODEMLTOCXX) $(CXXTESTOBJECTS)
	set -e; \
	for testobj in $(CXXTESTOBJECTS); do  \
		$(CXX) $(CXXFLAGS) $$testobj; \
	done

clean:
	rm -f *.dst.cpp *.XcodeML
