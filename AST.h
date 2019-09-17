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

#ifndef _AST_H
#define _AST_H
#include <cassert>
#include "ast_vertex.h"
#include "symbol_table.h"

#define  MAX_FILE_SIZE_FOR_DOT_FILES_IN_BYTES    50*1024

enum AST_type {
	ast_original=1000,
	ast_tangent_linear,
	ast_adjoint
};

class AST {
	private:
		ast_vertex* root;
		ast_vertex* tl_root;
		ast_vertex* adj_root;
		ast_vertex* adj_forward_section_root;
		ast_vertex* adj_reverse_section_root;
		void dump(ofstream& ofs, ast_vertex* v);
		inline bool check_if_dot_file_is_too_large(string filename);
		list< pair<ast_vertex*, ast_vertex*> > list_of_straight_line_codes;
		ast_vertex* code_to_insert_before_forward_section;
		ast_vertex* code_to_insert_before_reverse_section;
		list< void (*) (void) > prg_analysis;
		list<ast_vertex*> post_processing_test_expressions_to_assertion_mapping;

	public:
		ast_vertex* cfg_entry;
		ast_vertex* cfg_exit;
		map< int , pair<string, int> > critical_counter_names;
		map< int , string > atomic_flag_names;
		map< int , string > atomic_storage_names;

		map< ast_vertex*, pair< ast_vertex*, ast_vertex* > > test_expr_to_true_false_successors;
		list< ast_vertex* > list_of_assertions;

		AST();
		~AST();
		void init(ast_vertex* v);
		inline bool valid() const { return root!=NULL; }
		inline void perform_prg_analysis() ;
		inline void dump(ofstream& ofs) {   dump(ofs, root); }
		inline void dump_cfg(std::string filename) ;
		inline void dump_tl(ofstream& ofs) { dump(ofs, SECONDOF(SECONDOF(tl_root))); }
		inline void dump_adj(ofstream& ofs) { dump(ofs, adj_root); }
		inline void dump_adj_forward_section(ofstream& ofs) { dump(ofs, adj_forward_section_root); }
		inline void dump_adj_reverse_section(ofstream& ofs) { dump(ofs, adj_reverse_section_root); }
		void unparse(string output_filename, enum unparse_modes mode); 
		inline ast_vertex* get_root() const { return root; };

	friend class ast_vertex ;
	friend int yyparse();
	friend void entry_point_for_prg_analysis (AST &ast) ;
};

void 
AST::perform_prg_analysis() 
{
	for(list< void (*) (void) >::const_iterator it=prg_analysis.begin(); it!=prg_analysis.end(); it++) 
		(*it)();
}

void 
AST::dump_cfg(std::string filename)
{ 
	Unparser unparser_instance;
	Unparser* unparser=&unparser_instance;
	set< ast_vertex* > already_visited;
	set< pair<ast_vertex*, ast_vertex*> > edges;

	// Print out CFG to dot file.
	unparser_instance.suppress_linefeed=true;
	unparser->ofs.open(filename.c_str()); assert(unparser->ofs.good());
	unparser->ofs << "digraph { \n";
	for(set< pair<ast_vertex*,ast_vertex*> >::const_iterator it=root->flow.begin();it!=root->flow.end();it++)  {
		if( already_visited.count(it->first)==0 ) {
			unparser->v=it->first;
			// edge's source 
			unparser->ofs << unparser->v->id << " [label=\""<< unparser->v->id << ": line("<<unparser->v->line_number <<")\\n" ;
			if(unparser->v->is_short_code()) {
				unparser->v->unparse(unparser);
				//unparser->ofs.seekp( unparser->ofs.tellp()-1l );
				unparser->ofs << "\\n"  ;
			}
			else {
				ast_vertex*  v=unparser->v;
				Unparser* argv=unparser;
				unparser->ofs << "type(" << unparser->v->vertex_type_2_string() << ")\\n";
				switch(unparser->v->get_type()) {
					case VT_SPL_cond_while:
					case VT_SPL_cond_if:
						unparser->ofs << "test(";
						UNPARSE_FIRST;
						unparser->ofs << ")";
						break;
					default:
						break;
				}
			}
			unparser->ofs << "\"]" << endl;	
		}

		if( already_visited.count(it->second)==0 ) {
			unparser->v=it->second;
			// edge's target 
			unparser->ofs << unparser->v->id << " [label=\""<< unparser->v->id << ": line("<<unparser->v->line_number <<")\\n" ;
			if(unparser->v->is_short_code()) {
				unparser->v->unparse(unparser);
				//unparser->ofs.seekp( unparser->ofs.tellp()-1l );
				unparser->ofs << "\\n"  ;
			}
			else {
				ast_vertex*  v=unparser->v;
				Unparser* argv=unparser;
				unparser->ofs << "type(" << unparser->v->vertex_type_2_string() << ")\\n";
				switch(unparser->v->get_type()) {
					case VT_SPL_cond_while:
					case VT_SPL_cond_if:
						unparser->ofs << "test(";
						UNPARSE_FIRST;
						unparser->ofs << ")";
						break;
					default:
						break;
				}
			}
			unparser->ofs << "\"]" << endl;	
		}

		if( edges.count(make_pair(it->first, it->second))==0 ) {
			// edge
			unparser->ofs << it->first->id << " -> " << it->second->id << endl;
			edges.insert( make_pair(it->first, it->second) );
		}

		already_visited.insert(it->first);
		already_visited.insert(it->second);
	}
  	unparser->ofs << "}\n";
	unparser->ofs.close();
}


bool 
AST::check_if_dot_file_is_too_large(string filename)
{
  long fpos;
  FILE* fp = fopen(filename.c_str(), "r");  assert(fp);
  fseek(fp, 0, SEEK_END);
  fpos=ftell(fp);
  fclose(fp);
  return (fpos>MAX_FILE_SIZE_FOR_DOT_FILES_IN_BYTES);
}




#endif
