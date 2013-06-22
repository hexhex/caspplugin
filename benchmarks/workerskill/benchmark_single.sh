# $1: instance
# $2: timeout

# default parameters
export PATH=$1
export LD_LIBRARY_PATH=$2
instance=$3
to=$4

confstr="clingcon ;dlvhex2 --cspenable --plugindir=../../../local/lib; dlvhex2 --cspenable --headguess --plugindir=../../../local/lib"

# split configurations
IFS=';' read -ra confs <<< "$confstr"
header="#size"
i=0
for c in "${confs[@]}"
do
	header="$header   \"$c\""
	let i=i+1
done
echo $header

# do benchmark
echo -ne "$instance"

# for all configurations
i=0
for c in "${confs[@]}"
do
	echo -ne -e " "
	$(timeout $to time -o $instance.time.dat -f %e $c 1.hex 2>/dev/null >/dev/null)
	ret=$?
	output=$(cat $instance.time.dat)
	if [[ $ret == 124 ]]; then
		output="---"
	fi
	echo -ne $output
	# rm $instance.time.dat
	let i=i+1
done
echo -e -ne "\n"

rm prog$instance.hex
