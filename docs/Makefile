.PHONY: all clean

all: XcodeML_CXX.docx

XcodeML_CXX.docx: reference.docx XcodeML_CXX.md
	pandoc --toc --reference-docx=reference.docx XcodeML_CXX.md -o $@
