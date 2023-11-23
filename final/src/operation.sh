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
elif [[ $command == "exist" ]]; then
    if [[ ! -e $write ]]; then
        ./abc -c "read_truth -xf $read; collapse; sop; fx; strash; rewrite; refactor; rewrite; refactor; write $write; &read $write; &transduction -I 111 -R 117; &dc2; &deepsyn -I 4 -J 50 -T 5 -S 111 -t; &transduction -I 111 -R 117; &dc2; &deepsyn -I 4 -J 50 -T 10 -S 111 -t; &write $write;"
    fi
elif [[ $command == "cec" ]]; then 
    content="read_truth -xf $read ; cec -n $write ; "
elif [[ $command == "all" ]]; then
    ./abc -c "read_truth -xf $read; collapse; sop; fx; strash; rewrite; refactor; rewrite; refactor; write $write; &read $write; &transduction -I 111 -R 117; &dc2; &deepsyn -I 4 -J 50 -T 5 -S 111 -t; &transduction -I 111 -R 117; &dc2; &deepsyn -I 4 -J 50 -T 10 -S 111 -t; &write $write;"
else
    echo "Please enter these commands: "
    echo "      gen, opt, re, deep, exist, cec, all"
    echo ""
    echo "./operation_temp.sh <cmd> <read> <write>" 
    echo ""
    echo "example: ./operation_temp.sh gen ./benchmarks/ex01.truth ./answer/aig/ex01.aig"
fi