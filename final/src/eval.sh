command="$1"

if [[ $command == "aig" ]]; then
    for ((i = 0 ; i < 100 ; i++)); do
        if (($i < 10)); then
            if [[ ! -e "./answer/aig/ex0${i}.aig" ]]; then
                filename="./answer/aig/ex0${i}.aig"
                n=1
                while read line; do
                # reading each line
                echo "Line No. $n : $line"
                n=$((n+1))
                done < $filename
            fi
        else
            if [[ ! -e "./answer/aig/ex${i}.aig" ]]; then
                filename='company.txt'
                n=1
                while read line; do
                # reading each line
                echo "Line No. $n : $line"
                n=$((n+1))
                done < $filename
            fi
        fi
    done
elif [[ $command == "xaig" ]]; then
    for ((i = 0 ; i < 100 ; i++)); do
        if (($i < 10)); then
            if [[ ! -e "./answer/xaig/ex0${i}.aig" ]]; then
                
            fi
        else
            if [[ ! -e "./answer/xaig/ex${i}.aig" ]]; then
                
            fi
        fi
    done
else
    for ((i = 0 ; i < 100 ; i++)); do
        if (($i < 10)); then
            if [[ ! -e "./answer/aig/ex0${i}.aig" ]]; then
                
            fi
            if [[ ! -e "./answer/xaig/ex0${i}.aig" ]]; then
                
            fi
        else
            if [[ ! -e "./answer/aig/ex${i}.aig" ]]; then
                
            fi
            if [[ ! -e "./answer/xaig/ex${i}.aig" ]]; then
                
            fi
        fi
    done
fi

#rm -rf ./answer/xaig
#cp -r ./answer/aig ./answer/xaig