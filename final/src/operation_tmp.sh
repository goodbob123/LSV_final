command="$1"
read_case="$2"
benchmarks_path="./benchmarks/ex"
answer_path="./answer/ex"
truth_file="$benchmarks_path$read_case.truth"
aig_file="$answer_path$read_case.aig"

rm -rf operation.dofile;


if [[ $command == "gen" ]]; then 
    content="read_truth -xf  $truth_file; collapse; sop; fx; strash; write $aig_file; print_stats"
elif [[ $command == "opt" ]]; then 
    content="read_aiger $aig_file;print_stats; strash;&get; &transduction; &dc2 -v; write_aiger $aig_file;print_stats"
elif [[ $command == "resyn" ]]; then 
    content="read_aiger $aig_file; print_stats ;rewrite; refactor; write_aiger $aig_file;print_stats"
elif [[ $command == "deepsyn" ]]; then 
    content="read_aiger $aig_file;print_stats; strash; &get; &deepsyn -I 4 -J 50 -T 5 -S 111 -t; write_aiger $aig_file;print_stats"
elif [[ $command == "cec" ]]; then 
    content="read_truth -xf $truth_file ; cec -n $aig_file ; "
elif [[ $command == "all" ]]; then 
    content="read_truth -xf $truth_file; collapse; sop; fx; strash;print_stats;&get; rewrite; refactor; rewrite; refactor; &deepsyn -I 4 -J 50 -T 5 -S 111 -t -v; write_aiger $aig_file;
    read_aiger $aig_file;print_stats; &transduction;print_stats; &dc2 -v;print_stats; write_aiger $aig_file;"
elif [[ $command == "--help" ]]; then
    echo "Please enter these commands: "
    echo "      gen, opt, resyn, deepsyn, cec, all"
    echo ""
    echo "./operation_temp.sh <cmd> <case number>" 
    echo ""
    echo "example: ./operation_temp.sh gen 14"
else
    echo "Error: command not found"
    exit 1
fi


if [[ $command != "--help" ]]; then
    echo $content >> operation.dofile;
    ./abc -f operation.dofile;
    rm -rf operation.dofile;
fi

