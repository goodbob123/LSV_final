for i in {0..99}; do 

case=$i
if [[ $i -lt 10 ]]; then
    case="0$i"
fi

read=ref_materials/benchmarks/ex$case.truth
write=LSV_data/aig/ex$case.aig

if [[ !  $case ]]; then
    echo "Please enter a case number"
    exit 1
fi

echo "Input: $read"
echo "Output: $write"

## ABC truth table to aig
./abc -c "read_truth -xf $read; strash; write $write;"

## do transformation and optimization
dofile="do.all"
rm -f $dofile
echo "AIG2aag ./LSV_data/aig/ex$case.aig ./LSV_data/aag/ex$case.aag" >> $dofile
echo "CIRRead ./LSV_data/aag/ex$case.aag" >> $dofile
echo "CIRwindow" >> $dofile
echo "CIRWrite -Output ./LSV_data/window_aag/ex$case.aag" >> $dofile
echo "quit -f" >> $dofile
./window -f $dofile

done

echo "----[ runAll END ]----" 

