.PHONY: test

tmp.xml : access_specifier.cpp
	../../src/CtoXcodeML $< -- > $@

test : tmp.xml
	diff --brief --ignore-matching-lines="<XcodeProgram.*>" tmp.xml output.xml
