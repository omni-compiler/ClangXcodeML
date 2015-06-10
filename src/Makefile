LLVM_CONFIG = /usr/local/bin/llvm-config
CXX = /usr/local/bin/clang++
#LLVM_CONFIG = /usr/bin/llvm-config-3.6
#CXX = /usr/bin/clang++-3.6
CXXFLAGS = -O2 -g -Wall -Wextra -fno-rtti -std=c++11 -pedantic \
	`pkg-config --cflags libxml-2.0` \
	-D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS

USEDLIBS = -L$(shell $(LLVM_CONFIG) --libdir)
USEDLIBS += -lclangTooling -lclangFrontend -lclangSerialization -lclangDriver \
            -lclangParse -lclangSema -lclangAnalysis \
            -lclangAST -lclangASTMatchers -lclangEdit -lclangLex -lclangBasic
USEDLIBS += $(shell $(LLVM_CONFIG) --libs mcparser bitreader support mc option)
USEDLIBS += -lpthread -ldl -ltinfo -lz
USEDLIBS += `pkg-config --libs libxml-2.0` -lclang
USEDLIBS += $(OTHERLIBS)

OBJS = CtoXcodeML.o \
	XcodeMlVisitorBase.o \
	XcodeMlSymbolsVisitor.o \
	XcodeMlTypeTableVisitor.o \
	XcodeMlDeclarationsVisitor.o

CtoXcodeML: $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) $(USEDLIBS) -o CtoXcodeML

CtoXcodeML.o: CtoXcodeML.cpp XcodeMlVisitorBase.h \
	XcodeMlSymbolsVisitor.h \
	XcodeMlTypeTableVisitor.h \
	XcodeMlDeclarationsVisitor.h

XcodeMlVisitorBase.o: XcodeMlVisitorBase.cpp XcodeMlVisitorBase.h

XcodeMlSymbolsVisitor.o: XcodeMlSymbolsVisitor.cpp \
	XcodeMlSymbolsVisitor.h XcodeMlVisitorBase.h

XcodeMlTypeTableVisitor.o: XcodeMlTypeTableVisitor.cpp \
	XcodeMlTypeTableVisitor.h XcodeMlVisitorBase.h

XcodeMlDeclarationsVisitor.o: XcodeMlDeclarationsVisitor.cpp \
	XcodeMlDeclarationsVisitor.h XcodeMlVisitorBase.h

CtoXcodeML.ast: CtoXcodeML.cpp
	$(CXX) $(CXXFLAGS) -Xclang -ast-dump CtoXcodeML.cpp -o CtoXcodeML.ast 

test:
	./CtoXcodeML --lineno CtoXcodeML.cpp -- $(CXXFLAGS)

mockup: mockup.cpp

clean:
	rm -f CtoXcodeML mockup RAV.i CtoXcodeML.i  $(OBJS) *~

RAV.i: RAV.cpp
	$(CXX) $(CXXFLAGS) -E RAV.cpp -o RAV.i

CtoXcodeML.i: CtoXcodeML.cpp
	$(CXX) $(CXXFLAGS) -E CtoXcodeML.cpp -o CtoXcodeML.i

.PHONY: check-syntax
check-syntax:
	$(CXX) $(CXXFLAGS) -Wall -Wextra -pedantic -fsyntax-only $(CHK_SOURCES)
