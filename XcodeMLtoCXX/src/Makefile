LLVM_CONFIG = /usr/local/bin/llvm-config
CXX = /usr/local/bin/clang++
CXXFLAGS = -O2 -g -Wall -Wextra -fno-rtti -std=c++11 -pedantic \
	`pkg-config --cflags libxml-2.0` \
	-D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS $(PCHFLAGS)

USEDLIBS += -lpthread -ldl -ltinfo -lz
USEDLIBS += `pkg-config --libs libxml-2.0`
USEDLIBS += $(OTHERLIBS)

OBJS = XcodeMlType.o \
	XcodeMLtoCXX.o \
	LibXMLUtil.o \
	CodeBuilder.o \
	TypeAnalyzer.o \
	XMLString.o \
	XcodeMlEnvironment.o \
	SymbolBuilder.o

XcodeMLtoCXX: $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) $(USEDLIBS) -o XcodeMLtoCXX

XcodeMLtoCXX.o: \
	CodeBuilder.h \
	TypeAnalyzer.h
CodeBuilder.o: \
	XMLString.h \
	LibXMLUtil.h \
	XcodeMlType.h \
	XcodeMlEnvironment.h \
	TypeAnalyzer.h \
	SymbolAnalyzer.h \
	CodeBuilder.h \
	XMLWalker.h \
	AttrProc.h \
	SourceInfo.h
TypeAnalyzer.o: \
	XMLString.h \
	LibXMLUtil.h \
	SymbolAnalyzer.h \
	XcodeMlType.h \
	XcodeMlEnvironment.h \
	TypeAnalyzer.h \
	XMLWalker.h
XcodeMlType.o: \
  SymbolAnalyzer.h \
	TypeAnalyzer.h \
	XcodeMlEnvironment.h

clean:
	rm -f $(OBJS) *~

all: XcodeMLtoCXX
