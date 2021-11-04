#!/bin/bash

cd `dirname $0`
g++ -std=gnu++1z -I . -O2 -Wall -Wfatal-errors -Wextra -W main.cpp
rm -rf scores
rm -rf out
mkdir scores
mkdir out

st=0
en=20
procs=3
small=0

calc() {
  ~/.cargo/bin/cargo run --release --bin tester ./a.out < in/$1.txt > out/$1.txt 2> scores/$1.txt
}

export -f calc

# ~/.cargo/bin/cargo run --release --bin tester ./a.out < in/0000.txt > out/0000.txt
    
# for i in `seq 0 9`
# do
# ~/.cargo/bin/cargo run --release --bin tester ./a.out < in/000$i.txt > out/000$i.txt 2> scores/000$i.txt
# done

seq -f '%04g' $st $en | xargs -n1 -P$procs -I{} bash -c "calc {}"


python3 scoreSum.py