.SUFFIXES: .XcodeML .CXX
.PHONY: test clean
.PRECIOUS: %.XcodeML

all: test

ROOTDIR = ../../..
CTOXCODEMLDIR = $(ROOTDIR)/CtoXcodeML/src
XCODEMLTOCXXDIR = $(ROOTDIR)/XcodeMLtoCXX/src
CXX = /usr/local/bin/clang++
CXXFLAGS = -pedantic -fsyntax-only
CTOXCODEML = $(CTOXCODEMLDIR)/CtoXcodeML
XCODEMLTOCXX = $(XCODEMLTOCXXDIR)/XcodeMLtoCXX
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

check: $(CTOXCODEML) $(XCODEMLTOCXX) $(CXXTESTOBJECTS)
	for testobj in $(CXXTESTOBJECTS); do  \
		$(CXX) $(CXXFLAGS) $$testobj; \
	done

clean:
	rm -f *.CXX *.XcodeML
