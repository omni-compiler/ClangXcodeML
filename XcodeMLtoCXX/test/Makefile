.SUFFIXES: .XcodeML .CXX
.PHONY: test clean

CXX = /usr/local/bin/clang++
CXXFLAGS = -pedantic -fsyntax-only
CTOXCODEMLDIR = ../../src/
CTOXCODEML = ../../src/CtoXcodeML
XCODEMLTOCXX = ../XcodeMLtoCXX
CXXTESTCASES = $(wildcard *.cpp)
CXXTESTOBJECTS = $(CXXTESTCASES:.cpp=.CXX)

$(CTOXCODEML):
	$(MAKE) -C $(CTOXCODEMLDIR)

.c.XcodeML:
	$(CTOXCODEML) $< -- > $@

.cpp.XcodeML:
	$(CTOXCODEML) $< -- > $@

.XcodeML.CXX:
	$(XCODEMLTOCXX) $< > $@

test: $(CXXTESTOBJECTS)
	for testobj in $(CXXTESTOBJECTS); do  \
		$(CXX) $(CXXFLAGS) $$testobj; \
	done

clean:
	rm -f $(CXXTESTOBJECTS)
