.SUFFIXES: .xcodeml
.PHONY: check clean prepare-cpptest check-cpptest clean-cpptest build
.DELETE_ON_ERROR:

all: check

ROOTDIR = ../..
CXXTOXML = $(ROOTDIR)/CXXtoXML/src/CXXtoXML
CXXTOXML_INCLUDES = -I.
CXXTOXMLFLAGS = -- -Wall -W -pedantic $(CXXTOXML_INCLUDES)
XCODEMLTOCXXDIR = $(ROOTDIR)/XcodeMLtoCXX
XCODEMLTOCXX = $(XCODEMLTOCXXDIR)/XcodeMLtoCXX
CXXTESTCASES = $(wildcard *.src.cpp)
CXXDECOMPTESTOBJECTS = $(CXXTESTCASES:.src.cpp=.dst.cpp)
CXXCLANGXMLOBJECTS = $(CXXTESTCASES:.src.cpp=.xml)
CXXFLAGS = -pedantic -fsyntax-only

CPPTEST_TARBALL = ../../../CppTest.tar.gz
CPPTEST_EXTRACT_DIR = CppTest
TESTOBJECTS_TXT = testobjects.txt
EXAMINE_SH = ./examine.sh

%.xml: %.src.cpp
	../../CXXtoXML/src/CXXtoXML $< $(CXXTOXMLFLAGS) > $@

%.xcodeml: %.src.cpp
	$(CXXTOXML) $< $(CXXTOXMLFLAGS) \
		> $@

%.dst.cpp: %.xcodeml
	$(XCODEMLTOCXX) $< > $@

check: $(CXXDECOMPTESTOBJECTS)
	set -e; \
	for testobj in $(CXXDECOMPTESTOBJECTS); do  \
		$(CXX) $(CXXFLAGS) $$testobj; \
	done

clean:
	rm -rf *.xml *.dst.cpp *.xcodeml

clangxml: $(CXXCLANGXMLOBJECTS)

build:
	(cd $(ROOTDIR)/CXXtoXML/src; gmake clean; gmake)
	(cd $(ROOTDIR)/XcodeMLtoCXX/src; gmake clean; gmake)

prepare-cpptest: $(CPPTEST_TARBALL) $(TESTOBJECTS_TXT)
	tar xvf $(CPPTEST_TARBALL)
	@cat $(TESTOBJECTS_TXT) | while read line; do \
	   sed 's/^main/int main/' $(CPPTEST_EXTRACT_DIR)/$${line}.cc \
	      > $${line}.src.cpp; \
	done

check-cpptest:
	@cat $(TESTOBJECTS_TXT) | while read line; do \
	  sh $(EXAMINE_SH) $${line}; \
	done 2> err.log | tee result.csv

clean-cpptest:
	if [ -d $(CPPTEST_EXTRACT_DIR) ]; then \
	   (cd $(CPPTEST_EXTRACT_DIR); for file in *; do \
	       rm -f $$file ../$${file%.cc}.src.cpp \
	         ../$${file%.cc}.xml ../$${file%.cc}.xcodeml \
	         ../$${file%.cc}.dst.cpp ../$${file%.cc}.out; \
	    done); rmdir $(CPPTEST_EXTRACT_DIR); \
	 fi
	rm -f err.log result.csv
