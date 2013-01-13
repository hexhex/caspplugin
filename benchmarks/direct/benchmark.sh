for instance in *.hex
do
	echo "Running instance" $instance
	echo $(timeout 3m /usr/bin/time --format "%e" dlvhex2 --silent $instance >>/dev/null) 
done
