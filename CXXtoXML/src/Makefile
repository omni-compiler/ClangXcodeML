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
            -lclangAST -lclangASTMatchers -lclangEdit -lclangLex -lclangBasic \
            -lclangFormat -lclangARCMigrate -lclangDynamicASTMatchers \
            -lclangIndex -lclangCodeGen -lclangFrontendTool \
            -lclangStaticAnalyzerCheckers -lclangRewrite -lclangRewriteFrontend
USEDLIBS += $(shell $(LLVM_CONFIG) --libs mcparser bitreader support mc option)
USEDLIBS += -lpthread -ldl -ltinfo -lz
USEDLIBS += `pkg-config --libs libxml-2.0` -lclang
USEDLIBS += $(OTHERLIBS)

RAVOBJS = XMLRAV.o
OBJS =  CXXtoXML.o \
	ClangUtil.o \
	XMLVisitorBase.o \
	TypeTableInfo.o \
	DeclarationsVisitor.o \
	InheritanceInfo.o \
	NnsTableInfo.o \
	ClangOperator.o

CXXtoXML: $(RAVOBJS) $(OBJS)
	$(CXX) $(CXXFLAGS) $(RAVOBJS) $(OBJS) $(USEDLIBS) -o CXXtoXML

ClangUtil.o: \
	ClangUtil.h
XMLRAV.o: \
	XMLRAV.cpp \
	XMLRAV.h
CXXtoXML.o: \
	CXXtoXML.cpp \
	XMLRAV.h \
	XMLVisitorBase.h \
	TypeTableInfo.h \
	NnsTableInfo.h \
	DeclarationsVisitor.h
XMLVisitorBase.o: \
	XMLVisitorBase.cpp \
	XMLRAV.h \
	XMLVisitorBase.h
TypeTableInfo.o: \
	TypeTableInfo.cpp \
	TypeTableInfo.h \
	DeclarationsVisitor.h \
	XMLRAV.h \
	XMLVisitorBase.h
DeclarationsVisitor.o: \
	DeclarationsVisitor.cpp \
	DeclarationsVisitor.h \
	TypeTableInfo.h \
	XMLRAV.h \
	XMLVisitorBase.h \
	InheritanceInfo.cpp \
	InheritanceInfo.h \
	NnsTableInfo.h \
	ClangOperator.cpp \
	ClangOperator.h
InheritanceInfo.o: \
	InheritanceInfo.cpp \
	InheritanceInfo.h \
	Hash.h
ClangOperator.o: \
	ClangOperator.cpp \
	ClangOperator.h

distclean: clean
	rm -f $(RAVOBJS)

clean:
	rm -f CXXtoXML XMLRAV.i $(OBJS) *~

XMLRAV.i: XMLRAV.cpp
	$(CXX) $(CXXFLAGS) -E XMLRAV.cpp -o XMLRAV.i


.PHONY: check-syntax
check-syntax:
	$(CXX) $(CXXFLAGS) -Wall -Wextra -pedantic -fsyntax-only $(CHK_SOURCES)
