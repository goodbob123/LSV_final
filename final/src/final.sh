command="$1"
start="$2"
end="$3"

if [[ $command == "exist" ]]; then
    for ((i = 0 ; i < 100 ; i++)); do
        if (($i < 10)); then
            ./operation.sh exist ./benchmarks/ex0${i}.truth ./answer/aig/ex0${i}.aig
        else
            ./operation.sh exist ./benchmarks/ex${i}.truth ./answer/aig/ex${i}.aig
        fi
    done
elif [[ $command == "assign" ]]; then
    for ((i = $start ; i <= $end ; i++)); do
        if (($i < 10)); then
            ./operation.sh all ./benchmarks/ex0${i}.truth ./answer/aig/ex0${i}.aig
        else
            ./operation.sh all ./benchmarks/ex${i}.truth ./answer/aig/ex${i}.aig
        fi
    done
else
    for ((i = 0 ; i < 100 ; i++)); do
        if (($i < 10)); then
            ./operation.sh all ./benchmarks/ex0${i}.truth ./answer/aig/ex0${i}.aig
        else
            ./operation.sh all ./benchmarks/ex${i}.truth ./answer/aig/ex${i}.aig
        fi
    done
fi

#rm -rf ./answer/xaig
#cp -r ./answer/aig ./answer/xaig