# $1: input file
# $2: number of nodes
# $3: output file
# $4: threads

ulimit -s unlimited

mkdir -p bicc_output
mkdir -p Results

rm bicc_output/* -f

echo "Invoking bicc_decomposition..."
bicc/bicc_decomposition $1 bicc_output/ 0 $2 0 1 >> /dev/null
statsFile="bicc_output/stats"

while read line;
do
  #next steps
   #echo $firstline ; file which is used to call the maxclique pmc
   filename=$(echo $line|cut -f 1 -d " ")".mtx"
   bicc/Relabeller "bicc_output/"$filename $filename 1
   sh run.sh $filename "Results/"$3 $4
   rm -rf $filename
done < $statsFile
