#!/usr/bin/env zsh
#BSUB -J plain-parallel
#BSUB -o job%J-plain-parallel.stdout
#BSUB -e job%J-plain-parallel.stderr
#BSUB -u foerster@stce.rwth-aachen.de
#BSUB -N
#BSUB -R "select[hpcwork]"
#BSUB -M 1020000
#BSUB -W 4500
#BSUB -n 128
#BSUB -a "bcs openmp"
#BSUB -x
echo "Starting job at:"; date;
unset OMP_THREAD_LIMIT
export SPLDIR=/home/mf510951/phd/code/compiler/spl-transformer
export PAPISRC=/home/mf510951/workspace/external-projects/papi-5.1.0/src
cd $PAPISRC
make test
export testname=plain-parallel
export SPLcOnHPCWORK=$HPCWORK/spl-transformer-job-$RANDOM
rsync -av $SPLDIR/ $SPLcOnHPCWORK
export SPLDIR=$SPLcOnHPCWORK
cd $SPLcOnHPCWORK;
module unload gcc
module unload intel
module load gcc/4.8
#module load intel/14.0b
#module load intel/13.1.2.183
rm -rf CMakeCache.txt CMakeFiles cmake_install*
cmake CMakeLists.txt && make;
cd test-suite;
rm -f hostconfig.txt
hostname >>  hostconfig.txt; echo >>  hostconfig.txt; echo >>  hostconfig.txt; echo >>  hostconfig.txt;
cat /proc/cpuinfo >>  hostconfig.txt; echo >>  hostconfig.txt; echo >>  hostconfig.txt; echo >>  hostconfig.txt;
cat /proc/meminfo >>  hostconfig.txt; echo >>  hostconfig.txt; echo >>  hostconfig.txt; echo >>  hostconfig.txt;
if [ "$testname" != "test-suite" ]; then cd $testname; fi
make clean ; BASE=`pwd`; export LD_LIBRARY_PATH=$HOME/lib:$LD_LIBRARY_PATH
#echo "Start adjoint_scale_test_O0_without_ERA test at:"; date; make clean adjoint_scale_test_O0_without_ERA; echo "Test how many atomic constructs we have in adjoint code"; grep atomic a1_test.in.papi; ./adjoint_scale_test_O0_without_ERA; echo "End adjoint_scale_test_O0_without_ERA test at:"; date;
#echo "Start adjoint_scale_test_O3_without_ERA test at:"; date; make clean adjoint_scale_test_O3_without_ERA; echo "Test how many atomic constructs we have in adjoint code"; grep atomic a1_test.in.papi; ./adjoint_scale_test_O3_without_ERA; echo "End adjoint_scale_test_O3_without_ERA test at:"; date;
#echo "Start adjoint_scale_test_O0 test at:"; date; make clean adjoint_scale_test_O0; echo "Test how many atomic constructs we have in adjoint code"; grep atomic a1_test.in.papi; ./adjoint_scale_test_O0; echo "End adjoint_scale_test_O0 test at:"; date;
echo "Start adjoint_scale_test_O3 test at:"; date; make clean adjoint_scale_test_O3; echo "Test how many atomic constructs we have in adjoint code"; grep atomic a1_test.in.papi; ./adjoint_scale_test_O3 echo "End adjoint_scale_test_O3 test at:"; date;

