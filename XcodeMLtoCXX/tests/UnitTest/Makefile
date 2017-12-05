.SUFFIXES: .cpp
.PHONY: check

XCODEMLTOCXXDIR = ../..
XCODEMLTOCXXSRCDIR = $(XCODEMLTOCXXDIR)/src

LLVM_CONFIG = /usr/local/bin/llvm-config
CXX = /usr/local/bin/clang++
CXXFLAGS = -O2 -std=c++11 \
	$(PKG_CFLAGS) \
	-I$(XCODEMLTOCXXSRCDIR) \
	-D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS $(PCHFLAGS)

USEDLIBS += -lpthread -ldl -ltinfo -lz
USEDLIBS += $(PKG_LIBS)
USEDLIBS += $(OTHERLIBS)

PKG_CFLAGS = $(shell pkg-config --cflags libxml-2.0 2>/dev/null || echo -I/usr/include/libxml2)
PKG_LIBS = $(shell pkg-config --libs libxml-2.0 2>/dev/null || echo -lxml2)

TARGETS = $(basename $(wildcard *.cpp))

all: $(TARGETS)

OBJNAMES = XcodeMlType.o \
	LibXMLUtil.o \
	ClangClassHandler.o \
	CodeBuilder.o \
	Stream.o \
	StringTree.o \
	TypeAnalyzer.o \
	XMLString.o \
	XcodeMlEnvironment.o \
	SymbolAnalyzer.o \
	SymbolBuilder.o

OBJS = $(addprefix $(XCODEMLTOCXXSRCDIR)/,$(OBJNAMES))

$(OBJS):
	$(MAKE) -C $(XCODEMLTOCXXSRCDIR) $(notdir $@)

XcodeMlType: \
	$(XCODEMLTOCXXSRCDIR)/Stream.o \
	$(XCODEMLTOCXXSRCDIR)/StringTree.o \
	$(XCODEMLTOCXXSRCDIR)/XcodeMlEnvironment.o \
	$(XCODEMLTOCXXSRCDIR)/XcodeMlType.o

CXXCodeGenStream: \
	$(XCODEMLTOCXXSRCDIR)/Stream.o

clean:
	rm -f $(TARGETS) $(addsuffix .o, $(TARGETS))

check: $(TARGETS)
	set -e; \
	for testobj in $(TARGETS); do  \
		./$$testobj; \
	done
