#!/usr/bin/perl
use warnings;
use strict;

while(<>) {
	if( m/([at0-9]+_STACK[cif])\.push/ ) {
		my $stackname = $1; 
		my $C_function = $stackname . "_push";
		s#$stackname\.push#$C_function#;
	}
	elsif( m/([at0-9]+_STACK[cif])\.pop/ ) {
		my $stackname = $1; 
		my $C_function = $stackname . "_pop";
		s#$stackname\.pop#$C_function#;
	}
	elsif( m/([at0-9]+_STACK[cif]).top/ ) {
		my $stackname = $1; 
		my $C_function = $stackname . "_top";
		s#$stackname\.top#$C_function#;
	}
	elsif( m/([at0-9]+_STACK[cif]).empty/ ) {
		my $stackname = $1; 
		my $C_function = $stackname . "_empty";
		s#$stackname\.empty#$C_function#;
	}
	elsif( m/Stack[ci] (.*STACK[ci]);/ ) { 
		my $stackname = $1;
		my $stackcounter = $stackname . "_c";
		s#Stack[ci] $stackname#int *$stackname = NULL; int $stackcounter = 0#;
		$_ .= "#pragma omp threadprivate($stackname,$stackcounter)\n";
	}
	elsif( m/Stackf (.*STACKf);/ ) { 
		my $stackname = $1;
		my $stackcounter = $stackname . "_c";
		s#Stackf $stackname#double *$stackname = NULL; int $stackcounter = 0#;
		$_ .= "#pragma omp threadprivate($stackname,$stackcounter)\n";
	}
	print $_;
}
