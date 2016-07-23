.SUFFIXES: .XcodeML .CXX
.PHONY: test clean

all: test

CXX = /usr/local/bin/clang++
CXXFLAGS = -pedantic -fsyntax-only
CTOXCODEMLDIR = ../../src/
CTOXCODEML = ../../src/CtoXcodeML
XCODEMLTOCXXDIR = ../
XCODEMLTOCXX = ../XcodeMLtoCXX
CXXTESTCASES = $(wildcard *.cpp)
CXXTESTOBJECTS = $(CXXTESTCASES:.cpp=.CXX)

$(CTOXCODEML):
	$(MAKE) -C $(CTOXCODEMLDIR)

$(XCODEMLTOCXX):
	$(MAKE) -C $(XCODEMLTOCXXDIR)

.c.XcodeML:
	$(CTOXCODEML) $< -- > $@

.cpp.XcodeML: $(CTOXCODEML)
	$(CTOXCODEML) $< -- > $@

.XcodeML.CXX: $(XCODEMLTOCXX)
	$(XCODEMLTOCXX) $< > $@

test: $(CTOXCODEML) $(XCODEMLTOCXX) $(CXXTESTOBJECTS)
	for testobj in $(CXXTESTOBJECTS); do  \
		$(CXX) $(CXXFLAGS) $$testobj; \
	done

clean:
	rm -f *.CXX *.XcodeML
