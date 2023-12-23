command="$1"
rm -rf message
if [[ $command == "aig" ]]; then
    for ((i = 0 ; i < 100 ; i++)); do
        if (($i < 10)); then
            if [[  -e "./answer/aig/ex0${i}.aig" ]]; then
                filename="./answer/aig/ex0${i}.aig"
                ./abc -c "read $filename; print_stats;" > message
                ./eval message
            fi
        else
            if [[  -e "./answer/aig/ex${i}.aig" ]]; then
                filename="./answer/aig/ex${i}.aig"
                ./abc -c "read $filename; print_stats;" > message
                ./eval message
            fi
        fi
    done
elif [[ $command == "xaig" ]]; then
    for ((i = 0 ; i < 100 ; i++)); do
        if (($i < 10)); then
            if [[  -e "./answer/xaig/ex0${i}.aig" ]]; then
                filename="./answer/aig/ex0${i}.aig"
                ./abc -c "read $filename; print_stats;" > message
                ./eval message
            fi
        else
            if [[  -e "./answer/xaig/ex${i}.aig" ]]; then
                filename="./answer/aig/ex${i}.aig"
                ./abc -c "read $filename; print_stats;" > message
                ./eval message
            fi
        fi
    done
else
    for ((i = 0 ; i < 100 ; i++)); do
        if (($i < 10)); then
            if [[  -e "./answer/aig/ex0${i}.aig" ]]; then
                ffilename="./answer/aig/ex0${i}.aig"
                ./abc -c "read $filename; print_stats;" > message
                ./eval message
            fi
            if [[  -e "./answer/xaig/ex0${i}.aig" ]]; then
                filename="./answer/aig/ex0${i}.aig"
                ./abc -c "read $filename; print_stats;" > message
                ./eval message
            fi
        else
            if [[  -e "./answer/aig/ex${i}.aig" ]]; then
                filename="./answer/aig/ex${i}.aig"
                ./abc -c "read $filename; print_stats;" > message
                ./eval message
            fi
            if [[  -e "./answer/xaig/ex${i}.aig" ]]; then
                filename="./answer/aig/ex${i}.aig"
                ./abc -c "read $filename; print_stats;" > message
                ./eval message
            fi
        fi
    done
fi
rm -rf message
#rm -rf ./answer/xaig
#cp -r ./answer/aig ./answer/xaig