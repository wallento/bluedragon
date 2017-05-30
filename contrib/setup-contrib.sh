# Number of available cores.
ncores=`grep -c ^processor /proc/cpuinfo`

# Base directory.
BASEDIR=$1

# Contrib tools.
QEMU=qemu-2.9.0

#==============================================================================
# usage()
#==============================================================================

#
# Prints script usage and exits.
#
function usage
{
	echo "setup-contrib.sh <basedir>"
	exit
}

#==============================================================================
# setup_qemu()
#==============================================================================

#
# Setups QEMU.
#   $1 Installation directory.
#
function setup_qemu
{
	local basedir=`pwd`
	local installdir=$basedir/$1

	# Create installation directory.
	mkdir -p $installdir

	echo $installdir

	cd contrib

	cd $QEMU

	rm -rf build
	mkdir build
	cd build

	../configure                   \
		--prefix=$installdir       \
		--target-list=or1k-softmmu \

	make -j $ncores all

	make install
	
	cd $basedir
}

#==============================================================================
# MAIN
#==============================================================================

# Mussing base directory.
if [ -z $BASEDIR ]; then
	echo "missing base directory"
	usage
fi

setup_qemu $BASEDIR/qemu
