LLVM_CONFIG = /usr/local/bin/llvm-config
CXX = /usr/local/bin/clang++
#LLVM_CONFIG = /usr/bin/llvm-config-3.6
#CXX = /usr/bin/clang++-3.6
CXXFLAGS = -O2 -g -Wall -Wextra -fno-rtti -std=c++11 -pedantic \
	`pkg-config --cflags libxml-2.0` \
	-D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS $(PCHFLAGS)
#PCHFLAGS = -include XcodeMlVisitorBase.h
PCHFILES = XcodeMlVisitorBase.pch

USEDLIBS = -L$(shell $(LLVM_CONFIG) --libdir)
USEDLIBS += -lclangTooling -lclangFrontend -lclangSerialization -lclangDriver \
            -lclangParse -lclangSema -lclangAnalysis \
            -lclangAST -lclangASTMatchers -lclangEdit -lclangLex -lclangBasic
USEDLIBS += $(shell $(LLVM_CONFIG) --libs mcparser bitreader support mc option)
USEDLIBS += -lpthread -ldl -ltinfo -lz
USEDLIBS += `pkg-config --libs libxml-2.0` -lclang
USEDLIBS += $(OTHERLIBS)

RAVOBJS = XcodeMlRAV.o
OBJS =  CtoXcodeML.o \
	XcodeMlVisitorBase.o \
	SymbolsVisitor.o \
	TypeTableVisitor.o \
	DeclarationsVisitor.o

CtoXcodeML: $(RAVOBJS) $(OBJS)
	$(CXX) $(CXXFLAGS) $(RAVOBJS) $(OBJS) $(USEDLIBS) -o CtoXcodeML

XcodeMlRAV.o: \
	XcodeMlRAV.cpp \
	XcodeMlRAV.h
CtoXcodeML.o: \
	CtoXcodeML.cpp \
	XcodeMlRAV.h \
	XcodeMlVisitorBase.h \
	SymbolsVisitor.h \
	TypeTableVisitor.h \
	DeclarationsVisitor.h
XcodeMlVisitorBase.o: \
	XcodeMlVisitorBase.cpp \
	XcodeMlRAV.h \
	XcodeMlVisitorBase.h
SymbolsVisitor.o: \
	SymbolsVisitor.cpp \
	SymbolsVisitor.h \
	XcodeMlRAV.h \
	XcodeMlVisitorBase.h
TypeTableVisitor.o: \
	TypeTableVisitor.cpp \
	TypeTableVisitor.h \
	XcodeMlRAV.h \
	XcodeMlVisitorBase.h
DeclarationsVisitor.o: \
	DeclarationsVisitor.cpp \
	DeclarationsVisitor.h \
	XcodeMlRAV.h \
	XcodeMlVisitorBase.h

XcodeMlVisitorBase.pch: XcodeMlVisitorBase.h XcodeMlRAV.h
	clang -x c++-header $(CXXFLAGS) XcodeMlVisitorBase.h -o XcodeMlVisitorBase.pch

CtoXcodeML.ast: CtoXcodeML.cpp
	$(CXX) $(CXXFLAGS) -Xclang -ast-dump CtoXcodeML.cpp -o CtoXcodeML.ast 

test:
	./CtoXcodeML --lineno CtoXcodeML.cpp -- $(CXXFLAGS)

mockup: mockup.cpp

distclean: clean
	rm -f $(RAVOBJS) $(PCHFILES)

clean:
	rm -f CtoXcodeML mockup XcodeMlRAV.i CtoXcodeML.i  $(OBJS) *~

XcodeMlRAV.i: XcodeMlRAV.cpp
	$(CXX) $(CXXFLAGS) -E XcodeMlRAV.cpp -o XcodeMlRAV.i

CtoXcodeML.i: CtoXcodeML.cpp
	$(CXX) $(CXXFLAGS) -E CtoXcodeML.cpp -o CtoXcodeML.i


.PHONY: check-syntax
check-syntax:
	$(CXX) $(CXXFLAGS) -Wall -Wextra -pedantic -fsyntax-only $(CHK_SOURCES)
