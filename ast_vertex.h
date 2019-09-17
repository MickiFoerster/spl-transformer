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

#ifndef AST_VERTEX_H
#define AST_VERTEX_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <list>
#include <set>
#include <map>
#include <cassert>
#include <cstdlib>

using namespace std;

#include "unparser.h"

extern const int TAB_WIDTH;
extern string tl_prefix;
extern string adj_prefix;
extern int order_of_origin_code;

enum derivative_model { tangent_linear, adjoint };


#define YYSTYPE ast_vertex* 
#define DUMP_TREE(v)     { assert(v); ofstream ofs("dump.dot"); ofs<<"digraph {\n"; (v)->dump(ofs); ofs<<"}\n"; ofs.close(); system("dot -Tpdf dump.dot > dump.pdf"); }
#define DUMP_TREE_EXIT(v)     { assert(v); ofstream ofs("dump.dot"); ofs<<"digraph {\n"; (v)->dump(ofs); ofs<<"}\n"; ofs.close(); system("dot -Tpdf dump.dot > dump.pdf"); assert(0); }
#define DUMP_TREE_AND_HOLD(v)     \
		{  \
			assert(v);  \
			ostringstream oss; \
			static int dump_tree_counter=1; \
			oss<< "dump" << dump_tree_counter++; \
			string fn=oss.str(); \
			ofstream ofs( string(fn+".dot").c_str() ); \
			ofs<<"digraph {\n"; \
			(v)->dump(ofs); \
			ofs<<"}\n"; \
			ofs.close(); \
			oss.str(""); oss << "dot -Tpdf " << string((fn+".dot"))  << "  > " << string((fn+".pdf")) ; \
			cerr << (oss.str().c_str()) << endl; \
			system(oss.str().c_str()); \
			oss.str(""); oss << "evince " << string((fn+".pdf")) << "  &  " ; \
			cerr << "break and dump tree in "<< __FILE__ <<":"<<__LINE__ << endl; \
			cerr << (oss.str().c_str()) << endl; \
			system(oss.str().c_str()); \
			cerr << "Press <RETURN>\n" ;char hold; cin >> hold;\
		}
#define UNKNOWN_TYPE(ASTV)		cerr << "error:"<< ASTV->get_line_number() << ": The vertex type " << ASTV->vertex_type_2_string() << " is not handled here.\n"; assert(0);
#define ASSUME_TYPE(v, t)		if((v)->get_type()!=t) cerr << "error: The vertex type " << (v)->vertex_type_2_string() << " is not the one we expected here.\n"; assert((v)->get_type()==t);
#define ASSUME_TYPES(v, type1, type2)		if((v)->get_type()!=type1 && (v)->get_type()!=type2) cerr << "error: The vertex type " << (v)->vertex_type_2_string() << " is not the one we expected here.\n"; assert( (v)->get_type()==type1 || (v)->get_type()==type2 );
#define   ASSUME_TYPE_AND_CLONE_ROOT_AND_APPLY_TRANS_TO_FIRST(VT_TYPE, transformation)        \
			ast_vertex* v=this; \
			ast_vertex* S=NULL; \
			ASSUME_TYPE(this, VT_TYPE); \
			S=createS(); \
			ast_vertex* result=clone_node(); \
			ast_vertex* result_trans=FIRST->transformation(); ASSUME_TYPE(result_trans, VT_S); \
			add_child(result, result_trans);  \
			add_child(S, result); \
			ASSUME_TYPE(S, VT_S); \
			return S

#define FOR_EACH_it_IN(list_of_ast_vertex_pointer)     for(list<ast_vertex*>::const_iterator it=list_of_ast_vertex_pointer.begin(); it!=list_of_ast_vertex_pointer.end(); it++) 


enum vertex_type {
	VT_SPL_CODE,
	VT_GLOBAL_DECLARATIONS,
	VT_PARALLEL_REGION,
	VT_PARALLEL_FOR_REGION,
	VT_FOR_LOOP_HEADER,
	VT_FOR_LOOP_HEADER_INIT,
	VT_FOR_LOOP_HEADER_VAR_DEF,
	VT_FOR_LOOP_HEADER_TEST,
	VT_FOR_LOOP_HEADER_UPDATE,
	VT_SEQ_DECLARATIONS,
	VT_INT_DECLARATION,
	VT_UINT_DECLARATION,
	VT_INT_POINTER_DECLARATION,
	VT_FLOAT_DECLARATION,
	VT_FLOAT_POINTER_DECLARATION,
	VT_STACKc_DECLARATION,
	VT_STACKi_DECLARATION,
	VT_STACKf_DECLARATION,

	VT_S,

	VT_SPL_STACKi,
	VT_SPL_STACKf,
	VT_SPL_STACKc,
	VT_SPL_STACKfpush,
	VT_SPL_STACKipush,
	VT_SPL_STACKcpush,
	VT_SPL_STACKfpop,
	VT_SPL_STACKipop,
	VT_SPL_STACKcpop,
	VT_SPL_STACKcempty,
	VT_SPL_int_assign,
	VT_SPL_float_assign,
	VT_SPL_float_plus_assign,
	VT_SPL_cond_if,
	VT_SPL_cond_while,

	VT_SPL_identifier,
	VT_SPL_array_index,
	VT_SPL_expr_in_brackets,
	VT_SPL_expr_negative,

	VT_SPL_float_add,
	VT_SPL_float_sub,
	VT_SPL_float_mul,
	VT_SPL_float_div,
	VT_SPL_float_sin,
	VT_SPL_float_cos,
	VT_SPL_float_exp,
	VT_SPL_float_const,
	VT_SPL_STACKftop,

	VT_SPL_int_add,
	VT_SPL_int_sub,
	VT_SPL_int_mul,
	VT_SPL_int_div,
	VT_SPL_int_mudolo,
	VT_SPL_int_const,
	VT_SPL_STACKitop,
	VT_SPL_STACKctop,
	VT_SPL_int_omp_runtime_func,

	VT_SPL_boolean_or,
	VT_SPL_boolean_and,
	VT_SPL_eq,
	VT_SPL_neq,
	VT_SPL_leq,
	VT_SPL_geq,
	VT_SPL_lower,
	VT_SPL_greater,
	VT_SPL_not,

	VT_OMP_PRAGMA,
	VT_OMP_PARALLEL,
	VT_ACC_PRAGMA,
	VT_ACC_PARALLEL,
	VT_OMP_RUNTIME_ROUTINE,
	VT_PARALLEL,
	VT_CLAUSES,
	VT_PRIVATE,
	VT_FIRSTPRIVATE,
	VT_LASTPRIVATE,
	VT_REDUCTION,
	VT_SINGLE,
	VT_TASK,
	VT_MASTER,
	VT_CRITICAL,
	VT_BARRIER,
	VT_TASKWAIT,
	VT_ATOMIC,
	VT_EXCLUSIVE_READ_FAILURE,
	VT_READ,
	VT_WRITE,
	VT_UPDATE,
	VT_CAPTURE,
	VT_FLUSH,
	VT_ORDERED,
	VT_THREADPRIVATE,
	VT_NOWAIT,
	VT_OMP_FOR,

	VT_LIST_OF_VARS,
	VT_CFG_ENTRY,
	VT_CFG_EXIT,
	VT_DUMMY,
	VT_ASSERT,
	VT_BEFORE_REVERSE_MODE_HOOK,
};

class ast_vertex;
class AST;

bool operator_equal(const ast_vertex *l, const ast_vertex *r) ;
bool operator_lower( const ast_vertex* const l , const ast_vertex* const r) ;
void add_child(ast_vertex *p, ast_vertex *c);
void insert_child(ast_vertex *parent, list<ast_vertex*>::iterator it, ast_vertex *child);



class ast_vertex 
{
private:
	static int counter;
	static int intermediate_name_counter;
	static const string critical_counter_prefix;
	static const string atomic_flag_prefix;
	static const string atomic_storage_prefix;

	int id;
	enum vertex_type type;
	bool slc;
	bool seq;
	bool statement;
	int line_number;
	string str;
	ast_vertex *predecessor;
	ast_vertex *tl_pendant;
	ast_vertex *adj_forward_pendant;
	ast_vertex *adj_reverse_pendant;
	list<ast_vertex*> children;
	ast_vertex* init;
	set<ast_vertex*> final;
	set< pair<ast_vertex*,ast_vertex*> > flow;
	void (*unparser)(void*);

	ast_vertex();  // private std ctor!

	ast_vertex* tau_seq_decl();
	ast_vertex* tau_decl();
	ast_vertex* tau_seq_of_stmts();
	ast_vertex* tau_conditional();
	ast_vertex* tau_assign();
	ast_vertex* tau_stack();
	ast_vertex* tau_omp_for();
	ast_vertex* tau_dummy();
	ast_vertex* tau_barrier();
	ast_vertex* tau_atomic();
	ast_vertex* tau_master();
	ast_vertex* tau_critical();
	ast_vertex* tau_assert();

	ast_vertex* sigma_seq_decl();
	ast_vertex* sigma_decl();
	ast_vertex* sigma_seq_of_stmts();
	ast_vertex* sigma_conditional();
	ast_vertex* sigma_stack();
	ast_vertex* sigma_assign();
	ast_vertex* sigma_omp_for();
	ast_vertex* sigma_dummy();
	ast_vertex* sigma_barrier();
	ast_vertex* sigma_atomic();
	ast_vertex* sigma_master();
	ast_vertex* sigma_critical();
	ast_vertex* sigma_assert();

	ast_vertex* rho_seq_of_stmts();
	ast_vertex* rho_conditional();
	ast_vertex* rho_stack();
	ast_vertex* rho_assign();
	ast_vertex* rho_omp_for();
	ast_vertex* rho_dummy();
	ast_vertex* rho_barrier();
	ast_vertex* rho_atomic();
	ast_vertex* rho_exclusive_read();
	ast_vertex* rho_master();
	ast_vertex* rho_critical();
	ast_vertex* rho_assert();

	void set_slc();
	void compute_derivative_with_respect_to(ast_vertex* x, list<ast_vertex*>& path_to_indep);
	ast_vertex*  build_tl_rhs(string& n, string& der_n);
	ast_vertex*  build_adj_rhs_forward(string& n, string& der_n);
	ast_vertex*  build_adj_rhs_reverse(string& n, string& der_n);
	void get_intermediate_name(derivative_model m, string& var_name, string& der_var_name) ;
	inline void set_intermediate_name_counter_to_zero() { intermediate_name_counter=0; }
	void create_thread_local_stacks(ast_vertex* global_decl, derivative_model der_model);
	void connect2unparser();
	void partition_seq_into_slc_and_cfstmts(list<ast_vertex*> &seqs);
	ast_vertex* createSTACKcpush(int label, const string &stack_prefix);
	ast_vertex* createSTACKcpush(const string variable_name, const string &stack_prefix);
	ast_vertex* createBranchSTACKcTop_equal_label(int label, const string &stack_prefix, ast_vertex *subseq);
	void collect_labels_from(list<string> &l);
	void depth_worker(uint level, uint &max) ;

public:
	ast_vertex(int ln, enum vertex_type t, string s); 
	ast_vertex(const ast_vertex* const);
	~ast_vertex();
	bool valid();
	bool is_atomic();
	ast_vertex* clone();
	ast_vertex* clone_node();
	void appendS(ast_vertex* S) ;
	void appendD(ast_vertex* S) ;

	void unparse(void* argv) { unparser(argv); } ;
	bool is_short_code() ;
	inline const set< pair<ast_vertex*,ast_vertex*> > & get_flow() const { return flow; };
	inline vertex_type get_type() const { return type; };
	inline const string get_str() const { return str; };
	inline void set_str(string s) { str=s; };
	inline int get_line_number() const { return line_number; };
	inline ast_vertex* get_predecessor() const { return predecessor; };
	inline list<ast_vertex*>& get_children() { return children; };
	inline void remove_last_chilren() { children.pop_back(); };
	inline void clear_children() { children.clear(); };
	inline bool is_slc() const { return slc; };
	inline int get_id() const  { return id; }
	void dump(ofstream &ofs);
	const string vertex_type_2_string() ;
	void src_trafo_tl();
	void src_trafo_adj();
	ast_vertex* tau();
	ast_vertex* sigma();
	ast_vertex* rho();
	const string get_lhs_name();
	void set_CFG_elements();
	inline void set_seq() { seq=true; }
	inline void set_statement() { statement=true; }
	void print_statement_tail(ofstream& ofs) ;
	void well_formed_loop_test();
	uint depth() ;
	void replace(ast_vertex* subst_node) ;
	ast_vertex* get_complement_test();
	void normalize_arithmetic_expression();
	void are_there_only_uints(set<ast_vertex*> &s);

	// program analysis related data
	void (*phi) (void*);
	void *AI;

	friend class AST;
	friend void add_child(ast_vertex *p, ast_vertex *c) ;
	friend void insert_child(ast_vertex *parent, list<ast_vertex*>::iterator it, ast_vertex *child) ;
	friend int yyparse();
	friend bool operator_equal(const ast_vertex *const l, const ast_vertex *const r) ;
};

inline ast_vertex* 
createD() { return new ast_vertex(0, VT_SEQ_DECLARATIONS, ""); }
inline ast_vertex* 
createS() { return new ast_vertex(0, VT_S, ""); }


extern AST ast;
extern ast_vertex* local_seq_decls; 
extern ast_vertex* global_seq_decls; 
#endif
