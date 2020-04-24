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

echo -n "${target}.cc,"


if grep -sq \^${target}\$ ignore.txt ; then
  echo
  exit 0
fi

gmake "${target}.xcodeml" >&2 || abort 'CXXtoXcodeML failed'
gmake "${target}.dst.cpp" >&2 || abort 'XcodeMLtoCXX failed'

echo -n 'OK,' # CXX -> XML conversion was correctly done

clang++ ${cxxflags_for_dst} -o "${target}.out" "${target}.dst.cpp" || abort 'Compilation failed'

echo -n 'OK, ' # XML -> C++ conversion was correctly done

echo $(./"${target}.out")
