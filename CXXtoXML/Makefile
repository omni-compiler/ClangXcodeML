LLVM_CONFIG = /usr/local/bin/llvm-config
CXX = /usr/local/bin/clang++
#LLVM_CONFIG = /usr/bin/llvm-config-3.6
#CXX = /usr/bin/clang++-3.6
CXXFLAGS = -O2 -g -Wall -Wextra -fno-rtti -std=c++1y -pedantic \
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
	XMLVisitorBase.o \
	TypeTableVisitor.o \
	DeclarationsVisitor.o \
	InheritanceInfo.o \
	OperationKinds.o

CXXtoXML: $(RAVOBJS) $(OBJS)
	$(CXX) $(CXXFLAGS) $(RAVOBJS) $(OBJS) $(USEDLIBS) -o CXXtoXML

XMLRAV.o: \
	XMLRAV.cpp \
	XMLRAV.h
CXXtoXML.o: \
	CXXtoXML.cpp \
	XMLRAV.h \
	XMLVisitorBase.h \
	TypeTableVisitor.h \
	DeclarationsVisitor.h
XMLVisitorBase.o: \
	XMLVisitorBase.cpp \
	XMLRAV.h \
	XMLVisitorBase.h
TypeTableVisitor.o: \
	TypeTableVisitor.cpp \
	TypeTableVisitor.h \
	DeclarationsVisitor.h \
	XMLRAV.h \
	XMLVisitorBase.h
DeclarationsVisitor.o: \
	DeclarationsVisitor.cpp \
	DeclarationsVisitor.h \
	TypeTableVisitor.h \
	XMLRAV.h \
	XMLVisitorBase.h \
	InheritanceInfo.cpp \
	InheritanceInfo.h \
	OperationKinds.cpp \
	OperationKinds.h
InheritanceInfo.o: \
	InheritanceInfo.cpp \
	InheritanceInfo.h \
	Hash.h
OperationKinds.o: \
	OperationKinds.cpp \
	OperationKinds.h

distclean: clean
	rm -f $(RAVOBJS)

clean:
	rm -f CXXtoXML XMLRAV.i $(OBJS) *~

XMLRAV.i: XMLRAV.cpp
	$(CXX) $(CXXFLAGS) -E XMLRAV.cpp -o XMLRAV.i


.PHONY: check-syntax
check-syntax:
	$(CXX) $(CXXFLAGS) -Wall -Wextra -pedantic -fsyntax-only $(CHK_SOURCES)
