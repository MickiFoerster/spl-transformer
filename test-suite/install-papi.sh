#!/bin/sh
if [ ! -f papi-5.3.0.tar.gz ]; then wget "http://icl.cs.utk.edu/projects/papi/downloads/papi-5.3.0.tar.gz"; fi
if [ ! -f papi-5.3.0.tar.gz ]; then 
	echo ;
	echo ;
	echo "The PAPI 5.3.0 software could not be downloaded.";
	echo "We mark this by creating a file PAPI_NOT_SUCCESSFUL.";
    echo "This means that the test suite only performs runtime tests based on wall time." ;
	echo "In case that you want to try to download PAPI again, please remove the file PAPI_NOT_SUCCESSFUL.";
	echo ;
	touch PAPI_NOT_SUCCESSFUL;
	exit 1;
fi
if [ ! -d papi-5.3.0 ]; then tar xfz papi-5.3.0.tar.gz; fi
if [ -d papi-5.3.0 -a -d papi-5.3.0/src ]; 
then 
	base=`pwd`
	rm -f PAPI_NOT_SUCCESSFUL PAPI_SUCCESSFUL
	cd papi-5.3.0/src && 
	./configure && 
	make && 
	make test ;
	if [ "$?" -ne "0" ]; then 
		echo PAPI test failed; 
		touch $base/PAPI_NOT_SUCCESSFUL;
	else 
		echo PAPI test successful; 
		touch $base/PAPI_SUCCESSFUL;
	fi;
	#export PAPISRC=$PWD/papi-5.3.0/src
fi
