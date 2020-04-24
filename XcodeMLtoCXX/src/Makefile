LLVM_CONFIG = /usr/local/bin/llvm-config90
CXX = /usr/local/bin/clang++90
CXXFLAGS = $(OPTIMIZERFLAGS) -g -Wall -Wextra -fno-rtti -std=c++11 -pedantic \
	-I $(shell $(LLVM_CONFIG) --includedir) \
	$(PKG_CFLAGS) \
	-D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS $(PCHFLAGS)

USEDLIBS += -lpthread -ldl -ltinfo -lz
USEDLIBS += $(PKG_LIBS)
USEDLIBS += $(OTHERLIBS)

OPTIMIZERFLAGS = -O2
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
	XcodeMlTypeTable.o \
	SourceInfo.o \
	StringTree.o \
	ClangDeclHandler.o \
	ClangNestedNameSpecHandler.o \
	ClangStmtHandler.o \
	ClangTypeLocHandler.o \
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
	XcodeMlTypeTable.h \
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
	XcodeMlTypeTable.h \
	TypeAnalyzer.h \
	XMLWalker.h
XcodeMlNns.o: \
	StringTree.h \
	XcodeMlTypeTable.h \
	XcodeMlNns.h \
	XcodeMlType.h
XcodeMlType.o: \
	TypeAnalyzer.h \
	XcodeMlTypeTable.h

XcodeMlUtil.o: \
	XcodeMlUtil.h

clean:
	rm -f $(XCODEMLTOCXX)
	rm -f $(OBJS) *~

all: $(XCODEMLTOCXX)
