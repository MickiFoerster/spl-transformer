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

#include "symbol_table.h"

symbol::symbol(std::string n, ast_vertex* d, bool shared_or_private, bool is_symbol_intermediate, bool is_symbol_new_derivative): 
	is_intermediate(is_symbol_intermediate), 
	new_derivative(is_symbol_new_derivative),
	name(n), 
	decl(d), 
	shared(shared_or_private) , 
	order_of_identifier(-1), 
	derivative_type_of_identifier() 
{
	// Split prefix a2_t1_ from actual name of the symbol.
	string str=n;
	while(str!="") {
		if(str[0]=='a' || str[0]=='t') {
			derivative_model m = (str[0]=='t') ? tangent_linear : adjoint ;
			if( str[1]>='0' && str[1]<='9' ) {
				int order = (int) str[1]-48 ;
				if( str[2]=='_' ) {
					str = str.substr(3);
					derivative_type_of_identifier.push_back(m);
					if(order_of_identifier==-1) 
						order_of_identifier=order;
					else  
						assert(order<order_of_identifier); // The order must decrease from left to right.
				}
				else str="";
			}
			else str="";
		}
		else str="";
	}
	if(order_of_identifier==-1) 
		order_of_identifier=0;
	if( !is_symbol_new_derivative  && !is_symbol_intermediate  && order_of_identifier>order_of_origin_code ) {
		order_of_origin_code=order_of_identifier;
		ostringstream oss;
		oss << "t" << order_of_origin_code+1 << "_";
		tl_prefix = oss.str();
		oss.str(""); oss << "a" << order_of_origin_code+1 << "_";
		adj_prefix = oss.str();
	}
}



void 
symbol_table::clear_intermediates() 
{
	for(map< std::string, symbol* >::iterator it=t.begin();it!=t.end();it++) { 
		symbol* s=it->second;
		if( s->is_intermediate  || s->new_derivative ) {
			t.erase(it); 
			it=t.begin();
			delete s;
		}
	}
}


bool  // true: new symbol was created, false: the symbol was already defined
symbol_table::insert(ast_vertex* d, 
	   bool shared_or_private,  // shared=true, private=false
	   bool is_symbol_intermediate, 
	   bool is_symbol_new_derivative) 
{
	switch(d->get_type()) {
	case VT_INT_DECLARATION:
	case VT_UINT_DECLARATION:
	case VT_FLOAT_DECLARATION:
	case VT_INT_POINTER_DECLARATION:
	case VT_FLOAT_POINTER_DECLARATION:
	case VT_STACKc_DECLARATION: 
	case VT_STACKi_DECLARATION:
	case VT_STACKf_DECLARATION: 
		break;
	default:
		assert(0);
	}
	string n=d->get_lhs_name();
	assert(n.length()>0);
	if( t.count(n)!=0 ) { // If symbol is already contained.
		// In case that the symbol is an intermediate variable or
		// an derivative associate this symbol is not inserted twice.
		// Otherwise the insertion is an error because of using the same
		// declaration name twice.
		if( !is_symbol_intermediate && !is_symbol_new_derivative ) {
			cerr << "error: symbol '"<<n<<"' was previously defined in line " << get_decl(n)->get_line_number() << ".\n";
			assert(t.count(n)==0); // symbol is already defined 
			return false;
		}
		return false;
	}
	assert(t.count(n)==0); // symbol is a new one 
	symbol* sym = new symbol(n, d, shared_or_private, is_symbol_intermediate, is_symbol_new_derivative);
	t[n] = sym;
	return true;
}


symbol* 
symbol_table::get_sym(string n) 
{ 
	if( t.count(n)>0 ) 
		return t[n]; 
	cerr << "error: Symbol table does not contain symbol '"<< n << "'.\n";
	assert(0);
}

ast_vertex* 
symbol_table::get_decl(string n) 
{
	if( t.count(n)>0 ) 
		return t[n]->decl; 
	cerr << "error: Symbol table does not contain symbol '"<< n << "'.\n";
	assert(0);
}

bool 
symbol_table::is_shared(string n) 
{ 
	if( t.count(n)>0 ) 
		return ( t[n]->is_shared() ); 
	else {
		cerr << "error: symbol \e[1;31m" << n << "\e[0;0m is not defined but used.\n";
		assert(0);
		return 0; 
	}
}

bool 
symbol_table::is_private(string n) 
{ 
	if( t.count(n)>0 ) 
		return ( t[n]->is_private() ); 
	else {
		cerr << "error: symbol \e[1;31m" << n << "\e[0;0m is not defined but used.\n";
		assert(0);
		return 0; 
	}
}

bool 
symbol_table::is_int(string n) 
{ 
	if( t.count(n)>0 ) 
		return ( t[n]->is_int() ); 
	else {
		cerr << "error: symbol \e[1;31m" << n << "\e[0;0m is not defined but used.\n";
		assert(0);
		return 0; 
	}
}

bool 
symbol_table::is_float(string n) 
{ 
	if( t.count(n)>0 ) 
		return ( t[n]->is_float() ); 
	else {
		cerr << "error: symbol \e[1;31m" << n << "\e[0;0m is not defined but used.\n";
		assert(0);
		return 0; 
	}
}


bool 
symbol_table::is_float_pointer(string n) 
{ 
	if( t.count(n)>0 ) 
		return ( t[n]->is_float_pointer() ); 
	else {
		cerr << "error: symbol \e[1;31m" << n << "\e[0;0m is not defined but used.\n";
		assert(0);
		return 0; 
	}
}


bool 
symbol_table::is_defined_symbol(string n) 
{ 
	if( t.count(n)>0 ) 
		return 1;
	cerr << "error: symbol " << n << " is not defined but used.\n";
	assert(0);
	return 0; 
}


void 
symbol_table::dump() 
{
	cerr << "DUMP of Symbol Table\n--------------------\n";
	if(t.begin()==t.end()) cerr << "symbol table is empty.\n";
	for(map< std::string, symbol* >::const_iterator it=t.begin(); it!=t.end(); it++) {
		string s = (it->second->shared) ? "shared" : "private";
		string t = it->second->decl->vertex_type_2_string();
		cerr << "(" << it->first << ", " << t << ", " << s << ")" << endl;
	}
}


void 
symbol_table::dump2file(const string& filename) 
{
	ofstream ofs( filename.c_str() );
	ofs << "DUMP of Symbol Table\n--------------------\n";
	if(t.begin()==t.end()) ofs << "symbol table is empty.\n";
	for(map< std::string, symbol* >::const_iterator it=t.begin(); it!=t.end(); it++) {
		string s = (it->second->shared) ? "shared" : "private";
		string t = it->second->decl->vertex_type_2_string();
		ofs << "(" << it->first << ", " << t << ", " << s << ")" << endl;
	}
}
