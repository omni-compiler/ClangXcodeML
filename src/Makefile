LLVM_CONFIG = /usr/local/bin/llvm-config
CXX = /usr/local/bin/clang++
#LLVM_CONFIG = /usr/bin/llvm-config-3.5
#CXX = /usr/bin/clang++-3.5
CXXFLAGS = -O2 -g `pkg-config --cflags --libs libxml-2.0` -lclang \
	-fno-rtti -std=c++11 -pedantic \
	-D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS

USEDLIBS = -L$(shell $(LLVM_CONFIG) --libdir)
USEDLIBS += -lclangTooling -lclangFrontend -lclangSerialization -lclangDriver \
            -lclangParse -lclangSema -lclangAnalysis \
            -lclangAST -lclangASTMatchers -lclangEdit -lclangLex -lclangBasic
USEDLIBS += $(shell $(LLVM_CONFIG) --libs mcparser bitreader support mc option)
USEDLIBS += -lpthread -ldl -lz -ltinfo
USEDLIBS += $(OTHERLIBS)

CtoXcodeML: CtoXcodeML.cpp
	$(CXX) $(CXXFLAGS) CtoXcodeML.cpp $(USEDLIBS) -o CtoXcodeML

CtoXcodeML.ast: CtoXcodeML.cpp
	$(CXX) $(CXXFLAGS) -Xclang -ast-dump CtoXcodeML.cpp -o CtoXcodeML.ast 

test:
	./CtoXcodeML --lineno CtoXcodeML.cpp -- $(CXXFLAGS)

mockup: mockup.cpp

clean:
	rm -f CtoXcodeML mockup RAV.i

RAV.i: RAV.cpp
	$(CXX) $(CXXFLAGS) -E RAV.cpp -o RAV.i
