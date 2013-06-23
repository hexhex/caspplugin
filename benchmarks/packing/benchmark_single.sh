# $1: instance
# $2: timeout

# default parameters
export PATH=$1
export LD_LIBRARY_PATH=$2
instance=$3
to=$4

# get all possible answers
confstr="--quiet clingcon --csp-opt-all --opt-all --csp-num-as=1 --number=1; \
--quiet clingcon --csp-opt-all --opt-all --csp-num-as=1 --number=1 --csp-reduce-conflict=forward; \
--quiet clingcon --csp-opt-all --opt-all --csp-num-as=1 --number=1 --csp-reduce-conflict=backward; \
--quiet clingcon --csp-opt-all --opt-all --csp-num-as=1 --number=1 --csp-reduce-conflict=cc; \
--quiet dlvhex2 --cspenable --plugindir=../../../local/lib --silent --number=1; \
--quiet dlvhex2 --cspenable --plugindir=../../../local/lib --silent --number=1 --heuristics=monolithic --extlearn --csplearning=deletion; \
--quiet dlvhex2 --cspenable --plugindir=../../../local/lib --silent --number=1 --heuristics=monolithic --extlearn --csplearning=forward; \
--quiet dlvhex2 --cspenable --plugindir=../../../local/lib --silent --number=1 --heuristics=monolithic --extlearn --csplearning=backward; \
--quiet dlvhex2 --cspenable --plugindir=../../../local/lib --silent --number=1 --heuristics=monolithic --extlearn --csplearning=cc; \
--quiet dlvhex2 --cspenable --plugindir=../../../local/lib --silent --number=1 --heuristics=monolithic --extlearn --csplearning=wcc;"

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
	$(timeout $to time -o $instance.time.dat -f %e $c $instance-packing-0-0.hex 2>/dev/null >/dev/null)
	ret=$?
	output=$(cat $instance.time.dat)
	if [[ $ret == 124 ]]; then
		output="---"
	fi
	echo -ne $output
	rm $instance.time.dat
	let i=i+1
done
echo -e -ne "\n"

rm prog$instance.hex
