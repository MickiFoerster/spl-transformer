#!/usr/bin/perl
use warnings;
use strict;

my $in_omp_parallel=0;
while(<>) {
	s#\t#    #g;
	if( m/^#pragma omp parallel/ ) { 
		print;
		$in_omp_parallel=1; 
		while( <> ) {
			if( m/^{/ ) { print; last; }
			print;
		}
	}
	elsif( $in_omp_parallel==1  and m/^}/ ) {  print; last; }
	elsif( $in_omp_parallel==1 ) {  print; }
}
