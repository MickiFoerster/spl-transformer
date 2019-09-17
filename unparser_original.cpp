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
#include "ast_vertex.h"

#if 0
void 
unparser_PARALLEL_REGION_OMP_original(void* argv)  
{
	DEFINE_UNPARSER;
	MAKE_INDENT;
	unparser->ofs << "#pragma parallel\n";
	MAKE_INDENT;
	unparser->ofs << "{\n";
	INC_INDENT;
	UNPARSE_FIRST_TEST(VT_SEQ_DECLARATIONS);
	UNPARSE_SECOND_TEST(VT_S); 
	DEC_INDENT;
	MAKE_INDENT;
	unparser->ofs << "}\n";
}

void 
unparser_PARALLEL_FOR_REGION_OMP_original(void* argv)  
{
	DEFINE_UNPARSER;
	MAKE_INDENT;
	unparser->ofs << "#pragma parallel for\n";
	MAKE_INDENT;
	unparser->ofs << "for(";   UNPARSE_FIRST_TEST(VT_FOR_LOOP_HEADER);   unparser->ofs << ")\n"; 
	MAKE_INDENT; unparser->ofs << "{\n";
	INC_INDENT;
	UNPARSE_SECOND_TEST(VT_SEQ_DECLARATIONS); 
	UNPARSE_THIRD_TEST(VT_S);
	DEC_INDENT;
	MAKE_INDENT;
	unparser->ofs << "}\n";
}	

void 
unparser_SEQ_DECLARATIONS_original(void* argv)  
{
	DEFINE_UNPARSER;
	UNPARSE_ALL; 
}

void 
unparser_S_original(void* argv) 
{ 
	DEFINE_UNPARSER;
	MAKE_INDENT; unparser->ofs << "// unparser_S_original start (id "<<v->get_id() <<")" << endl;
	for(list<ast_vertex*>::const_iterator it=unparser->v->children.begin();it!=unparser->v->children.end();it++) { 
		assert(*it); 
		unparser->v = *it; 
		MAKE_INDENT;
		(*it)->unparse(argv); 
		unparser->v=v; 
		if( (*it)->type!=VT_SPL_cond_if  &&  (*it)->type!=VT_SPL_cond_while ) unparser->ofs << ";";
		unparser->ofs << endl;
	}
	MAKE_INDENT; unparser->ofs << "// unparser_S_original end (id "<<v->get_id() <<")"  <<  endl;
}

#endif
