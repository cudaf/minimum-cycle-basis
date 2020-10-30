# $1: input file
# $2: output file
# $3: threads
mcb/mcb_cuda $1 $2"_R.txt" $3
mcb/mcb_cpu $1 $2"_C.txt" $3
mcb/mcb_cpu_baseline $1 $2"_W.txt" $3
