SPLc=$$SPLDIR/SPLc
PRETTY_PRINTER=$(shell echo $$PWD/../../../prettyprinter/prettyprinter.pl)
EXTRACTOR=$$SPLDIR/test-suite/extract-parallel-region.pl
PAPISRC=$(shell echo $$PAPISRC)
DUMMY=$(shell echo $$SPLDIR/dummy)
BIN=$(shell basename `pwd`)
include $(shell if [ "$$CC" = "icc" ];then printf $$SPLDIR/test-suite/intel-makefile.inc; else printf $$SPLDIR/test-suite/gcc-makefile.inc;fi)
# The final version should include the following instead of gcc-4.8.0 from the above include command.
#include $(shell printf $$SPLDIR/test-suite/gcc-std-makefile.inc)
BASENAME_CXX=$(shell if [ -n "`echo $$CXX | sed 's/\([^\ ]*\).*/\1/'`" ];then basename `echo $$CXX | sed 's/\([^\ ]*\).*/\1/'`;else echo g++-$(GCC_VERSION);fi)
BIN_O0=$(BIN)-O0-$(BASENAME_CXX)
BIN_O3=$(BIN)-O3-$(BASENAME_CXX)
BIN_O0_PAPI=$(BIN)-O0-papi-$(BASENAME_CXX)
BIN_O3_PAPI=$(BIN)-O3-papi-$(BASENAME_CXX)
INC_PATH=-I$$PWD -I$$SPLDIR -I$$SPLDIR/test-suite 

test_suite:
	if [ ! -f $$SPLDIR/test-suite/PAPI_NOT_SUCCESSFUL -a ! -f $$SPLDIR/test-suite/PAPI_SUCCESSFUL ];then cd $$SPLDIR/test-suite && ./install-papi.sh;fi 
	@printf " CC             : $(CC)\n"
	@printf " CXX            : $(CXX)\n"
	@printf " CCFLAGS        : $(CCFLAGS)\n"
	@printf " CXXFLAGS       : $(CXXFLAGS)\n"
	@printf " O0 flags       : $(O0_FLAGS)\n"
	@printf " O3 flags       : $(O3_FLAGS)\n"
	@printf " Linker         : $(LDFLAGS)\n"
	@printf " LD_LIBRARY_PATH: $(LD_LIBRARY_PATH)\n"
	/bin/bash -c 'if [ "`basename $(shell pwd)`" == "test-suite" ];then  \
		for i in *; \
		do \
			if [ -d $$i ];then \
				printf "\e[1;31mEntering test $$i ... \e[0;0m\n" ;\
				cd $$i;\
				make scale_test;\
				printf "\e[1;31mLeaving test $$i.\e[0;0m\n" ; \
				cd -;\
			fi;\
		done; \
	else \
		make scale_test ; \
	fi'
	@/bin/bash -c 'printf "Target $@ was processed \e[1;32msuccessfully\e[0;0m.\n"'


finite_differences_test : finite_diff_testapp prerequirements 
	env LD_LIBRARY_PATH=$(LD_LIBRARY_PATH):$$LD_LIBRARY_PATH ./$<  && touch $@


# OFF:   md5test   finite_differences_test
scale_test :    $(BIN_O0_PAPI)   $(BIN_O3_PAPI)   $(BIN_O0)   $(BIN_O3)   ../drawbars.py  
	if [ ! -f $$SPLDIR/test-suite/PAPI_SUCCESSFUL -a ! -f $$SPLDIR/test-suite/PAPI_NOT_SUCCESSFUL ];then  echo "PAPI has not been installed. We try to download it."; cd $$SPLDIR/test-suite; ./install-papi.sh; cd -; fi
	if [ -f $$SPLDIR/test-suite/PAPI_NOT_SUCCESSFUL ];then env LD_LIBRARY_PATH=$(LD_LIBRARY_PATH):$$LD_LIBRARY_PATH ./$(BIN_O0) ; fi
	if [ -f $$SPLDIR/test-suite/PAPI_NOT_SUCCESSFUL ];then env LD_LIBRARY_PATH=$(LD_LIBRARY_PATH):$$LD_LIBRARY_PATH  ./$(BIN_O3); fi
	if [ -f $$SPLDIR/test-suite/PAPI_SUCCESSFUL ];then env LD_LIBRARY_PATH=$(LD_LIBRARY_PATH):$$LD_LIBRARY_PATH ./$(BIN_O0_PAPI) ; fi
	if [ -f $$SPLDIR/test-suite/PAPI_SUCCESSFUL ];then env LD_LIBRARY_PATH=$(LD_LIBRARY_PATH):$$LD_LIBRARY_PATH  ./$(BIN_O3_PAPI); fi
	@printf "Target $@ was processed \e[1;32msuccessfully\e[0;0m.\n"
	
scale_test_O0 :    $(BIN_O0_PAPI)   $(BIN_O0)    ../drawbars.py  
	if [ -f $$SPLDIR/test-suite/PAPI_NOT_SUCCESSFUL ];then env LD_LIBRARY_PATH=$(LD_LIBRARY_PATH):$$LD_LIBRARY_PATH ./$(BIN_O0); fi
	if [ -f $$SPLDIR/test-suite/PAPI_SUCCESSFUL ];then env LD_LIBRARY_PATH=$(LD_LIBRARY_PATH):$$LD_LIBRARY_PATH ./$(BIN_O0_PAPI); fi
	@printf "Target $@ was processed \e[1;32msuccessfully\e[0;0m.\n"
	
scale_test_O3 :    $(BIN_O3_PAPI)   $(BIN_O3)    ../drawbars.py  
	if [ -f $$SPLDIR/test-suite/PAPI_NOT_SUCCESSFUL ];then env LD_LIBRARY_PATH=$(LD_LIBRARY_PATH):$$LD_LIBRARY_PATH ./$(BIN_O3); fi
	if [ -f $$SPLDIR/test-suite/PAPI_SUCCESSFUL ];then env LD_LIBRARY_PATH=$(LD_LIBRARY_PATH):$$LD_LIBRARY_PATH ./$(BIN_O3_PAPI); fi
	@printf "Target $@ was processed \e[1;32msuccessfully\e[0;0m.\n"
	
scale_test_without_PAPI :   $(BIN_O0)   $(BIN_O3)   ../drawbars.py  
	env LD_LIBRARY_PATH=$(LD_LIBRARY_PATH):$$LD_LIBRARY_PATH ./$(BIN_O0)
	env LD_LIBRARY_PATH=$(LD_LIBRARY_PATH):$$LD_LIBRARY_PATH  ./$(BIN_O3)
	@printf "Target $@ was processed \e[1;32m  successfully  \e[0;0m.\n"
	

clusterjob :
	cat $$SPLDIR/test-suite/job-template-builder.sh | sh > $(BIN).bsub

md5test :  test.in  prerequirements
	rm -f $<.spl t1_$<.spl a1_$<.spl
	$(SPLc) $< -O3
	md5sum -c  md5sums.txt  && touch $@
	rm -f $<.spl t1_$<.spl a1_$<.spl

original_unparse_test :  test.in  prerequirements
	$(SPLc) $<  -O3
	$(EXTRACTOR) $<     > a
	$(EXTRACTOR) $<.spl > b
	@diff --ignore-tab-expansion --ignore-space-change --ignore-all-space   a b;if [ $$? == 0 ];then printf "\e[1;32mOriginal code and unparsed code are the same.\e[0;0m\n";rm -f a b; else printf "\e[1;31mOriginal code and unparsed code differs!\e[0;0m\n";vimdiff a b;fi

md5sums.txt : test.in  prerequirements
	rm -f $<.spl t1_$<.spl a1_$<.spl
	$(SPLc) $<  -O3
	for i in  $<.spl  t1_$<.spl  a1_$<.spl;do md5sum $$i;done > $@
	for i in  $<.spl  t1_$<.spl  a1_$<.spl;do mv $$i `basename $$i .spl`.ref;git add $@ `basename $$i .spl`.ref;done

#  test.in.spl.withCstack  t1_test.in.spl.withCstack  a1_test.in.spl.withCstack 
finite_diff_testapp : ../finite_differences_test.cpp  prerequirements
	make clean;
	$(SPLc) test.in  --suppress-global-region --ld -O3
	perl $$SPLDIR/stacks2C.pl  test.in.spl > test.in.spl.withCstack
	perl $$SPLDIR/stacks2C.pl  t1_test.in.spl > t1_test.in.spl.withCstack
	perl $$SPLDIR/stacks2C.pl  a1_test.in.spl > a1_test.in.spl.withCstack
	$(CXX) $(O0_FLAGS) $(CXXFLAGS)  $(INC_PATH)  $<   $(DUMMY).o  -o $@

$(BIN_O0) : ../driver.cpp test.in.spl.withCstack  t1_test.in.spl.withCstack  a1_test.in.spl.withCstack  test.in.papi.withCstack  t1_test.in.papi.withCstack  a1_test.in.papi.withCstack  prerequirements
	$(CXX) $(O0_FLAGS) $(CXXFLAGS)  $(INC_PATH)  $<    $(DUMMY).o -o $@

$(BIN_O3) : ../driver.cpp test.in.spl.withCstack  t1_test.in.spl.withCstack  a1_test.in.spl.withCstack  test.in.papi.withCstack  t1_test.in.papi.withCstack  a1_test.in.papi.withCstack  prerequirements
	$(CXX) $(O3_FLAGS) $(CXXFLAGS)   $(INC_PATH)  $<   $(DUMMY).o -o $@


adjoint_scale_test_O0_without_ERA : ../driver.cpp test.in.spl.withCstack  t1_test.in.spl.withCstack  a1_test.in.spl.withCstack  prerequirements
	$$SPLDIR/SPLc test.in -O0 --papi --suppress-global-region --mem && for i in test.in.spl t1_test.in.spl a1_test.in.spl;do mv $$i `basename $$i .spl`.papi;done
	perl $$SPLDIR/stacks2C.pl `basename test.in.papi.withCstack .withCstack` > test.in.papi.withCstack	
	perl $$SPLDIR/stacks2C.pl `basename t1_test.in.papi.withCstack .withCstack` > t1_test.in.papi.withCstack	
	perl $$SPLDIR/stacks2C.pl `basename a1_test.in.papi.withCstack .withCstack` > a1_test.in.papi.withCstack	
	$(CXX) $(O0_FLAGS) $(CXXFLAGS)      $(INC_PATH) -I$(PAPISRC) -DPAPI -DONLY_ADJOINT $<  $(DUMMY).o  -o $@ -L$(PAPISRC) -lpapi -L$(PAPISRC)/libpfm4/lib -lpfm

adjoint_scale_test_O3_without_ERA : ../driver.cpp test.in.spl.withCstack  t1_test.in.spl.withCstack  a1_test.in.spl.withCstack  prerequirements
	$$SPLDIR/SPLc test.in -O0 --papi --suppress-global-region --mem && for i in test.in.spl t1_test.in.spl a1_test.in.spl;do mv $$i `basename $$i .spl`.papi;done
	perl $$SPLDIR/stacks2C.pl `basename test.in.papi.withCstack .withCstack` > test.in.papi.withCstack	
	perl $$SPLDIR/stacks2C.pl `basename t1_test.in.papi.withCstack .withCstack` > t1_test.in.papi.withCstack	
	perl $$SPLDIR/stacks2C.pl `basename a1_test.in.papi.withCstack .withCstack` > a1_test.in.papi.withCstack	
	$(CXX) $(O3_FLAGS) $(CXXFLAGS)      $(INC_PATH) -I$(PAPISRC) -DPAPI  -DONLY_ADJOINT $<  $(DUMMY).o  -o $@ -L$(PAPISRC) -lpapi -L$(PAPISRC)/libpfm4/lib -lpfm


adjoint_scale_test_O0 : ../driver.cpp test.in.papi.withCstack  t1_test.in.papi.withCstack  a1_test.in.papi.withCstack   test.in.spl.withCstack  t1_test.in.spl.withCstack  a1_test.in.spl.withCstack  prerequirements
	$(CXX) $(O0_FLAGS) $(CXXFLAGS)      $(INC_PATH) -I$(PAPISRC) -DPAPI -DONLY_ADJOINT $<  $(DUMMY).o  -o $@ -L$(PAPISRC) -lpapi -L$(PAPISRC)/libpfm4/lib -lpfm
adjoint_scale_test_O3 : ../driver.cpp test.in.papi.withCstack  t1_test.in.papi.withCstack  a1_test.in.papi.withCstack   test.in.spl.withCstack  t1_test.in.spl.withCstack  a1_test.in.spl.withCstack  prerequirements
	$(CXX) $(O3_FLAGS) $(CXXFLAGS)      $(INC_PATH) -I$(PAPISRC) -DPAPI  -DONLY_ADJOINT $<  $(DUMMY).o  -o $@ -L$(PAPISRC) -lpapi -L$(PAPISRC)/libpfm4/lib -lpfm


# without PAPI
adjoint_scale_test_O0_without_ERA_without_PAPI : ../driver.cpp prerequirements
	$(SPLc) test.in --suppress-global-region --time --mem -O0 
	perl $$SPLDIR/stacks2C.pl test.in.spl    > test.in.spl.withCstack	
	perl $$SPLDIR/stacks2C.pl t1_test.in.spl > t1_test.in.spl.withCstack	
	perl $$SPLDIR/stacks2C.pl a1_test.in.spl > a1_test.in.spl.withCstack	
	$$SPLDIR/SPLc test.in -O0 --suppress-global-region --mem && for i in test.in.spl t1_test.in.spl a1_test.in.spl;do mv $$i `basename $$i .spl`.papi;done
	perl $$SPLDIR/stacks2C.pl `basename test.in.papi.withCstack .withCstack` > test.in.papi.withCstack	
	perl $$SPLDIR/stacks2C.pl `basename t1_test.in.papi.withCstack .withCstack` > t1_test.in.papi.withCstack	
	perl $$SPLDIR/stacks2C.pl `basename a1_test.in.papi.withCstack .withCstack` > a1_test.in.papi.withCstack	
	$(CXX) $(O0_FLAGS) $(CXXFLAGS)      $(INC_PATH) -DONLY_ADJOINT $<  $(DUMMY).o  -o $@ 

adjoint_scale_test_O3_without_ERA_without_PAPI : ../driver.cpp prerequirements
	$(SPLc) test.in --suppress-global-region --time --mem -O0 
	perl $$SPLDIR/stacks2C.pl test.in.spl    > test.in.spl.withCstack	
	perl $$SPLDIR/stacks2C.pl t1_test.in.spl > t1_test.in.spl.withCstack	
	perl $$SPLDIR/stacks2C.pl a1_test.in.spl > a1_test.in.spl.withCstack	
	$$SPLDIR/SPLc test.in -O0 -suppress-global-region --mem && for i in test.in.spl t1_test.in.spl a1_test.in.spl;do mv $$i `basename $$i .spl`.papi;done
	perl $$SPLDIR/stacks2C.pl `basename test.in.papi.withCstack .withCstack` > test.in.papi.withCstack	
	perl $$SPLDIR/stacks2C.pl `basename t1_test.in.papi.withCstack .withCstack` > t1_test.in.papi.withCstack	
	perl $$SPLDIR/stacks2C.pl `basename a1_test.in.papi.withCstack .withCstack` > a1_test.in.papi.withCstack	
	$(CXX) $(O3_FLAGS) $(CXXFLAGS)      $(INC_PATH) -DONLY_ADJOINT $<  $(DUMMY).o  -o $@ 


adjoint_scale_test_O0_without_PAPI : ../driver.cpp   prerequirements
	$(SPLc) test.in --suppress-global-region --time --mem -O3 
	perl $$SPLDIR/stacks2C.pl test.in.spl    > test.in.spl.withCstack	
	perl $$SPLDIR/stacks2C.pl t1_test.in.spl > t1_test.in.spl.withCstack	
	perl $$SPLDIR/stacks2C.pl a1_test.in.spl > a1_test.in.spl.withCstack	
	$(SPLc) test.in -O3 -suppress-global-region --mem && for i in test.in.spl t1_test.in.spl a1_test.in.spl;do mv $$i `basename $$i .spl`.papi;done
	perl $$SPLDIR/stacks2C.pl `basename test.in.papi.withCstack .withCstack` > test.in.papi.withCstack	
	perl $$SPLDIR/stacks2C.pl `basename t1_test.in.papi.withCstack .withCstack` > t1_test.in.papi.withCstack	
	perl $$SPLDIR/stacks2C.pl `basename a1_test.in.papi.withCstack .withCstack` > a1_test.in.papi.withCstack	
	$(CXX) $(O0_FLAGS) $(CXXFLAGS)      $(INC_PATH) -DONLY_ADJOINT $<  $(DUMMY).o  -o $@ 
adjoint_scale_test_O3_without_PAPI : ../driver.cpp prerequirements
	$(SPLc) test.in --suppress-global-region --time --mem -O3 
	perl $$SPLDIR/stacks2C.pl test.in.spl    > test.in.spl.withCstack	
	perl $$SPLDIR/stacks2C.pl t1_test.in.spl > t1_test.in.spl.withCstack	
	perl $$SPLDIR/stacks2C.pl a1_test.in.spl > a1_test.in.spl.withCstack	
	$(SPLc) test.in -O3 -suppress-global-region --mem && for i in test.in.spl t1_test.in.spl a1_test.in.spl;do mv $$i `basename $$i .spl`.papi;done
	perl $$SPLDIR/stacks2C.pl `basename test.in.papi.withCstack .withCstack` > test.in.papi.withCstack	
	perl $$SPLDIR/stacks2C.pl `basename t1_test.in.papi.withCstack .withCstack` > t1_test.in.papi.withCstack	
	perl $$SPLDIR/stacks2C.pl `basename a1_test.in.papi.withCstack .withCstack` > a1_test.in.papi.withCstack	
	$(CXX) $(O3_FLAGS) $(CXXFLAGS)      $(INC_PATH) -DONLY_ADJOINT $<  $(DUMMY).o  -o $@ 


# without PAPI END




adjoint_test_app_noop : ../adjoint-test-wt-p-threads.cpp a1_test.in.papi.withCstack  a1_test.in.spl.withCstack  prerequirements
	$(SPLc) test.in --suppress-global-region --time --mem -O0
	$(CXX) $(O0_FLAGS) $(CXXFLAGS)   $(INC_PATH) -I$(PAPISRC) -DPAPI   $<   $(DUMMY).o -o $@ -L$(PAPISRC) -lpapi -L$(PAPISRC)/libpfm4/lib -lpfm

adjoint_test_app_opt : ../adjoint-test-wt-p-threads.cpp a1_test.in.papi.withCstack a1_test.in.spl.withCstack prerequirements
	$(SPLc) test.in --suppress-global-region --time --mem -O3
	$(CXX) $(O0_FLAGS) $(CXXFLAGS)   $(INC_PATH) -I$(PAPISRC) -DPAPI   $<   $(DUMMY).o -o $@ -L$(PAPISRC) -lpapi -L$(PAPISRC)/libpfm4/lib -lpfm

test.in.spl: test.in 
	cd $$SPLDIR && make
	$(SPLc) $< --suppress-global-region --time --mem -O3
	#$(SPLc) $< --suppress-global-region --time --mem -O3 --suppress-atomic

test.in.spl.withCstack : test.in.spl
	perl $$SPLDIR/stacks2C.pl `basename $@ .withCstack` > $@	
t1_test.in.spl.withCstack : test.in.spl
	perl $$SPLDIR/stacks2C.pl `basename $@ .withCstack` > $@	
a1_test.in.spl.withCstack : test.in.spl
	perl $$SPLDIR/stacks2C.pl `basename $@ .withCstack` > $@	

$(BIN_O0_PAPI) : ../driver.cpp test.in.papi.withCstack  t1_test.in.papi.withCstack  a1_test.in.papi.withCstack   test.in.spl.withCstack  t1_test.in.spl.withCstack  a1_test.in.spl.withCstack  prerequirements
	if [ -f $$SPLDIR/test-suite/PAPI_SUCCESSFUL ];then $(CXX) $(O0_FLAGS) $(CXXFLAGS)      $(INC_PATH) -I$(PAPISRC) -DPAPI  $<  $(DUMMY).o  -o $@ -L$(PAPISRC) -lpapi -L$(PAPISRC)/libpfm4/lib -lpfm; fi

$(BIN_O3_PAPI) : ../driver.cpp test.in.papi.withCstack  t1_test.in.papi.withCstack  a1_test.in.papi.withCstack   test.in.spl.withCstack  t1_test.in.spl.withCstack  a1_test.in.spl.withCstack  prerequirements
	if [ -f $$SPLDIR/test-suite/PAPI_SUCCESSFUL ];then $(CXX) $(O3_FLAGS) $(CXXFLAGS)      $(INC_PATH) -I$(PAPISRC)  -DPAPI $<   $(DUMMY).o  -o $@ -L$(PAPISRC) -lpapi -L$(PAPISRC)/libpfm4/lib -lpfm; fi

test.in.papi: test.in 
	cd $$SPLDIR && make
	$(SPLc) $< -O3 --papi --suppress-global-region --mem && for i in $<.spl t1_$<.spl a1_$<.spl;do mv $$i `basename $$i .spl`.papi;done

test.in.papi.withCstack : test.in.papi
	perl $$SPLDIR/stacks2C.pl `basename $@ .withCstack` > $@	
t1_test.in.papi.withCstack : test.in.papi
	perl $$SPLDIR/stacks2C.pl `basename $@ .withCstack` > $@	
a1_test.in.papi.withCstack : test.in.papi
	perl $$SPLDIR/stacks2C.pl `basename $@ .withCstack` > $@	

$(DUMMY).o : $(DUMMY).c
	cd $$SPLDIR && $(CXX) -c dummy.c ;

$(SPLc) : 
	cd $$SPLDIR && make

prerequirements : $(SPLc) $(DUMMY).o
	cd $$SPLDIR && $(CXX) -c dummy.c ;
	targetfile=test_specific_decl.c;if [ ! -f $$targetfile ];then touch $$targetfile;fi; 
	targetfile=test_specific_init.c;if [ ! -f $$targetfile ];then touch $$targetfile;fi; 
	#-if [ "`hostname | perl -n -e 'if( m/\.rz\.RWTH-Aachen\.de/i ) {print "cluster\n";}'`" == "cluster" ];then module unload intel;module unload gcc;module load intel/13.1;fi

plots : ../drawbars.py ../draw_memstats.py
	python ../drawbars.py *.plot;
	python ../draw_memstats.py *mem.plot

t2_t1_test.in.spl.spl : test.in
	cd $$SPLDIR && make
	$(SPLc) $< -O3 
	$(SPLc) t1_$<.spl -O3 --papi --suppress-global-region --mem 

test.in.spl.prettyprinted : test.in 
	cd $$SPLDIR ;make; cd -; 
	$$SPLDIR/SPLc $<  --suppress-global-region 
	perl $(PRETTY_PRINTER) $< > $@ 

t1_test.in.spl.prettyprinted : test.in
	perl $(PRETTY_PRINTER) t1_test.in.spl > $@ 

a1_test.in.spl.prettyprinted : test.in
	perl $(PRETTY_PRINTER) a1_test.in.spl > $@ 

clean:
	if [ "`basename $$PWD`" != "test-suite" ];then rm -f  \
		*.withCstack \
		*.spl \
		*.papi \
		*.eps \
		*.png \
		ast*.pdf \
		cfg*.pdf \
		*.out  \
		a1_symbol_table.txt \
		symdump.txt \
		t1_symbol_table.txt \
		symbol_table.txt \
		vgcore* core* \
		$(BIN)-O*    \
		finite_diff_testapp  \
		adjoint_test adjoint_test_app \
		exclusive-read-*.txt; \
	fi

clean-all :
	make clean
	rm -f finite_differences_test md5sums.txt md5test

.PHONY: clean scale_test test_suite clean-all  original_unparse_test prerequirements plots


