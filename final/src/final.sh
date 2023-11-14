for ((i = 0 ; i < 100 ; i++)); do
    if (($i < 10)); then
        ./operation.sh all ./benchmarks/ex0${i}.truth ./benchmarks/ex0${i}.truth
    else
        ./operation.sh all ./benchmarks/ex${i}.truth ./benchmarks/ex${i}.truth
    fi
done

rm -rf ./answer/xaig
cp -r ./answer/aig ./answer/xaig