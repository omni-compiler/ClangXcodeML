.SUFFIXES: .cpp
.PHONY: check

XCODEMLTOCXXDIR = ../..
XCODEMLTOCXXSRCDIR = $(XCODEMLTOCXXDIR)/src

LLVM_CONFIG = /usr/local/bin/llvm-config
CXX = /usr/local/bin/clang++
CXXFLAGS = -O2 -std=c++11 \
	`pkg-config --cflags libxml-2.0` \
	-I$(XCODEMLTOCXXSRCDIR) \
	-D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS $(PCHFLAGS)

USEDLIBS += -lpthread -ldl -ltinfo -lz
USEDLIBS += `pkg-config --libs libxml-2.0`
USEDLIBS += $(OTHERLIBS)

TARGETS = $(basename $(wildcard *.cpp))

all: $(TARGETS)

OBJNAMES = XcodeMlType.o \
	LibXMLUtil.o \
	ClangClassHandler.o \
	CodeBuilder.o \
	CXXCodeGen.o \
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
	$(XCODEMLTOCXXSRCDIR)/StringTree.o \
	$(XCODEMLTOCXXSRCDIR)/XcodeMlEnvironment.o \
	$(XCODEMLTOCXXSRCDIR)/XcodeMlType.o

CXXCodeGenStream: \
	$(XCODEMLTOCXXSRCDIR)/CXXCodeGen.o

clean:
	rm -f $(TARGETS) $(addsuffix .o, $(TARGETS))

check: $(TARGETS)
	set -e; \
	for testobj in $(TARGETS); do  \
		./$$testobj; \
	done
