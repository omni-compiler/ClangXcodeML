.PHONY: check

tmp.xml : $(wildcard *.cpp)
	../../src/CtoXcodeML $< -- > $@

check : tmp.xml
	diff --brief --ignore-matching-lines="<XcodeProgram.*>" tmp.xml output.xml
