#! /bin/sh

cxxflags_for_dst=""
for i in reorder \
	deprecated \
	parentheses-equality \
	unused-parameter \
	unused-variable \
	c++11-long-long \
	bitfield-constant-conversion \
	sign-compare \
	unused-private-field
do
    cxxflags_for_dst="${cxxflags_for_dst} -Wno-"$i
done


abort() {
  echo "$@"
  exit 1
}

target="$1"

if grep -sq \^${target}\$ ignore.txt ; then
  exit 0
fi

echo -n "${target}.src.cpp,"

make "${target}.xcodeml" >&2 || abort 'CXXtoXML failed'
make "${target}.dst.cpp" >&2 || abort 'XcodeMLtoCXX failed'

echo -n 'OK,' # CXX -> XML conversion was correctly done

clang++ ${cxxflags_for_dst} -o "${target}.out" "${target}.dst.cpp" || abort 'Compile Error'

echo -n 'OK, ' # XML -> C++ conversion was correctly done

echo $(./"${target}.out")
