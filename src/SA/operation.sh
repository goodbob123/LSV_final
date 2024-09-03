command="$1"
read="$2"
write="$3"

if [[ $command == "exist" ]]; then
    if [[ ! -e $write ]]; then
        ./abc -c "read_truth -xf $read; collapse; lsv_bdd_reorder 2; sop; fx; strash; lsv_sa; write $write;"
    fi
elif [[ $command == "all" ]]; then
    ./abc -c "read_truth -xf $read; collapse; lsv_bdd_reorder 2; sop; fx; strash; lsv_sa; write $write;"
    #./abc -c "read_truth -xf $read; collapse; sop; fx; strash; rewrite; refactor; rewrite; refactor; write $write; &read $write; &transduction -I 111 -R 117; &dc2; &deepsyn -I 4 -J 50 -T 5 -S 111 -t; &transduction -I 111 -R 117; &dc2; &deepsyn -I 4 -J 50 -T 10 -S 111 -t;print_stats; orchestrate;print_stats; &write $write;"
else
    echo "Please enter these commands: "
    echo "      exist, all"
    echo ""
    echo "./operation.sh <cmd> <read> <write>" 
    echo ""
    echo "example: ./operation.sh all ./benchmarks/ex01.truth ./answer/aig/ex01.aig"
fi