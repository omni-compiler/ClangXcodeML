LLVM_CONFIG = /usr/local/bin/llvm-config
CXX = /usr/local/bin/clang++
#LLVM_CONFIG = llvm-config-3.5
#CXX = clang++-3.5

USEDLIBS = -L$(shell $(LLVM_CONFIG) --libdir)
USEDLIBS += -lclangTooling -lclangFrontend -lclangSerialization -lclangDriver \
            -lclangParse -lclangSema -lclangAnalysis \
            -lclangAST -lclangASTMatchers -lclangEdit -lclangLex -lclangBasic
USEDLIBS += $(shell $(LLVM_CONFIG) --libs mcparser bitreader support mc option)
USEDLIBS += -lpthread -ldl
USEDLIBS += $(OTHERLIBS)

CtoXcodeML: CtoXcodeML.cpp
	$(CXX) -O2 -fno-rtti -std=c++11 -D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS \
		CtoXcodeML.cpp $(USEDLIBS) -o CtoXcodeML

CXXFLAGS = -O2 -g `pkg-config --cflags --libs libxml-2.0` -lclang

mockup: mockup.cpp

clean:
	rm -f CtoXcodeML mockup
