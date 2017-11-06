LLVM_CONFIG = /usr/local/bin/llvm-config
CXX = /usr/local/bin/clang++
CXXFLAGS = -O2 -g -Wall -Wextra -fno-rtti -std=c++11 -pedantic \
	$(PKG_CFLAGS) \
	-D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS $(PCHFLAGS)

USEDLIBS += -lpthread -ldl -ltinfo -lz
USEDLIBS += $(PKG_LIBS)
USEDLIBS += $(OTHERLIBS)

PKG_CFLAGS = $(shell pkg-config --cflags libxml-2.0 2>/dev/null || echo -I/usr/include/libxml2)
PKG_LIBS = $(shell pkg-config --libs libxml-2.0 2>/dev/null || echo -lxml2)

XCODEMLTOCXX = ../XcodeMLtoCXX

OBJS = XcodeMlType.o \
	Stream.o \
	XcodeMLtoCXX.o \
	LibXMLUtil.o \
	CodeBuilder.o \
	NnsAnalyzer.o \
	TypeAnalyzer.o \
	XMLString.o \
	XcodeMlEnvironment.o \
	StringTree.o \
	ClangClassHandler.o \
	XcodeMlName.o \
	XcodeMlNns.o \
	XcodeMlOperator.o \
	XcodeMlUtil.o

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
	NnsAnalyzer.o \
	TypeAnalyzer.h \
	CodeBuilder.h \
	XMLWalker.h \
	AttrProc.h \
	SourceInfo.h
TypeAnalyzer.o: \
	XMLString.h \
	LibXMLUtil.h \
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
	TypeAnalyzer.h \
	XcodeMlEnvironment.h

XcodeMlUtil.o: \
	XcodeMlUtil.h

clean:
	rm -f $(XCODEMLTOCXX)
	rm -f $(OBJS) *~

all: $(XCODEMLTOCXX)
