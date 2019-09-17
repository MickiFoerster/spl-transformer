/* Copyright (c) 2013, 2014 Michael Foerster, 
   <Michael.Foerster@com-science.de>

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <iostream>
#include <fstream>
#include <sstream>
#include <cassert>
#include <cstdio>
#include <cstdlib>

#include "AST.h"
using namespace std;

int order_of_origin_code=-1;
extern int yyparse();
extern void yyrestart(FILE*);
extern void scan_begin(const char * cmd);
extern void scan_end ();

bool option_long_double=0;
bool option_mpfr=0;
bool option_suppress_global_region=0;
bool option_papi=0;
bool option_time=0;
bool option_prg_analysis=0;
bool option_mem_statistic=0;
bool option_suppress_atomic=0;

int main(int argc, char* argv[])
{
	string src_filename;
	ostringstream oss;
	int rc=-1;
	if( argc==1 ) { // From stdin, then print prompt
		size_t found;
		char c;
		string inputbuffer;
		cout << "Type in a SPL code and finish your input with END:\n";
		//cout << ">>>>>>>>>";
		do {
			c=getc(stdin);
			inputbuffer+=c;
			found=inputbuffer.find("END");
			if (found!=std::string::npos) {
				inputbuffer = inputbuffer.substr(0, found);
				break;
			}
		} while (c!=EOF);
		scan_begin(inputbuffer.c_str());
		rc=yyparse(); 
		if(!rc) fprintf(stderr,"Parsing was successful.\n");
		else    fprintf(stderr,"Error during parsing.\n");
		scan_end();
		src_filename = "stdin";
	} 
	else {
		const int number_of_options = 12;
		const string options[number_of_options] = {
													"--help", 
													"--ld", 
													"--mpfr", 
													"--suppress-global-region", 
													"--papi", 
													"--time", 
													"-O0", "-O1", "-O2", "-O3", 
													"--mem",
													"--suppress-atomic",
												  };
		for(int i=1;i<argc;i++) {
			if( string(argv[i]) == options[0] ) {
				cout << "SPLc (c) 2013 Michael Foerster (michael.foerster@com-science.de)\n";
				cout << "Syntax:  "<<argv[0]<<"   <input file>   <options> \n";
				cout << "Help:\n";
				for(int j=0;j<number_of_options;j++)
					cout << options[j] << endl;
				return 0;
			}
		}
		FILE* fp=NULL;
		src_filename = argv[1];
		for(int i=2;i<argc;i++) {
			     if( string(argv[i]) == options[1] )
				option_long_double=1;
			else if( string(argv[i]) == options[2] )
				option_mpfr=1;
			else if( string(argv[i]) == options[3] )
				option_suppress_global_region=1;
			else if( string(argv[i]) == options[4] )
				option_papi=1;
			else if( string(argv[i]) == options[5] )
				option_time=1;
			else if( string(argv[i]) == options[6] )
				option_prg_analysis=0;
			else if( string(argv[i]) == options[7] )
				option_prg_analysis=1;
			else if( string(argv[i]) == options[8] )
				option_prg_analysis=1;
			else if( string(argv[i]) == options[9] )
				option_prg_analysis=1;
			else if( string(argv[i]) == options[10] )
				option_mem_statistic=1;
			else if( string(argv[i]) == options[11] )
				option_suppress_atomic=1;
		}
		fp=fopen(argv[1], "r"); 
		if(fp==NULL) { cerr << "File " << argv[1] << " cannot be opened.\n"; return 1; }
		assert(fp);
		yyrestart(fp); 
		rc=yyparse(); 	
		if(!rc) fprintf(stderr,"Parsing was successful.\n");
		else    fprintf(stderr,"Error during parsing.\n");
		fclose(fp);
	}

	if(!rc) { // if parsing was successful
		assert(ast.valid());

		// Call program analysis
		if( option_prg_analysis )
			ast.perform_prg_analysis();

		// unparse ast
		ast.unparse( src_filename+".spl", unparse_original);


		// unparse tangent-linear code
		oss.str("");
		int i;
		for(i=src_filename.length(); i>=0 && src_filename[i]!='/'; i--)
			;
		i++;
		if(i>0)
			oss << src_filename.substr(0,i);
		oss << tl_prefix << src_filename.substr(i) << ".spl";
		ast.unparse(oss.str().c_str(), unparse_tangent_linear);

		// unparse adjoint code
		oss.str("");
		if(i>0)
			oss << src_filename.substr(0,i);
		oss << adj_prefix << src_filename.substr(i) << ".spl";
		ast.unparse(oss.str().c_str(), unparse_adjoint);

	}
	else 
		cerr << "Error while parsing file " << src_filename << endl;
	return rc;
}



