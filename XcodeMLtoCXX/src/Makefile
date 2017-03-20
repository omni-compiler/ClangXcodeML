LLVM_CONFIG = /usr/local/bin/llvm-config
CXX = /usr/local/bin/clang++
CXXFLAGS = -O2 -g -Wall -Wextra -fno-rtti -std=c++11 -pedantic \
	`pkg-config --cflags libxml-2.0` \
	-D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS $(PCHFLAGS)

USEDLIBS += -lpthread -ldl -ltinfo -lz
USEDLIBS += `pkg-config --libs libxml-2.0`
USEDLIBS += $(OTHERLIBS)
XCODEMLTOCXX = ../XcodeMLtoCXX

OBJS = XcodeMlType.o \
	Stream.o \
	XcodeMLtoCXX.o \
	LibXMLUtil.o \
	CodeBuilder.o \
	TypeAnalyzer.o \
	XMLString.o \
	XcodeMlEnvironment.o \
	StringTree.o \
	SymbolBuilder.o \
	SymbolAnalyzer.o \
	ClangClassHandler.o \
	XcodeMlNns.o

$(XCODEMLTOCXX): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) $(USEDLIBS) -o $(XCODEMLTOCXX)

XcodeMLtoCXX.o: \
	CodeBuilder.h \
	TypeAnalyzer.h
CodeBuilder.o: \
	XMLString.h \
	LibXMLUtil.h \
	XcodeMlType.h \
	XcodeMlEnvironment.h \
	TypeAnalyzer.h \
	Symbol.h \
	CodeBuilder.h \
	XMLWalker.h \
	AttrProc.h \
	SourceInfo.h
TypeAnalyzer.o: \
	XMLString.h \
	LibXMLUtil.h \
	Symbol.h \
	SymbolAnalyzer.h \
	XcodeMlType.h \
	XcodeMlEnvironment.h \
	TypeAnalyzer.h \
	XMLWalker.h
XcodeMlNns.o: \
	StringTree.h \
	XcodeMlEnvironment.h \
	XcodeMlNns.h \
	XcodeMlType.h
XcodeMlType.o: \
  Symbol.h \
	TypeAnalyzer.h \
	XcodeMlEnvironment.h

clean:
	rm -f $(XCODEMLTOCXX)
	rm -f $(OBJS) *~

all: $(XCODEMLTOCXX)
