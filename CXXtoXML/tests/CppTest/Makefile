.SUFFIXES: .xml
.PHONY: test clean

all: test

ROOTDIR = ../../..
CXXTOXMLDIR = $(ROOTDIR)/CXXtoXML/src
CXXTOXML = $(CXXTOXMLDIR)/CXXtoXML
CXXTOXMLFLAGS = --
CXXTESTCASES = $(wildcard *.cc)
CXXTESTOBJECTS = $(CXXTESTCASES:.cc=.xml)

$(CXXTOXML):
	$(MAKE) -C $(CXXTOXMLDIR)

%.xml: %.cc $(CXXTOXML)
	$(CXXTOXML) $< $(CXXTOXMLFLAGS) > $@

check: $(CXXTOXML) $(CXXTESTOBJECTS)

clean:
	rm -f *.xml
