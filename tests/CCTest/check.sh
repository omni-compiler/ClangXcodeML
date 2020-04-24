#!/bin/sh


make_with_preferences() {
  target=$1
  gmake \
    CXXTOXMLFLAGS=' -- -I. -I/usr/local/lib/clang/3.6.2/include -w' \
    $target >&2
}

examine() {
  target=$1
  if $(make_with_preferences "${target}.xcodeml") ; then
    echo  # success
  else
    echo 'CXXtoXcodeML fail'
    exit 1
  fi

  if $(make_with_preferences "${target}.dst.cpp") ; then
    echo  # success
  else
    echo 'XcodeMLtoCXX fail'
    exit 1
  fi

  clang-format -style=LLVM -i ${target}.dst.cpp

  if $(clang++ -o "${target}.out" "${target}.dst.cpp") ; then
    echo  # success
  else
    echo 'Compilation fail'
    exit 1
  fi

  echo $(./"${target}.out")

}

print_line() {
  read cxx_to_xcodeml_result
  if [ -n "${cxx_to_xcodeml_result}" ] ; then
    echo "Error: ${cxx_to_xcodeml_result}"
      # C++ -> XML conversion failed
    exit 1
  fi

  read xcodeml_to_cxx_result
  if [ -n "${xcodeml_to_cxx_result}" ] ; then
    echo "Error: ${xcodeml_to_cxx_result}"
      # C++ -> XML conversion failed
    exit 1
  fi

  echo -n 'OK,' # CXX -> XML conversion was correctly done

  read compiler_result
  if [ -n "${compiler_result}" ] ; then
    echo "Error: ${compiler_result}"
      # XML -> C++ conversion failed
    exit 1
  fi

  echo -n 'OK, ' # XML -> C++ conversion was correctly done

  read execution_result
  echo ${execution_result}
}

print_csv() {
  while read -r line
  do
    echo -n "${line}.cc,"
    examine ${line} | print_line
  done
}

gmake -s clean
rm -f *.out err.log result.csv
print_csv < testobjects.txt > result.csv 2> err.log
