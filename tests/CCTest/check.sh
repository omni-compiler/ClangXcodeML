#!/bin/bash -xv


make_with_preferences() {
  target=$1
  make \
    CXXTOXMLFLAGS=' -- -I. -I/usr/local/lib/clang/3.6.2/include -w' \
    -s \
    $target
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

  if $(clang++ -o "${target}.out" "${target}.dst.cpp") ; then
    echo  # success
  else
    echo 'Compilation fail'
    exit 1
  fi

  echo $(./"${target}.out")

}

print_csv() {
  while read -r line
  do
    echo -n ${line}.cc,
    make_with_preferences ${line}.xcodeml
    if [ $? -gt 0 ]; then
      echo "Error"
    else
      echo -n "OK,"
      make_with_preferences ${line}.dst.cpp
      if [ $? -gt 0 ]; then
        echo "Error"
      else
        echo "OK"
      fi
    fi
  done
}

make -s clean
print_csv < testobjects.txt > result.csv 2> err.log
