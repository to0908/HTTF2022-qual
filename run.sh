#!/bin/bash

cd `dirname $0`
g++ -std=gnu++1z -I . -O2 -Wall -Wfatal-errors -Wextra -W main.cpp
rm -rf scores
rm -rf out
mkdir scores
mkdir out

st=0
en=20
procs=0
slep=9

calc() {
  ~/.cargo/bin/cargo run --release --bin tester ./a.out < in/$1.txt > out/$1.txt 2> scores/$1.txt
}

export -f calc

# -J : 150ケース実行
while getopts "j:J" optKey; do
  case "$optKey" in
    J)
      en=149
      slep=60
      ;;
  esac
done

seq -f '%04g' $st $en | xargs -n1 -P$procs -I{} bash -c "calc {}" & sleep $slep

python3 scoreSum.py