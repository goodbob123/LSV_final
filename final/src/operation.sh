command="$1"
read="$2"
write="$3"

if [[ $command == "gen" ]]; then
    ./abc -c "read_truth -xf $read; collapse; sop; fx; strash; write $write;"
elif [[ $command == "opt" ]]; then
    ./abc -c "&read $read; &transduction -T 4 -I 111 -R 117; &dc2; &write $write;"
elif [[ $command == "re" ]]; then
    ./abc -c "read $read; rewrite; refactor; write $write;"
elif [[ $command == "deep" ]]; then
    ./abc -c "&read $read; &deepsyn -I 4 -J 50 -T 10 -S 111 -t; &write $write;"
else
    ./abc -c "read_truth -xf $read; collapse; sop; fx; strash; rewrite; refactor; rewrite; refactor; write $write; &read $write; &transduction -I 111 -R 117; &dc2; &deepsyn -I 4 -J 50 -T 5 -S 111 -t; &transduction -I 111 -R 117; &dc2; &deepsyn -I 4 -J 50 -T 10 -S 111 -t; &write $write;"
fi