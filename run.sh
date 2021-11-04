#!/bin/sh

cd `dirname $0`
g++ -std=gnu++1z -I . -O2 -Wall -Wfatal-errors -Wextra -W main.cpp
rm -rf scores
mkdir scores

# ~/.cargo/bin/cargo run --release --bin tester ./a.out < in/0000.txt > out/0000.txt
    
for i in `seq 0 9`
do
~/.cargo/bin/cargo run --release --bin tester ./a.out < in/000$i.txt > out/000$i.txt 2> scores/000$i.txt
done


python3 scoreSum.py