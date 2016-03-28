.PHONY: check

tmp.xml : $(wildcard *.cpp)
	../../src/CtoXcodeML $< -- -std=c++14 > $@

check : tmp.xml
	diff --brief --ignore-matching-lines="<XcodeProgram.*>" tmp.xml output.xml
