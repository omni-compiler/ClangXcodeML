#!/bin/bash


make_here() {
  target=$1
  make \
    CXXTOXMLFLAGS=' -- -I. -I/usr/local/lib/clang/3.6.2/include -w' \
    $target
}

print_csv() {
  while read -r line
  do
    echo -n ${line}.cc,
    make_here ${line}.xcodeml
    if [ $? -gt 0 ]; then
      echo "Error"
    else
      echo -n "OK,"
      make_here ${line}.dst.cpp
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
