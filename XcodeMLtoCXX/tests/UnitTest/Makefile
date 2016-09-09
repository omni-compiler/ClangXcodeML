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

TARGETS = $(patsubst %.cpp,%.out,$(wildcard *.cpp))

OBJNAMES = XcodeMlType.o \
	LibXMLUtil.o \
	CodeBuilder.o \
	TypeAnalyzer.o \
	XMLString.o \
	XcodeMlEnvironment.o \
	SymbolsBuilder.o

OBJS = $(addprefix $(XCODEMLTOCXXSRCDIR)/,$(OBJNAMES))

%.out: %.cpp
	$(CXX) $(CXXFLAGS) $(OBJS) $(USEDLIBS) -o$@ $^

clean:
	rm -f $(TARGETS)

check: $(TARGETS)
	set -e; \
	for testobj in $(TARGETS); do  \
		./$$testobj; \
	done
