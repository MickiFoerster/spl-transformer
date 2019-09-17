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

#include "AST.h"
#include "constant-propagation.h"
#include "interval-analysis.h"
#include "exclusive-read.h"


AST::AST(): root(NULL), 
	        tl_root(NULL), 
			adj_root(NULL), 
			code_to_insert_before_reverse_section(NULL)
{  
	cfg_entry = new ast_vertex(0, VT_CFG_ENTRY, "");
	cfg_exit  = new ast_vertex(0, VT_CFG_EXIT, "");
	code_to_insert_before_forward_section = createS();
	code_to_insert_before_reverse_section = createS();

	//prg_analysis.push_back(&constant_propagation::entry_point_for_prg_analysis);
	//prg_analysis.push_back(&interval_analysis::entry_point_for_prg_analysis);
	prg_analysis.push_back(&exclusive_read::entry_point_for_prg_analysis);
}

AST::~AST()
{  // create init file for critical counter variables
	char *cur_wor_dir = getenv("PWD"); assert(cur_wor_dir);
	string fn(string(cur_wor_dir) + "/test_specific_decl.c");
	ifstream ifs(fn.c_str());
	string file_content; 
	while( ifs.good() ) { 
		string line; 
		getline(ifs,line); 
		file_content+=line+"\n"; 
	}
	ifs.close();
	//ofstream ofs( fn.c_str(), ios::app );
	ofstream ofs( fn.c_str() );
	for(map< int , pair<string,int> >::const_iterator it=critical_counter_names.begin();it!=critical_counter_names.end();it++) {
		ostringstream oss;
		oss << "unsigned int " << it->second.first << "="<< it->second.second<< ";\n";
		ofs << oss.str(); 
	}
	for(map< int , string >::const_iterator it=atomic_flag_names.begin();it!=atomic_flag_names.end();it++) {
		ostringstream oss;
		oss << "unsigned int " << it->second << "=0;\n";
		ofs << oss.str(); 
	}
	for(map< int , string >::const_iterator it=atomic_storage_names.begin();it!=atomic_storage_names.end();it++) {
		Unparser unparser;
		ostringstream oss;
		oss << unparser.float_type << " " << it->second << "=0.;\n";
		ofs << oss.str(); 
	}
	delete cfg_entry;
	delete cfg_exit;
}


void 
AST::init(ast_vertex* v){ 
	root=v; 
	root->set_CFG_elements();

	// For program analysis:
	if( option_prg_analysis ) {
		// Post processing steps:
		// 1. connect test expressions from conditional statements or loop statements with their corresponding successors.
		for(list<ast_vertex*>::const_iterator it_cond_stmt=post_processing_test_expressions_to_assertion_mapping.begin();it_cond_stmt!=post_processing_test_expressions_to_assertion_mapping.end();it_cond_stmt++)  {
			switch((*it_cond_stmt)->get_type()) { 
				case VT_SPL_cond_if:
				case VT_SPL_cond_while:
					break;
				default:
					assert(0);
			}
			// Look for these ast_vertices in flow set of SPL_CODE. This should give two matches.
			list< pair<ast_vertex*,ast_vertex*> > l;
			for(set< pair<ast_vertex*,ast_vertex*> >::const_iterator it=root->flow.begin();it!=root->flow.end();it++)  {
				if( it->first==*it_cond_stmt ) {
					l.push_back( *it );
				}
			}
			assert( l.size()==2 );
			ast_vertex* true_branch = SECONDOF(*it_cond_stmt)->init;
			list< pair<ast_vertex*,ast_vertex*> >::const_iterator it=l.begin();
			if( it->second==true_branch ) {
				it++;
				test_expr_to_true_false_successors[*it_cond_stmt] = make_pair(true_branch, it->second);
			}
			else
				test_expr_to_true_false_successors[*it_cond_stmt] = make_pair(true_branch, it->second);
		}
	}
}

void 
AST::dump(ofstream& ofs, ast_vertex* v)
{
	if(v==NULL) return;
	for(std::list<ast_vertex*>::const_iterator it=v->children.begin();it!=v->children.end();it++)
		dump(ofs, *it);
	ofs << v->id << " [label=\""<< v->id << ": line("<<v->line_number <<")\\n" 
           "type(" << v->vertex_type_2_string() << ")\\n"
           "str(" << v->str << ")  "  ;
	if(v->init!=v) ofs << "init(" << v->init->id << ")  "  ;
	ofs << "final(";
	for(set<ast_vertex*>::const_iterator it=v->final.begin();it!=v->final.end();it++) {
		if(it!=v->final.begin())  ofs << ",";
		ofs << (*it)->id ;
	}
	ofs << ")\\n";
    ofs << "\"]" << endl;	
	for(std::list<ast_vertex*>::const_iterator it=v->children.begin();it!=v->children.end();it++) {
		ofs << v->id << "->" << (*it)->id << endl;
	}
}



void 
AST::unparse(string output_filename, enum unparse_modes mode)
{
	Unparser unparser_instance;
	Unparser *unparser=&unparser_instance;
	ast_vertex *v;

	unparser->current_mode=mode;
	unparser->ofs.open(output_filename.c_str());
	if(!unparser->ofs.good()) {
		cerr << "error: Cannot open " << output_filename << " for writing.\n";
	}
	assert(unparser->ofs.good());
	ofstream ofs;
	switch(mode) {
		case unparse_original:
			ofs.open("ast.dot"); assert(ofs.good());
			ofs << "digraph { \n"; ast.dump(ofs); ofs << "}\n";
			ofs.close();
			if( !check_if_dot_file_is_too_large("ast.dot") ) system("dot -Tpdf ast.dot > ast.pdf");
			ast.dump_cfg("cfg.dot");
			if( !check_if_dot_file_is_too_large("cfg.dot") ) system("dot -Tpdf cfg.dot > cfg.pdf");
			sym_tbl.dump2file("symbol_table.txt");

			unparser->v=root;
			root->unparse(unparser);
			break;
		case unparse_tangent_linear:
			assert(valid());
			sym_tbl.clear_intermediates();
			tl_root = root->tau();  
			assert(tl_root);

			sym_tbl.dump2file("t1_symbol_table.txt");
			// dump tangent-linear AST
			ofs.open("ast-tl.dot"); assert(ofs.good());
			ofs << "digraph { \n";
			ast.dump_tl(ofs);
			ofs << "}\n";
			ofs.close();
			if( !check_if_dot_file_is_too_large("ast-tl.dot") ) system("dot -Tpdf ast-tl.dot > ast-tl.pdf"); 

			v=tl_root;
			unparser->v=v;
			v->unparse(unparser);
			break;
		case unparse_adjoint:
			assert(valid());
			sym_tbl.clear_intermediates();
			adj_root=root->sigma();
			assert(adj_root);

			sym_tbl.dump2file("a1_symbol_table.txt");
			// dump adjoint AST
			ofs.open("ast-adj.dot"); assert(ofs.good());
			ofs << "digraph { \n"; ast.dump_adj(ofs); ofs << "}\n";
			ofs.close();
			// dump forward section
			ofs.open("ast-adj-forward-section.dot"); assert(ofs.good());
			ofs << "digraph { \n"; ast.dump_adj_forward_section(ofs); ofs << "}\n";
			ofs.close();
			// dump reverse section
			ofs.open("ast-adj-reverse-section.dot"); assert(ofs.good());
			ofs << "digraph { \n"; ast.dump_adj_reverse_section(ofs); ofs << "}\n";
			ofs.close();
			if( !check_if_dot_file_is_too_large("ast-adj.dot") ) system("dot -Tpdf ast-adj.dot > ast-adj.pdf");
			if( !check_if_dot_file_is_too_large("ast-adj-forward-section.dot") ) system("dot -Tpdf ast-adj-forward-section.dot > ast-adj-forward-section.pdf");
			if( !check_if_dot_file_is_too_large("ast-adj-reverse-section.dot") ) system("dot -Tpdf ast-adj-reverse-section.dot > ast-adj-reverse-section.pdf");

			v=adj_root;
			unparser->v=v;
			v->unparse(unparser);
			break;
		default: cerr << "do not know correct mode: " << mode << endl;assert(0);
	}
	unparser->ofs.close();
}

