function make_clean()
{
	echo "cleaning bcm sdk for x86 arch..."
	source env_x86_4_mrv
	#ib_console 
	make clean
	cd ../../../../scripts/
	echo -e "cleaning bcm sdk for x86 arch done"
}

time make_clean "${@}"
date 1>&2
