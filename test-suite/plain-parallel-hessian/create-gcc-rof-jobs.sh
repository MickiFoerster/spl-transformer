cat << EOM
#!/usr/bin/env zsh
#BSUB -J plain-parallel-hessian-rof-p$1
#BSUB -o job%J-plain-parallel-hessian-rof.stdout
#BSUB -e job%J-plain-parallel-hessian-rof.stderr
#BSUB -u foerster@stce.rwth-aachen.de
#BSUB -N
#BSUB -R "select[hpcwork]"
#BSUB -M 1024000 
#BSUB -W 600
#BSUB -n $1
#BSUB -a "bcs openmp"
echo "Starting job at:"; date;
unset OMP_THREAD_LIMIT
export SPLDIR=/home/mf510951/phd/code/compiler/spl-transformer
export PAPISRC=/home/mf510951/workspace/external-projects/papi-5.3.0/src
module unload gcc
module unload intel
#module load gcc/4.8
module load intel/14.0
cd /home/mf510951/plain-parallel-hessian
BASE=/home/mf510951/phd/code/compiler/spl-transformer/test-suite/plain-parallel-hessian; export LD_LIBRARY_PATH=/home/mf510951/lib:/home/mf510951/programs/gcc-4.7.2/lib64:/usr/local_rwth/sw/gcc/4.8.1/lib64:/usr/local_rwth/sw/gcc/4.8.1/lib:/usr/local/cuda/lib64:/home/mf510951/lib:/usr/lib64/qt4/lib64:/home/mf510951/lib:/opt/lsf/9.1/linux2.6-glibc2.3-x86_64/lib
cd \$BASE
jobdir=/hpcwork/mf510951/plain-parallel-hessian-job-with-$1-threads-rof-id\$RANDOM; 
mkdir \$jobdir && echo "Successfully created folder \$jobdir.";
if [ -d \$jobdir ];
then
	cp -rv \$BASE/* \$jobdir;
	cd \$jobdir
	rm -f hostconfig.txt
	hostname >>  hostconfig.txt; echo >>  hostconfig.txt; echo >>  hostconfig.txt; echo >>  hostconfig.txt;
	cat /proc/cpuinfo >>  hostconfig.txt; echo >>  hostconfig.txt; echo >>  hostconfig.txt; echo >>  hostconfig.txt;
	cat /proc/meminfo >>  hostconfig.txt; echo >>  hostconfig.txt; echo >>  hostconfig.txt; echo >>  hostconfig.txt;
	make clean;
	make rof;
	echo "make process errorcode: \$?";
	echo "Starting 'plain parallel hessian' test case with $1 threads."
	printf "Working directory: "; pwd;
	export LD_LIBRARY_PATH=\$HOME/lib:\$LD_LIBRARY_PATH;
	\$jobdir/rof ;
fi
echo "Ending job at:"; date;
EOM
