# $1: instance
# $2: timeout

# default parameters
export PATH=$1
export LD_LIBRARY_PATH=$2
instance=$3
to=$4

confstr="dlvhex2 --cspenable; dlvhex2 --cspenable --headguess"

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

# write HEX program
prog="
$dom(5..10)
"
for (( j = 1; j <= instance; j++ ))
do
	prog="domain($j). $prog"
done
echo $prog > prog$instance.hex

# for all configurations
i=0
for c in "${confs[@]}"
do
	echo -ne -e " "
	$(timeout $to time -o $instance.time.dat -f %e $c --plugindir=../../../local/lib simple.hex 2>/dev/null >debug.log)
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
