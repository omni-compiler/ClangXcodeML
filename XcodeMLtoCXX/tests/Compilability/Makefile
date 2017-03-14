.SUFFIXES: .CXXasXcodeML
.PHONY: test clean
.PRECIOUS: %.CXXasXcodeML %.CasXcodeML
.DELETE_ON_ERROR:

all: test

ROOTDIR = ../../..
CXXTOXMLDIR = $(ROOTDIR)/CXXtoXML/src
XCODEMLTOCXXDIR = $(ROOTDIR)/XcodeMLtoCXX
CXXTOXMLXSLTDIR = $(CXXTOXMLDIR)/XSLTs
CC = /usr/local/bin/clang
CFLAGS = -pedantic -fsyntax-only
CXX = /usr/local/bin/clang++
CXXFLAGS = -pedantic -fsyntax-only
CXXTOXML = $(CXXTOXMLDIR)/CXXtoXML
CXXTOXMLFLAGS = --
XCODEMLTOCXX = $(XCODEMLTOCXXDIR)/XcodeMLtoCXX
CTESTCASES = $(wildcard *.src.c)
CTESTOBJECTS = $(CTESTCASES:.src.c=.dst.c)
CXXTESTCASES = $(wildcard *.src.cpp)
CXXTESTOBJECTS = $(CXXTESTCASES:.src.cpp=.dst.cpp)
$(CXXTOXML):
	$(MAKE) -C $(CXXTOXMLDIR)

$(XCODEMLTOCXX):
	$(MAKE) -C $(XCODEMLTOCXXDIR)

%.CasXcodeML: %.src.c $(CXXTOXML)
	$(CXXTOXML) $< $(CXXTOXMLFLAGS) \
	  | xsltproc $(CXXTOXMLXSLTDIR)/drop_prop_column.xsl - \
		| xsltproc $(CXXTOXMLXSLTDIR)/drop_prop_file.xsl - \
		| xsltproc $(CXXTOXMLXSLTDIR)/drop_prop_lineno.xsl - \
		| xsltproc $(CXXTOXMLXSLTDIR)/reorder_decl.xsl - \
		| xsltproc $(CXXTOXMLXSLTDIR)/add_symbols_elem.xsl - \
		| xsltproc $(CXXTOXMLXSLTDIR)/add_globalsymbols_elem.xsl - \
		| xsltproc $(CXXTOXMLXSLTDIR)/Program2XcodeProgram.xsl - \
		| xmllint --format - \
		> $@

%.dst.c: %.CasXcodeML $(XCODEMLTOCXX)
	$(XCODEMLTOCXX) $< > $@

%.CXXasXcodeML: %.src.cpp $(CXXTOXML)
	$(CXXTOXML) $< $(CXXTOXMLFLAGS) \
	  | xsltproc $(CXXTOXMLXSLTDIR)/drop_prop_column.xsl - \
		| xsltproc $(CXXTOXMLXSLTDIR)/drop_prop_file.xsl - \
		| xsltproc $(CXXTOXMLXSLTDIR)/drop_prop_lineno.xsl - \
		| xsltproc $(CXXTOXMLXSLTDIR)/reorder_decl.xsl - \
		| xsltproc $(CXXTOXMLXSLTDIR)/add_symbols_elem.xsl - \
		| xsltproc $(CXXTOXMLXSLTDIR)/add_globalsymbols_elem.xsl - \
		| xsltproc $(CXXTOXMLXSLTDIR)/Program2XcodeProgram.xsl - \
		| xmllint --format - \
		> $@

%.dst.cpp: %.CXXasXcodeML $(XCODEMLTOCXX)
	$(XCODEMLTOCXX) $< > $@

check: $(CXXTOXML) $(XCODEMLTOCXX) $(CXXTESTOBJECTS) $(CTESTOBJECTS)
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
