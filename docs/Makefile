.PHONY: all clean

all: XcodeML_CXX.docx ClangXML.docx


%.docx: %.md reference.docx
	pandoc \
		--filter pandoc-crossref \
		--from=markdown_github+yaml_metadata_block \
		--toc \
		--reference-docx=reference.docx\
		$< -o $@

%.html: %.md
	pandoc \
		-s \
		--filter pandoc-crossref \
		--from=markdown_github+yaml_metadata_block \
		--toc \
		--number-sections \
		$< -o $@
