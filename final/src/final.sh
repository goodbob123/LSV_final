for ((i = 0 ; i < 100 ; i++)); do
    if (($i < 10)); then
        ./operation.sh all ./benchmarks/ex0${i}.truth ./answer/aig/ex0${i}.aig
    else
        ./operation.sh all ./benchmarks/ex${i}.truth ./answer/aig/ex${i}.aig
    fi
done

rm -rf ./answer/xaig
cp -r ./answer/aig ./answer/xaig