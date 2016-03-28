.PHONY: check $(TARGET)

TARGET = tmp.xml
CTOXCODEML = ../../src/CtoXcodeML

$(TARGET) : $(wildcard *.cpp) $(CTOXCODEML)
	$(CTOXCODEML) $< -- -std=c++14 > $@

check : tmp.xml
	diff --brief --ignore-matching-lines="<XcodeProgram.*>" tmp.xml output.xml
