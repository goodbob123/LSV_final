command="$1"
read="$2"
write="$3"

rm -rf operation.dofile;
if [[ $command == "gen"]]; then
    read $read; collapse; sop; fx; strash; write_aiger $write;
elif [[ $command == "opt"]]; then
    &read_aiger $read; &transduction; &dc2; &write_aiger $write;
elif [[ $command == "resyn"]]; then
    read_aiger $read; rewrite; refactor; write_aiger $write;
elif [[ $command == "deepsyn"]]; then
    read $read; &deepsyn -I 4 -J 50 -T 5 -S 111 -t; write_aiger $write;
else
    read $read; collapse; sop; fx; strash; rewrite; refactor; rewrite; refactor; &deepsyn -I 4 -J 50 -T 5 -S 111 -t; write_aiger $write;
    &read_aiger $write; &transduction; &dc2; &write_aiger $write;
fi
../abc -dofile operation.dofile;