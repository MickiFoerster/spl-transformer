SPLc=$$SPLDIR/SPLc
DUMMY=$$SPLDIR/dummy.o
PRETTY_PRINTER=$(shell echo $$PWD/../../../../prettyprinter/prettyprinter.pl)
EXTRACTOR=$$SPLDIR/test-suite/extract-parallel-region.pl
PAPISRC=$(shell echo $$PAPISRC)
TESTSUITE=$$SPLDIR/test-suite
include $(shell if [ "$$CC" = "icc" ];then printf $$SPLDIR/test-suite/intel-makefile.inc; else printf $$SPLDIR/test-suite/gcc-makefile.inc;fi)
BASENAME_CXX=$(shell if [ -n "`echo $$CXX | sed 's/\([^\ ]*\).*/\1/'`" ];then basename `echo $$CXX | sed 's/\([^\ ]*\).*/\1/'`;else echo g++-$(GCC_VERSION);fi)
LD_FLAGS=-L$(PAPISRC) -lpapi -L$(PAPISRC)/libpfm4/lib -lpfm

all : fof for rof

fof : test.in fof.cpp
	$(SPLc) $< -O3
	$(SPLc) t1_$<.spl -O3 --suppress-global-region --papi
	$(CXX) $(O3_FLAGS) $(CXXFLAGS)  -o $@ -DPAPI -I$(PAPISRC) -I$(SPLDIR) -I$(TESTSUITE) $@.cpp $(DUMMY) $(LD_FLAGS)

for : test.in for.cpp
	$(SPLc) $< -O3
	$(SPLc) a1_$<.spl -O3 --suppress-global-region --papi
	perl $$SPLDIR/stacks2C.pl  t2_a1_$<.spl.spl > t2_a1_$<.spl.spl.withCstack
	$(CXX) $(O3_FLAGS) $(CXXFLAGS)  -o $@ -DPAPI -I$(PAPISRC) -I$(SPLDIR) -I$(TESTSUITE) $@.cpp $(DUMMY) $(LD_FLAGS)

rof : test.in rof.cpp
	$(SPLc) $< -O3
	$(SPLc) t1_$<.spl -O3 --suppress-global-region --papi
	perl $$SPLDIR/stacks2C.pl  a2_t1_$<.spl.spl > a2_t1_$<.spl.spl.withCstack
	$(CXX) $(O3_FLAGS) $(CXXFLAGS)  -o $@ -DPAPI -I$(PAPISRC) -I$(SPLDIR) -I$(TESTSUITE) $@.cpp $(DUMMY) $(LD_FLAGS)



t2_t1_test.in.spl.spl.prettyprinted : test.in
	cd $$SPLDIR ;make; cd -; 
	$$SPLDIR/SPLc $<  
	$$SPLDIR/SPLc t1_$<.spl  --suppress-global-region 
	perl $(PRETTY_PRINTER) t2_t1_$<.spl.spl > $@ 

t2_a1_test.in.spl.spl.prettyprinted : test.in
	cd $$SPLDIR ;make; cd -; 
	$$SPLDIR/SPLc $<  
	$$SPLDIR/SPLc a1_$<.spl  --suppress-global-region 
	perl $(PRETTY_PRINTER) t2_a1_$<.spl.spl > $@ 

a2_t1_test.in.spl.spl.prettyprinted : test.in
	cd $$SPLDIR ;make; cd -; 
	$$SPLDIR/SPLc $<  
	$$SPLDIR/SPLc t1_$<.spl  --suppress-global-region 
	perl $(PRETTY_PRINTER) a2_t1_$<.spl.spl > $@ 


clean : 
	rm -f fof for rof
