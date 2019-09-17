GCC_VERSION=4.8.0
GCC_BASENAME=$(shell if [ -f $$HOME/programs/gcc-$(GCC_VERSION)/bin/g++-$(GCC_VERSION) ];then printf "$$HOME/programs/gcc-$(GCC_VERSION)/bin/g++-$(GCC_VERSION)";else echo "$$HOME/workspace/external-projects/gcc-$(GCC_VERSION)/bld/bin/g++-$(GCC_VERSION)";fi)
GCC_BASE_DIR=$(shell if [ -f $$HOME/programs/gcc-$(GCC_VERSION)/bin/g++-$(GCC_VERSION) ];then printf "$$HOME/programs/gcc-$(GCC_VERSION)";else echo "$$HOME/workspace/external-projects/gcc-$(GCC_VERSION)/bld";fi)
GCC_INCLUDE=-I$(GCC_BASE_DIR)/include/c++/$(GCC_VERSION)
INTEL_INCLUDE=-I $(shell echo $$INCLUDE | sed "s/:/ -I /g")
GCC_LDFLAGS=-L$(GCC_BASE_DIR)/lib64
GCC_LIB_PATH=$(GCC_BASE_DIR)/lib64
O0_FLAGS=$(shell if [ -n "`icpc 2>&1 | egrep '^icpc: command line error'`" ];then echo "-g -O0";else echo "-ggdb3 -O0";fi) 
O3_FLAGS=-O3
CXX=$(shell if [ -n "`icpc 2>&1 | egrep '^icpc: command line error'`" ];then echo icpc;else echo $(GCC_BASENAME);fi)
CXXFLAGS=$(shell if [ -n "`icpc 2>&1 | egrep '^icpc: command line error'`" ];then echo -openmp;else echo -fopenmp;fi) $(INTEL_INCLUDE) 
LDFLAGS=$$FLAGS_INTEL_LINKER 
INTEL_LIB_PATH=$(shell icpcpath=`which icpc`;if [ $$? == 0 ];then echo `dirname $$icpcpath`/../../compiler/lib/intel64;fi)
LD_LIBRARY_PATH=$(INTEL_LIB_PATH):$(GCC_LIB_PATH):$(PAPISRC):$(PAPISRC)/libpfm4/lib:$$LD_LIBRARY_PATH

