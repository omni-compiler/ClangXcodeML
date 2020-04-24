LLVM_CONFIG = /usr/local/bin/llvm-config90
#CXX = /usr/local/bin/clang++
CXX= /usr/local/bin/clang++90
#LLVM_CONFIG = /usr/bin/llvm-config-3.6
#CXX = /usr/bin/clang++-3.6
#nCXXFLAGS = -O2 -g -Wall -Wextra -Wno-unused-parameter -fno-rtti -std=c++11 -pedantic \
#	$(INCLUDE) \
#	-D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS

CXXFLAGS = -g -Wall -Wextra -Wno-unused-parameter -fno-rtti -std=c++11 -pedantic \
	$(INCLUDE) \
	-D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS

USEDLIBS = -L$(shell $(LLVM_CONFIG) --libdir)
USEDLIBS += -lclangTooling -lclangFrontend -lclangSerialization -lclangDriver \
            -lclangParse -lclangSema -lclangAnalysis \
            -lclangAST -lclangASTMatchers -lclangEdit -lclangLex -lclangBasic \
            -lclangFormat -lclangARCMigrate -lclangDynamicASTMatchers \
            -lclangIndex -lclangCodeGen -lclangFrontendTool \
            -lclangStaticAnalyzerCheckers -lclangRewrite -lclangRewriteFrontend
INCLUDE = -I. \
	  -I $(shell $(LLVM_CONFIG) --includedir) \
	  $(PKG_CFLAGS)
USEDLIBS += $(shell $(LLVM_CONFIG) --libs mcparser bitreader support mc option)
USEDLIBS += -lpthread -ldl -ltinfo -lz
USEDLIBS += $(PKG_LIBS) \
	    -lclang
USEDLIBS += $(OTHERLIBS)

PKG_CFLAGS = $(shell pkg-config --cflags libxml-2.0 2>/dev/null || echo -I/usr/include/libxml2)
PKG_LIBS = $(shell pkg-config --libs libxml-2.0 2>/dev/null || echo -lxml2)

OBJS =  CXXtoXcodeML.o \
	ClangUtil.o \
	TypeTableInfo.o \
	InheritanceInfo.o \
	NnsTableInfo.o \
	XcodeMlNameElem.o \
	XMLRecursiveASTVisitor.o \
	ClangOperator.o

CXXtoXcodeML: $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) $(USEDLIBS) -o CXXtoXcodeML

ClangUtil.o: \
	ClangUtil.h

CXXtoXcodeML.o: \
	CXXtoXcodeML.cpp \
	TypeTableInfo.h \
	NnsTableInfo.h \
	XMLRecursiveASTVisitor.o 

XMLRecursiveASTVisitor.o: \
	XMLRecursiveASTVisitor.cpp \
	XMLRecursiveASTVisitor.h \
 	TypeTableInfo.h \
 	XMLRecursiveASTVisitor.cpp \
 	XMLRecursiveASTVisitor.h \
 	InheritanceInfo.cpp \
 	InheritanceInfo.h \
 	NnsTableInfo.h \
 	ClangOperator.cpp \
 	ClangOperator.h

TypeTableInfo.o: \
	TypeTableInfo.cpp \
	TypeTableInfo.h

InheritanceInfo.o: \
	InheritanceInfo.cpp \
	InheritanceInfo.h \
	Hash.h

ClangOperator.o: \
	ClangOperator.cpp \
	ClangOperator.h

clean:
	rm -f CXXtoXcodeML $(OBJS) *~


.PHONY: check-syntax
check-syntax:
	$(CXX) $(CXXFLAGS) -Wall -Wextra -pedantic -fsyntax-only $(CHK_SOURCES)
