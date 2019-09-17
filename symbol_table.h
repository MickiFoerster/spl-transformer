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

#ifndef _SYMBOL_TABLE_H
#define _SYMBOL_TABLE_H
#include <vector>
#include <map>
#include "ast_vertex.h"

class symbol_table;

class symbol {
private:
	bool is_intermediate;
	bool new_derivative;
	symbol() {}
public:
	bool is_sym_intermediate() { return is_intermediate; }
	symbol(std::string n, ast_vertex* d, bool shared_or_private, bool is_symbol_intermediate=0, bool is_symbol_new_derivative=0);
	inline bool is_shared() const { return shared; }
	inline bool is_private() const { return !shared; }
	inline bool is_int() const { 
		return (decl->get_type()==VT_UINT_DECLARATION || decl->get_type()==VT_INT_DECLARATION || decl->get_type()==VT_INT_POINTER_DECLARATION); 
	}
	inline bool is_float() const { return (decl->get_type()==VT_FLOAT_DECLARATION || decl->get_type()==VT_FLOAT_POINTER_DECLARATION); }
	inline bool is_float_pointer() const { return decl->get_type()==VT_FLOAT_POINTER_DECLARATION; }
	string name;
	ast_vertex* decl;
	bool shared;
	string basename;
	int order_of_identifier;
	vector< derivative_model > derivative_type_of_identifier;
	friend class symbol_table;
};

class symbol_table {
private:
	map< std::string, symbol* > t;
public:
	symbol_table(){}

	void clear_intermediates() ;
	bool insert(ast_vertex* d, 
					   bool shared_or_private,  // shared=true, private=false
					   bool is_symbol_intermediate=0, 
					   bool is_symbol_new_derivative=0) ;
	symbol* get_sym(string n) ;
	ast_vertex* get_decl(string n) ;
	bool is_int(string n) ;
	bool is_shared(string n) ;
	bool is_private(string n) ;
	bool is_float(string n) ;
	bool is_float_pointer(string n) ;
	bool is_defined_symbol(string n) ;
	void dump() ;
	void dump2file(const string& filename) ;
};

extern symbol_table sym_tbl;
#endif
