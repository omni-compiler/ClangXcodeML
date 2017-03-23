#! /bin/sh

abort() {
  echo "$@"
  exit 1
}

target="$1"

echo -n "${target}.cc,"

make "${target}.xcodeml" >&2 || abort 'CXXtoXcodeML fail'
make "${target}.dst.cpp" >&2 || abort 'XcodeMLtoCXX fail'

echo -n 'OK,' # CXX -> XML conversion was correctly done

clang++ -o "${target}.out" "${target}.dst.cpp" || abort 'Compilation fail'

echo -n 'OK, ' # XML -> C++ conversion was correctly done

echo $(./"${target}.out")
