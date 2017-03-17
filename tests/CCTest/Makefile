.SUFFIXES: .xml .xcodeml
.PHONY: check clean
.DELETE_ON_ERROR:

all: test

ROOTDIR = ../..
CXXTOXMLDIR = $(ROOTDIR)/CXXtoXML
CXXTOXML = $(CXXTOXMLDIR)/src/CXXtoXML
CXXTOXMLFLAGS = --
CXXTOXMLXSLTDIR = $(CXXTOXMLDIR)/src/XSLTs
XCODEMLTOCXXDIR = $(ROOTDIR)/XcodeMLtoCXX
XCODEMLTOCXX = $(XCODEMLTOCXXDIR)/XcodeMLtoCXX
CXXTESTCASES = $(wildcard *.cc)
CXXTESTOBJECTS = $(CXXTESTCASES:.cc=.xml)
CXXDECOMPTESTOBJECTS = $(CXXTESTCASES:.cc=.generated.cpp)
CXXFLAGS = -pedantic -fsyntax-only

$(CXXTOXML):
	$(MAKE) -C $(CXXTOXMLDIR)/src

$(XCODEMLTOCXX):
	$(MAKE) -C $(XCODEMLTOCXXDIR)

%.xml: %.cc $(CXXTOXML)
	$(CXXTOXML) $< $(CXXTOXMLFLAGS) > $@

%.xcodeml: %.xml
	xsltproc $(CXXTOXMLXSLTDIR)/drop_prop_column.xsl $< \
		| xsltproc $(CXXTOXMLXSLTDIR)/drop_prop_file.xsl - \
		| xsltproc $(CXXTOXMLXSLTDIR)/drop_prop_lineno.xsl - \
		| xsltproc $(CXXTOXMLXSLTDIR)/reorder_decl.xsl - \
		| xsltproc $(CXXTOXMLXSLTDIR)/add_symbols_elem.xsl - \
		| xsltproc $(CXXTOXMLXSLTDIR)/add_globalsymbols_elem.xsl - \
		| xsltproc $(CXXTOXMLXSLTDIR)/Program2XcodeProgram.xsl - \
		| xmllint --format - \
		> $@

%.generated.cpp: %.xcodeml $(XCODEMLTOCXX)
	$(XCODEMLTOCXX) $< > $@

check: $(CXXTOXML) $(CXXDECOMPTESTOBJECTS)
	set -e; \
	for testobj in $(CXXDECOMPTESTOBJECTS); do  \
		$(CXX) $(CXXFLAGS) $$testobj; \
	done

clean:
	rm -f *.xml *.generated.cpp *.xcodeml
