command="$1"
read="$2"
write="$3"

if [[ $command == "gen" ]]; then
    ./abc -c "read_truth -xf $read; collapse; lsv_bdd_reorder 2; sop; fx; strash; write $write;"
elif [[ $command == "opt" ]]; then
    ./abc -c "&read $read; &transduction -T 4 -I 111 -R 117; &dc2; &write $write;"
elif [[ $command == "re" ]]; then
    ./abc -c "read $read; rewrite; refactor; write $write;"
elif [[ $command == "deep" ]]; then
    ./abc -c "&read $read; &deepsyn -I 4 -J 50 -T 10 -S 111 -t; &write $write;"
elif [[ $command == "exist" ]]; then
    if [[ ! -e $write ]]; then
        ./abc -c "read_truth -xf $read; collapse; lsv_bdd_reorder 2; sop; fx; strash; lsv_sa; write $write;"
    fi
elif [[ $command == "cec" ]]; then 
    content="read_truth -xf $read ; cec -n $write ; "
elif [[ $command == "test_hungling" ]]; then
    ./abc -c "lsv_print_message Testingfile:$read; read_truth -xf $read; collapse; sop; fx; strash;lsv_print_message origin: ; print_stats;read_truth -xf $read; collapse;lsv_bdd_reorder 2; sop; fx; strash;lsv_print_message reorder: ; print_stats;"
elif [[ $command == "all" ]]; then
    ./abc -c "read_truth -xf $read; collapse; lsv_bdd_reorder 2; sop; fx; strash; lsv_sa; write $write;"
    #./abc -c "read_truth -xf $read; collapse; sop; fx; strash; rewrite; refactor; rewrite; refactor; write $write; &read $write; &transduction -I 111 -R 117; &dc2; &deepsyn -I 4 -J 50 -T 5 -S 111 -t; &transduction -I 111 -R 117; &dc2; &deepsyn -I 4 -J 50 -T 10 -S 111 -t;print_stats; orchestrate;print_stats; &write $write;"
else
    echo "Please enter these commands: "
    echo "      gen, opt, re, deep, exist, cec, all"
    echo ""
    echo "./operation.sh <cmd> <read> <write>" 
    echo ""
    echo "example: ./operation.sh gen ./benchmarks/ex01.truth ./answer/aig/ex01.aig"
fi