.SUFFIXES: .xml .xcodeml
.PHONY: test clean
.DELETE_ON_ERROR:

all: test

ROOTDIR = ../../..
CXXTOXMLDIR = $(ROOTDIR)/CXXtoXML
CXXTOXML = $(CXXTOXMLDIR)/src/CXXtoXML
CXXTOXMLFLAGS = --
CXXTOXMLXSLTDIR = $(CXXTOXMLDIR)/src/XSLTs
CXXTESTCASES = $(wildcard *.cc)
CXXTESTOBJECTS = $(CXXTESTCASES:.cc=.xml)

$(CXXTOXML):
	$(MAKE) -C $(CXXTOXMLDIR)

%.xml: %.cc $(CXXTOXML)
	$(CXXTOXML) $< $(CXXTOXMLFLAGS) > $@

%.xcodeml: %.xml
	xsltproc $(CXXTOXMLXSLTDIR)/drop_prop_column.xsl $< \
		| xsltproc $(CXXTOXMLXSLTDIR)/drop_prop_file.xsl - \
		| xsltproc $(CXXTOXMLXSLTDIR)/drop_prop_lineno.xsl - \
		| xsltproc $(CXXTOXMLXSLTDIR)/Program2XcodeProgram.xsl - \
		| xmllint --format - \
		> $@

check: $(CXXTOXML) $(CXXTESTOBJECTS)

clean:
	rm -f *.xml
