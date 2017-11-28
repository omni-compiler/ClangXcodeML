.PHONY: all clean

all: XcodeML_CXX.docx ClangXML.docx


%.docx: %.md reference.docx
	pandoc \
		--filter pandoc-crossref \
		--toc \
		--reference-docx=reference.docx\
		$< -o $@

%.html: %.md
	pandoc \
		-s \
		--filter pandoc-crossref \
		--toc \
		--number-sections \
		$< -o $@
