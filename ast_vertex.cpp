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

#include <cassert>
#include <vector>
#include "ast_vertex.h"
#include "symbol_table.h"
#include "AST.h"

int ast_vertex::counter=0;
int ast_vertex::intermediate_name_counter=0;
ast_vertex* local_seq_decls=NULL; 
ast_vertex* global_seq_decls=NULL; 
const int TAB_WIDTH=4;
string tl_prefix;
string adj_prefix;
const string ast_vertex::critical_counter_prefix = "critical_counter_";
const string ast_vertex::atomic_flag_prefix = "atomic_flag_";
const string ast_vertex::atomic_storage_prefix = "atomic_storage_";

ast_vertex::ast_vertex(int ln, enum vertex_type t, string s): 
	id(counter++), 
	type(t), 
	slc(0),
	seq(0),
	statement(0),
	line_number(ln),
	str(s),
	predecessor(0),
	tl_pendant(0),
	adj_forward_pendant(0),
	adj_reverse_pendant(0),
	init(this),
	phi(NULL),
	AI(NULL)
{
	set_slc();
	connect2unparser();
	valid();
	final.insert(this);
}

ast_vertex::ast_vertex(const ast_vertex* const v):
	id(counter++), 
	type(v->type), 
	slc(v->slc),
	seq(v->seq),
	statement(v->statement),
	line_number(v->line_number),
	str(v->str),
	predecessor(0),
	tl_pendant(0),
	adj_forward_pendant(0),
	adj_reverse_pendant(0),
	init(this),
	phi(NULL),
	AI(NULL)
{
	set_slc();
	connect2unparser();
	valid();
	final.insert(this);
}

ast_vertex::~ast_vertex() { }

const string 
ast_vertex::vertex_type_2_string() 
{
	switch(type) {
		case VT_SPL_CODE: return "SPL_CODE";
		case VT_GLOBAL_DECLARATIONS: return "GLOBAL_DECLARATIONS";
		case VT_PARALLEL_REGION: return "PARALLEL_REGION";
		case VT_PARALLEL_FOR_REGION: return "PARALLEL_FOR_REGION";

		case VT_SPL_STACKi: return "SPL_STACKi";
		case VT_SPL_STACKf: return "SPL_STACKf";
		case VT_SPL_STACKc: return "SPL_STACKc";

		case VT_FOR_LOOP_HEADER: return "FOR_LOOP_HEADER";
		case VT_FOR_LOOP_HEADER_INIT: return "FOR_LOOP_HEADER_INIT";
		case VT_FOR_LOOP_HEADER_VAR_DEF: return "VT_FOR_LOOP_HEADER_VAR_DEF";
		case VT_FOR_LOOP_HEADER_TEST: return "FOR_LOOP_HEADER_TEST";
		case VT_FOR_LOOP_HEADER_UPDATE: return "FOR_LOOP_HEADER_UPDATE";
		case VT_SEQ_DECLARATIONS: return "SEQ_DECLARATIONS";
		case VT_INT_DECLARATION: return "INT_DECLARATION";
		case VT_UINT_DECLARATION: return "UINT_DECLARATION";
		case VT_FLOAT_DECLARATION: return "FLOAT_DECLARATION";
		case VT_INT_POINTER_DECLARATION: return "INT_POINTER_DECLARATION";
		case VT_FLOAT_POINTER_DECLARATION: return "FLOAT_POINTER_DECLARATION";
		case VT_STACKc_DECLARATION: return "STACKc_DECLARATION";
		case VT_STACKi_DECLARATION: return "STACKi_DECLARATION";
		case VT_STACKf_DECLARATION: return "STACKf_DECLARATION";

		case VT_S: return "S";
		case VT_SPL_STACKfpush: return "SPL_STACKfpush";
		case VT_SPL_STACKipush: return "SPL_STACKipush";
		case VT_SPL_STACKcpush: return "SPL_STACKcpush";
		case VT_SPL_STACKcempty: return "SPL_STACKcempty";
		case VT_SPL_STACKfpop: return "SPL_STACKfpop";
		case VT_SPL_STACKipop: return "SPL_STACKipop";
		case VT_SPL_STACKcpop: return "SPL_STACKcpop";
		case VT_SPL_int_assign: return "SPL_int_assign";
		case VT_SPL_float_assign: return "SPL_float_assign";
		case VT_SPL_float_plus_assign: return "SPL_float_plus_assign";
		case VT_SPL_cond_if: return "SPL_cond_if";
		case VT_SPL_cond_while: return "SPL_cond_while";

		case VT_SPL_identifier: return "SPL_identifier";
		case VT_SPL_array_index: return "SPL_array_index";
		case VT_SPL_expr_in_brackets: return "SPL_expr_in_brackets";
		case VT_SPL_expr_negative: return "SPL_expr_negative";

		case VT_SPL_float_add: return "SPL_float_add";
		case VT_SPL_float_sub: return "SPL_float_sub";
		case VT_SPL_float_mul: return "SPL_float_mul";
		case VT_SPL_float_div: return "SPL_float_div";
		case VT_SPL_float_sin: return "SPL_float_sin";
		case VT_SPL_float_cos: return "SPL_float_cos";
		case VT_SPL_float_exp: return "SPL_float_exp";
		case VT_SPL_float_const: return "SPL_float_const";
		case VT_SPL_STACKftop: return "SPL_STACKftop";

		case VT_SPL_int_add: return "SPL_int_add";
		case VT_SPL_int_sub: return "SPL_int_sub";
		case VT_SPL_int_mul: return "SPL_int_mul";
		case VT_SPL_int_div: return "SPL_int_div";
		case VT_SPL_int_mudolo: return "SPL_int_mudolo";
		case VT_SPL_int_const: return "SPL_int_const";
		case VT_SPL_STACKitop: return "SPL_STACKitop";
		case VT_SPL_STACKctop: return "SPL_STACKctop";
		case VT_SPL_int_omp_runtime_func: return "SPL_int_omp_runtime_func";

		case VT_SPL_boolean_or: return "SPL_boolean_or";
		case VT_SPL_boolean_and: return "SPL_boolean_and";
		case VT_SPL_eq: return "SPL_eq";
		case VT_SPL_neq: return "SPL_neq";
		case VT_SPL_leq: return "SPL_leq";
		case VT_SPL_geq: return "SPL_geq";
		case VT_SPL_lower: return "SPL_lower";
		case VT_SPL_greater: return "SPL_greater";
		case VT_SPL_not: return "SPL_not";

		case VT_OMP_PRAGMA: return "OMP_PRAGMA";
		case VT_OMP_PARALLEL: return "OMP_PARALLEL";
		case VT_ACC_PRAGMA: return "ACC_PRAGMA";
		case VT_ACC_PARALLEL: return "ACC_PARALLEL";
		case VT_PARALLEL: return "PARALLEL";
		case VT_CLAUSES: return "CLAUSES";
		case VT_PRIVATE: return "PRIVATE";
		case VT_FIRSTPRIVATE: return "FIRSTPRIVATE";
		case VT_LASTPRIVATE: return "LASTPRIVATE";
		case VT_REDUCTION: return "REDUCTION";
		case VT_SINGLE: return "SINGLE";
		case VT_TASK: return "TASK";
		case VT_MASTER: return "MASTER";
		case VT_CRITICAL: return "CRITICAL";
		case VT_TASKWAIT: return "TASKWAIT";
		case VT_ATOMIC: return "ATOMIC";
		case VT_EXCLUSIVE_READ_FAILURE: return "EXCLUSIVE READ FAILURE";
		case VT_READ: return "READ";
		case VT_WRITE: return "WRITE";
		case VT_UPDATE: return "UPDATE";
		case VT_CAPTURE: return "CAPTURE";
		case VT_FLUSH: return "FLUSH";
		case VT_ORDERED: return "ORDERED";
		case VT_THREADPRIVATE: return "THREADPRIVATE";
		case VT_NOWAIT: return "NOWAIT";
		case VT_OMP_FOR: return "OMP_FOR";
		case VT_LIST_OF_VARS: return "LIST_OF_VARS"; 
		case VT_CFG_ENTRY: return "CFG_ENTRY"; 
		case VT_CFG_EXIT: return "CFG_EXIT"; 
		case VT_OMP_RUNTIME_ROUTINE: return "OMP_RUNTIME_ROUTINE";
		case VT_BARRIER: return "BARRIER"; 
		case VT_ASSERT: return "ASSERT"; 
		case VT_DUMMY: return "DUMMY"; 
		case VT_BEFORE_REVERSE_MODE_HOOK: return "BEFORE_REVERSE_MODE_HOOK"; 
		default: 
			 cerr << "error: unknown type id " << type << endl; 
			 assert(0); 
			 return "";
	}
}

void 
ast_vertex::dump(ofstream &ofs)
{
	ast_vertex *v=this;
	if(v==NULL) return;
	for(std::list<ast_vertex*>::const_iterator it=v->children.begin();it!=v->children.end();it++)
		(*it)->dump(ofs);
	ofs << v->id << " [label=\""<< v->id << ":\\n" 
           "type(" << v->vertex_type_2_string() << ")\\n"
           "str(" << v->str << ")\\n"  
           "line("<<v->line_number <<")"<<"\"]" << endl;	
	for(std::list<ast_vertex*>::const_iterator it=v->children.begin();it!=v->children.end();it++) {
		ofs << v->id << "->" << (*it)->id << endl;
	}
}

bool 
ast_vertex::is_atomic()
{
	cerr << "asdf: " << get_id() << endl;
	switch(type)
	{
	case VT_SPL_CODE: return false;

	//case VT_ATOMIC:
	//case VT_SINGLE:
	//case VT_MASTER:
	case VT_CRITICAL: return true;

	default: break;
	}
	if(predecessor==NULL) return false;
	else return predecessor->is_atomic();
}

void 
ast_vertex::set_slc()
{
	switch(type)
	{
	case VT_S:
		{
			bool is_slc=0;
			if(children.begin()!=children.end()) {
				is_slc=1;
				for(list<ast_vertex*>::const_iterator it=children.begin();it!=children.end();it++)  {
					is_slc &= (*it)->is_slc();
				}
			}
			slc=is_slc;
		}
		break;
	case VT_INT_DECLARATION:
	case VT_UINT_DECLARATION:
	case VT_INT_POINTER_DECLARATION:
	case VT_FLOAT_DECLARATION:
	case VT_FLOAT_POINTER_DECLARATION:
	case VT_SPL_STACKfpush:
	case VT_SPL_STACKipush:
	case VT_SPL_STACKcpush:
	case VT_SPL_STACKfpop:
	case VT_SPL_STACKipop:
	case VT_SPL_STACKcpop:
	case VT_SPL_int_assign:
	case VT_SPL_float_assign:
	case VT_SPL_float_plus_assign:
	case VT_SPL_STACKcempty:
	case VT_STACKc_DECLARATION:
	case VT_STACKi_DECLARATION:
	case VT_STACKf_DECLARATION:
	case VT_ATOMIC:
	case VT_EXCLUSIVE_READ_FAILURE:
	case VT_BARRIER:
	case VT_ASSERT:
	case VT_BEFORE_REVERSE_MODE_HOOK:
		slc=1;
		break;
	case VT_SPL_STACKi:
	case VT_SPL_STACKf:
	case VT_SPL_STACKc:
	case VT_SPL_cond_if:
	case VT_SPL_cond_while:
	case VT_PARALLEL:
	case VT_SINGLE:
	case VT_TASK:
	case VT_MASTER:
	case VT_CRITICAL:
	case VT_OMP_FOR:
	case VT_PARALLEL_REGION:
	case VT_PARALLEL_FOR_REGION:
	case VT_CFG_ENTRY:
	case VT_CFG_EXIT:
	case VT_SEQ_DECLARATIONS:
	case VT_SPL_identifier:
	case VT_SPL_array_index:
	case VT_SPL_expr_in_brackets:
	case VT_SPL_expr_negative:
	case VT_SPL_float_add:
	case VT_SPL_float_sub:
	case VT_SPL_float_mul:
	case VT_SPL_float_div:
	case VT_SPL_float_sin:
	case VT_SPL_float_cos:
	case VT_SPL_float_exp:
	case VT_SPL_float_const:
	case VT_SPL_STACKftop:
	case VT_SPL_int_add:
	case VT_SPL_int_sub:
	case VT_SPL_int_mul:
	case VT_SPL_int_div:
	case VT_SPL_int_mudolo:
	case VT_SPL_int_const:
	case VT_SPL_STACKitop:
	case VT_SPL_STACKctop:
	case VT_SPL_int_omp_runtime_func:
	case VT_SPL_eq:
	case VT_SPL_boolean_or: 
	case VT_SPL_boolean_and: 
	case VT_SPL_neq:
	case VT_SPL_leq:
	case VT_SPL_geq:
	case VT_SPL_lower:
	case VT_SPL_greater:
	case VT_SPL_not:
	case VT_OMP_PRAGMA:
	case VT_OMP_PARALLEL:
	case VT_ACC_PRAGMA:
	case VT_ACC_PARALLEL:
	case VT_OMP_RUNTIME_ROUTINE:
	case VT_CLAUSES:
	case VT_PRIVATE:
	case VT_FIRSTPRIVATE:
	case VT_LASTPRIVATE:
	case VT_REDUCTION:
	case VT_TASKWAIT:
	case VT_READ:
	case VT_WRITE:
	case VT_UPDATE:
	case VT_CAPTURE:
	case VT_FLUSH:
	case VT_ORDERED:
	case VT_THREADPRIVATE:
	case VT_NOWAIT:
	case VT_LIST_OF_VARS:
	case VT_DUMMY:
	case VT_SPL_CODE:
	case VT_GLOBAL_DECLARATIONS:
	case VT_FOR_LOOP_HEADER:
	case VT_FOR_LOOP_HEADER_INIT:
	case VT_FOR_LOOP_HEADER_VAR_DEF:
	case VT_FOR_LOOP_HEADER_TEST:
	case VT_FOR_LOOP_HEADER_UPDATE:
		slc=0;
		break;
	default:
		UNKNOWN_TYPE(this);
		slc=0;
	}
}

void ast_vertex::get_intermediate_name(derivative_model m, string& var_name, string& der_var_name) 
{
	ostringstream oss;
	oss << "v" << intermediate_name_counter++ << "_from" << order_of_origin_code;
	var_name = oss.str();
	switch(m) {
		case tangent_linear:
			der_var_name = tl_prefix + var_name;
			break;
		case adjoint:
			der_var_name = adj_prefix + var_name;
			break;
		default:
			assert(0);
	}
	ast_vertex* decl = new ast_vertex(0, VT_FLOAT_DECLARATION, "");
	ast_vertex* identifier = new ast_vertex(0, VT_SPL_identifier, var_name);
	add_child(decl, identifier);
	// Derivative  variable t1_v or a1_v
	ast_vertex* der_decl = new ast_vertex(0, VT_FLOAT_DECLARATION, "");
	ast_vertex* der_identifier = new ast_vertex(0, VT_SPL_identifier, der_var_name);
	add_child(der_decl, der_identifier);
	bool var_is_private=false;
	bool var_is_intermediate=true;
	if( sym_tbl.insert(decl, var_is_private, var_is_intermediate) )  // if there is not already an symbol in sym_tbl, insert intermediate variable.
		add_child(local_seq_decls, decl);
	else {
		delete decl;
		delete identifier;
	}
	if( sym_tbl.insert(der_decl, var_is_private, var_is_intermediate) )   // Insert intermediate variable if there is not already a symbol with that name.
		add_child(local_seq_decls, der_decl);
	else {
		delete der_decl;
		delete der_identifier;
	}
}

void 
ast_vertex::set_CFG_elements() 
{
	ast_vertex* v=this;
	switch(this->type) {
		case VT_SPL_CODE: {
			ast_vertex *D=FIRSTOF(this);
			ast_vertex *P=SECONDOF(this);
			ASSUME_TYPE( D, VT_GLOBAL_DECLARATIONS );
			switch(P->get_type()) {
				case VT_PARALLEL_REGION:
				case VT_PARALLEL_FOR_REGION:
					break;
				default:
					assert(0);
			}
			//ASSUME_TYPE( P, VT_PARALLEL_REGION );

			// final set:
			final.clear();
			for(set<ast_vertex*>::const_iterator it=P->final.begin(); it!=P->final.end(); it++) 
				final.insert(*it);


			// CFG_ENTRY -> first declartion -> ... -> last declaration -> PARALLEL_REGION
			// flow set:
			// flow(D)
			for(set< pair<ast_vertex*,ast_vertex*> >::const_iterator setit=D->flow.begin();setit!=D->flow.end();setit++) 
				flow.insert( *setit );
			// flow(P)
			for(set< pair<ast_vertex*,ast_vertex*> >::const_iterator setit=P->flow.begin();setit!=P->flow.end();setit++) 
				flow.insert( *setit );
			if( D->children.size()>0 ) {
				// init  
				init = FIRSTOF(D);
				// flow( D->P )
				for(set<ast_vertex*>::const_iterator it=D->final.begin(); it!=D->final.end(); it++) 
					flow.insert( make_pair(*it, P->init) );
				// flow( ENTRY -> first declaration of D )
				flow.insert( make_pair(ast.cfg_entry, FIRSTOF(D)) );
			}
			else {
				// init  
				init = FIRSTOF(P);
				// flow( ENTRY -> P )
				flow.insert( make_pair(ast.cfg_entry, P) );
			}
			break;
		}
		case VT_GLOBAL_DECLARATIONS: {
			// The data flow inside the sequence is already set up due handling 
			// of the case of VT_SEQ_DECLARATIONS. 
			assert(0); // Do not do things twice!
			break;
		}
		case VT_PARALLEL_REGION :   {
			ast_vertex* v=this;
			ast_vertex* D=FIRST;
			ast_vertex* S=SECOND;
			ASSUME_TYPE( D, VT_SEQ_DECLARATIONS );
			ASSUME_TYPE( S, VT_S );
			// final set:
			final.clear();
			for(set<ast_vertex*>::const_iterator it=S->final.begin(); it!=S->final.end(); it++) 
				final.insert(*it);

			// flow set:
			// flow(D)
			for(set< pair<ast_vertex*,ast_vertex*> >::const_iterator setit=D->flow.begin();setit!=D->flow.end();setit++) 
				flow.insert( *setit );
			// flow(S)
			for(set< pair<ast_vertex*,ast_vertex*> >::const_iterator setit=S->flow.begin();setit!=S->flow.end();setit++) 
				flow.insert( *setit );
			if( D->children.size()>0 ) {
				// flow( D->S )
				for(set<ast_vertex*>::const_iterator it=D->final.begin(); it!=D->final.end(); it++) 
					flow.insert( make_pair(*it, S->init) );
				// flow( #parallel->D )
				flow.insert( make_pair(this, D->init) );
			}
			else {
				// flow( #parallel->S )
				flow.insert( make_pair(this, S->init) );
			}

			// flow( final(#parallel)-> EXIT )
			for(set<ast_vertex*>::const_iterator it=final.begin(); it!=final.end(); it++) 
				flow.insert( make_pair(*it, ast.cfg_exit) );
			break;
		}
		case VT_PARALLEL_FOR_REGION : {
			ast_vertex* v=this;
			ast_vertex* loop_header=FIRST;
			ast_vertex* D=SECOND;
			ast_vertex* S=THIRD;
			ast_vertex* loop_init=NULL;
			ast_vertex* loop_test=NULL;
			ast_vertex* loop_update=NULL;

			ASSUME_TYPE( loop_header, VT_FOR_LOOP_HEADER );
			ASSUME_TYPE( D, VT_SEQ_DECLARATIONS );
			ASSUME_TYPE( S, VT_S );
			v=loop_header;
			loop_init=FIRST; loop_test=SECOND; loop_update=THIRD;
			ASSUME_TYPE( loop_init, VT_FOR_LOOP_HEADER_INIT );
			ASSUME_TYPE( loop_test, VT_FOR_LOOP_HEADER_TEST );
			ASSUME_TYPE( loop_update, VT_FOR_LOOP_HEADER_UPDATE );
			final.clear();
			final.insert( loop_test );
			// flow set:
			// flow(D)
			for(set< pair<ast_vertex*,ast_vertex*> >::const_iterator setit=D->flow.begin();setit!=D->flow.end();setit++) 
				flow.insert( *setit );
			// flow(S)
			for(set< pair<ast_vertex*,ast_vertex*> >::const_iterator setit=S->flow.begin();setit!=S->flow.end();setit++) 
				flow.insert( *setit );
			// flow( #parallelFor -> loop_init )
			flow.insert( make_pair(this, loop_init) );
			// flow( loop_init -> loop_test )
			flow.insert( make_pair(loop_init, loop_test) );

			if( D->children.size()>0 ) {
				// flow( D->S )
				for(set<ast_vertex*>::const_iterator it=D->final.begin(); it!=D->final.end(); it++) 
					flow.insert( make_pair(*it, S->init) );
				// flow( loop_test -> init(D) )
				flow.insert( make_pair(loop_test, D->init) );
			}
			else {
				// flow( loop_test -> init(S) )
				flow.insert( make_pair(loop_test, S->init) );
			}

			// flow( final(S) -> loop_update )
			for(set<ast_vertex*>::const_iterator it=S->final.begin(); it!=S->final.end(); it++) 
				flow.insert( make_pair(*it, loop_update) );
			// flow( loop_update -> loop_test )
			flow.insert( make_pair(loop_update, loop_test) );
			// flow( loop_test -> EXIT )
			flow.insert( make_pair(loop_test, ast.cfg_exit) );
			break;
		}
		case VT_SPL_cond_if: {
			ast_vertex* v=this;
			ast_vertex* S=SECOND;
			ASSUME_TYPE( S, VT_S );
			// final set:
			final.clear();
			final.insert(this);

			// Statements with an associated subsequence do have the final set of 
			// their subsequence.
			for(set<ast_vertex*>::const_iterator it=S->final.begin(); it!=S->final.end(); it++) 
				final.insert(*it);

			// flow set:
			for(set< pair<ast_vertex*,ast_vertex*> >::const_iterator setit=S->flow.begin();setit!=S->flow.end();setit++) 
				flow.insert( *setit );
			flow.insert( make_pair(this, S->init) );
			break;
		}
		case VT_SPL_cond_while: { // flow( while(b)^l {S'} ) = flow(S') cup { (l,init(S')) } cup { (l',l) | l' in final(S') }
			ast_vertex* v=this;
			ast_vertex* S=SECOND;
			ASSUME_TYPE( S, VT_S );
			assert( flow.size()==0 );
			for(set< pair<ast_vertex*,ast_vertex*> >::const_iterator setit=S->flow.begin();setit!=S->flow.end();setit++) 
				flow.insert( *setit );
			flow.insert( make_pair(this, S->init) );
			for(set<ast_vertex*>::const_iterator finalit=S->final.begin();finalit!=S->final.end();finalit++) 
				flow.insert( make_pair(*finalit, this) );
			break;
		}
		case VT_OMP_FOR: { 
			ast_vertex* v=this;
			ast_vertex* loop_init=FIRST;
			ast_vertex* loop=SECOND;
			ASSUME_TYPE( loop_init, VT_SPL_int_assign );
			ASSUME_TYPE( loop, VT_SPL_cond_while );

			final.clear();
			for(set<ast_vertex*>::const_iterator it=loop->final.begin(); it!=loop->final.end(); it++) 
				final.insert(*it);

			assert( flow.size()==0 );
			// flow from init statement to associated loop 
			for(set<ast_vertex*>::const_iterator finalit=loop_init->final.begin();finalit!=loop_init->final.end();finalit++) 
				flow.insert( make_pair(*finalit, loop->init) );
			// flow from associated loop 
			for(set< pair<ast_vertex*,ast_vertex*> >::const_iterator setit=loop->flow.begin();setit!=loop->flow.end();setit++) 
				flow.insert( *setit );
			// flow from #for to init statement
			flow.insert( make_pair(this, loop_init->init) );
			break;
		}
		case VT_SEQ_DECLARATIONS: 
		case VT_S: 
			init = (*children.begin()); 
			final.clear();
			final = children.back()->final;

			// If there are more than one children we connect the second last children with the last children.	
			// And fill the flow from the last children to the current sequence.
			switch( children.size() ) {
				case 0:
					assert(0);
					break;
				case 1:
					for(set< pair<ast_vertex*,ast_vertex*> >::const_iterator setit=FIRST->flow.begin();setit!=FIRST->flow.end();setit++) 
						flow.insert( *setit );
					break;
				default: {
					ast_vertex* lastch;
					ast_vertex* seclastch;
					list<ast_vertex*>::const_iterator it=v->children.end(); it--;
					lastch=*it; it--;
					seclastch=*it;

					// Insert flow from last children. All the previous flow is already in set.
					for(set< pair<ast_vertex*,ast_vertex*> >::const_iterator setit=lastch->flow.begin();setit!=lastch->flow.end();setit++) 
						flow.insert( *setit );
					for(set<ast_vertex*>::const_iterator finalit=seclastch->final.begin();finalit!=seclastch->final.end();finalit++) 
						flow.insert( make_pair(*finalit, lastch->init) );
					break;
				}
			}
			break;
		case VT_ATOMIC: 
			{
				ast_vertex* v=this;
				ast_vertex* incr_assign=FIRST;
				ASSUME_TYPE( incr_assign, VT_SPL_float_plus_assign );
				init=FIRST;
				final.clear();
				final.insert(FIRST);
				assert( flow.size()==0 );
				flow.insert( make_pair(this, incr_assign->init) );
				break;
			}
		case VT_MASTER: 
		case VT_CRITICAL: 
			{
				ast_vertex* v=this;
				ast_vertex* S=FIRST;
				ASSUME_TYPE( S, VT_S );
				init=this;
				final.clear();
				// Statements with an associated subsequence do have the final set of their subsequence.
				for(set<ast_vertex*>::const_iterator it=S->final.begin(); it!=S->final.end(); it++) 
					final.insert(*it);

				// flow set:
				assert( flow.size()==0 );
				for(set< pair<ast_vertex*,ast_vertex*> >::const_iterator setit=S->flow.begin();setit!=S->flow.end();setit++) 
					flow.insert( *setit );
				flow.insert( make_pair(this, S->init) );
				break;
			}
		default:
			break;
	}
}

bool
ast_vertex::valid() 
{
	switch(this->type) {
		case VT_SPL_CODE: assert(this->unparser==&unparser_spl_code); break;
		case VT_GLOBAL_DECLARATIONS: assert(this->unparser==&unparser_global_declarations); break;
		case VT_FOR_LOOP_HEADER: assert(this->unparser==&unparser_FOR_LOOP_HEADER); break;
		case VT_FOR_LOOP_HEADER_INIT: assert(this->unparser==&unparser_FOR_LOOP_HEADER_INIT); break;
		case VT_FOR_LOOP_HEADER_VAR_DEF: assert(this->unparser==&unparser_FOR_LOOP_HEADER_VAR_DEF); break;
		case VT_FOR_LOOP_HEADER_TEST: assert(this->unparser==&unparser_FOR_LOOP_HEADER_TEST); break;
		case VT_FOR_LOOP_HEADER_UPDATE: assert(this->unparser==&unparser_FOR_LOOP_HEADER_UPDATE); break;
		case VT_SEQ_DECLARATIONS: assert(this->unparser==&unparser_SEQ_DECLARATIONS); break;
		case VT_INT_DECLARATION: assert(this->unparser==&unparser_INT_DECLARATION); break;
		case VT_UINT_DECLARATION: assert(this->unparser==&unparser_UINT_DECLARATION); break;
		case VT_FLOAT_DECLARATION: assert(this->unparser==&unparser_FLOAT_DECLARATION); break;
		case VT_INT_POINTER_DECLARATION: assert(this->unparser==&unparser_INT_POINTER_DECLARATION); break;
		case VT_FLOAT_POINTER_DECLARATION: assert(this->unparser==&unparser_FLOAT_POINTER_DECLARATION); break;
		case VT_STACKc_DECLARATION: assert(this->unparser==&unparser_STACKc_DECLARATION); break;
		case VT_STACKi_DECLARATION: assert(this->unparser==&unparser_STACKi_DECLARATION); break;
		case VT_STACKf_DECLARATION: assert(this->unparser==&unparser_STACKf_DECLARATION); break;

		case VT_SPL_STACKi: assert(this->unparser==&unparser_spl_STACKi); break;
		case VT_SPL_STACKf: assert(this->unparser==&unparser_spl_STACKf); break;
		case VT_SPL_STACKc: assert(this->unparser==&unparser_spl_STACKc); break;
		case VT_SPL_STACKfpush: assert(this->unparser==&unparser_spl_STACKfpush); break;
		case VT_SPL_STACKipush: assert(this->unparser==&unparser_spl_STACKipush); break;
		case VT_SPL_STACKcpush: assert(this->unparser==&unparser_spl_STACKcpush); break;
		case VT_SPL_STACKcempty: assert(this->unparser==&unparser_spl_STACKcempty); break;
		case VT_SPL_STACKfpop: assert(this->unparser==&unparser_spl_STACKfpop); break;
		case VT_SPL_STACKipop: assert(this->unparser==&unparser_spl_STACKipop); break;
		case VT_SPL_STACKcpop: assert(this->unparser==&unparser_spl_STACKcpop); break;

		case VT_SPL_identifier: assert(this->unparser==&unparser_spl_identifier); break;
		case VT_SPL_array_index: assert(this->unparser==&unparser_spl_array_index); break;
		case VT_SPL_expr_in_brackets: assert(this->unparser==&unparser_spl_expr_in_brackets); break;
		case VT_SPL_expr_negative: assert(this->unparser==&unparser_spl_expr_negative); break;

		case VT_SPL_float_add: assert(this->unparser==&unparser_spl_float_add); break;
		case VT_SPL_float_sub: assert(this->unparser==&unparser_spl_float_sub); break;
		case VT_SPL_float_mul: assert(this->unparser==&unparser_spl_float_mul); break;
		case VT_SPL_float_div: assert(this->unparser==&unparser_spl_float_div); break;
		case VT_SPL_float_sin: assert(this->unparser==&unparser_spl_float_sin); break;
		case VT_SPL_float_cos: assert(this->unparser==&unparser_spl_float_cos); break;
		case VT_SPL_float_exp: assert(this->unparser==&unparser_spl_float_exp); break;
		case VT_SPL_float_const: assert(this->unparser==&unparser_spl_float_const); break;
		case VT_SPL_STACKftop: assert(this->unparser==&unparser_spl_STACKftop); break;

		case VT_SPL_int_add: assert(this->unparser==&unparser_spl_int_add); break;
		case VT_SPL_int_sub: assert(this->unparser==&unparser_spl_int_sub); break;
		case VT_SPL_int_mul: assert(this->unparser==&unparser_spl_int_mul); break;
		case VT_SPL_int_div: assert(this->unparser==&unparser_spl_int_div); break;
		case VT_SPL_int_mudolo: assert(this->unparser==&unparser_spl_int_mudolo); break;
		case VT_SPL_int_const: assert(this->unparser==&unparser_spl_int_const); break;
		case VT_SPL_STACKitop: assert(this->unparser==&unparser_spl_STACKitop); break;
		case VT_SPL_STACKctop: assert(this->unparser==&unparser_spl_STACKctop); break;

		case VT_SPL_boolean_or: assert(this->unparser==&unparser_spl_boolean_or); break;
		case VT_SPL_boolean_and: assert(this->unparser==&unparser_spl_boolean_and); break;
		case VT_SPL_eq: assert(this->unparser==&unparser_spl_eq); break;
		case VT_SPL_neq: assert(this->unparser==&unparser_spl_neq); break;
		case VT_SPL_leq: assert(this->unparser==&unparser_spl_leq); break;
		case VT_SPL_geq: assert(this->unparser==&unparser_spl_geq); break;
		case VT_SPL_lower: assert(this->unparser==&unparser_spl_lower); break;
		case VT_SPL_greater: assert(this->unparser==&unparser_spl_greater); break;
		case VT_SPL_not: assert(this->unparser==&unparser_spl_not); break;


		case VT_CLAUSES: assert(this->unparser==&unparser_clauses); break;
		case VT_ATOMIC: assert(this->unparser==&unparser_omp_ATOMIC); break;
		case VT_EXCLUSIVE_READ_FAILURE: assert(this->unparser==&unparser_ad_exclusive_read_failure); break;
		case VT_PARALLEL_REGION : assert(this->unparser==&unparser_PARALLEL_REGION); break;
		case VT_PARALLEL_FOR_REGION : assert(this->unparser==&unparser_PARALLEL_FOR_REGION); break;
		case VT_SPL_cond_if: assert(this->unparser==&unparser_spl_cond_if); break;
		case VT_SPL_cond_while: assert(this->unparser==&unparser_spl_cond_while); break;
		case VT_SPL_int_assign: assert(this->unparser==&unparser_spl_int_assign); break;
		case VT_S: assert(this->unparser==&unparser_S); break;
		case VT_SPL_float_assign: assert(this->unparser==&unparser_spl_float_assign); break;
		case VT_SPL_float_plus_assign: assert(this->unparser==&unparser_spl_float_plus_assign); break;
		case VT_THREADPRIVATE: assert(this->unparser==&unparser_spl_omp_threadprivate); break;
		case VT_OMP_FOR: assert(this->unparser==&unparser_spl_omp_for); break;
		case VT_LIST_OF_VARS: assert(this->unparser==&unparser_list_of_vars); break;
		case VT_CFG_ENTRY: assert(this->unparser==&unparser_cfg_entry); break; 
		case VT_CFG_EXIT: assert(this->unparser==&unparser_cfg_exit); break; 
		case VT_OMP_RUNTIME_ROUTINE: assert(this->unparser==&unparser_OMP_RUNTIME_ROUTINE); break; 
		case VT_BARRIER: assert(this->unparser==&unparser_barrier); break;
		case VT_ASSERT: assert(this->unparser==&unparser_assert); break;
		case VT_BEFORE_REVERSE_MODE_HOOK: assert(this->unparser==&unparser_before_reverse_mode_hook); break;
		case VT_DUMMY: assert(this->unparser==&unparser_dummy); break;
		case VT_MASTER: assert(this->unparser==&unparser_master); break;
		case VT_CRITICAL: assert(this->unparser==&unparser_critical); break;

		case VT_OMP_PRAGMA:
		case VT_OMP_PARALLEL:
		case VT_ACC_PRAGMA:
		case VT_ACC_PARALLEL:
		case VT_PARALLEL:
		case VT_PRIVATE:
		case VT_FIRSTPRIVATE:
		case VT_LASTPRIVATE:
		case VT_REDUCTION:
		case VT_SINGLE:
		case VT_TASK:
		case VT_TASKWAIT:
		case VT_READ:
		case VT_WRITE:
		case VT_UPDATE:
		case VT_CAPTURE:
		case VT_FLUSH:
		case VT_ORDERED:
		case VT_NOWAIT: assert(0); // unknown vertex
		default: 
			return false;
	}
	return true;
}

void 
ast_vertex::connect2unparser() 
{
	switch(this->type) {
		case VT_SPL_CODE: this->unparser=&unparser_spl_code; break;
		case VT_GLOBAL_DECLARATIONS: this->unparser=&unparser_global_declarations; break;
		case VT_SPL_STACKi: this->unparser=&unparser_spl_STACKi; break;
		case VT_SPL_STACKf: this->unparser=&unparser_spl_STACKf; break;
		case VT_SPL_STACKc: this->unparser=&unparser_spl_STACKc; break;
		case VT_FOR_LOOP_HEADER: this->unparser=&unparser_FOR_LOOP_HEADER; break;
		case VT_FOR_LOOP_HEADER_INIT: this->unparser=&unparser_FOR_LOOP_HEADER_INIT; break;
		case VT_FOR_LOOP_HEADER_VAR_DEF: this->unparser=&unparser_FOR_LOOP_HEADER_VAR_DEF; break;
		case VT_FOR_LOOP_HEADER_TEST: this->unparser=&unparser_FOR_LOOP_HEADER_TEST; break;
		case VT_FOR_LOOP_HEADER_UPDATE: this->unparser=&unparser_FOR_LOOP_HEADER_UPDATE; break;
		case VT_SEQ_DECLARATIONS: this->unparser=&unparser_SEQ_DECLARATIONS; break;
		case VT_INT_DECLARATION: this->unparser=&unparser_INT_DECLARATION; break;
		case VT_UINT_DECLARATION: this->unparser=&unparser_UINT_DECLARATION; break;
		case VT_FLOAT_DECLARATION: this->unparser=&unparser_FLOAT_DECLARATION; break;
		case VT_INT_POINTER_DECLARATION: this->unparser=&unparser_INT_POINTER_DECLARATION; break;
		case VT_FLOAT_POINTER_DECLARATION: this->unparser=&unparser_FLOAT_POINTER_DECLARATION; break;
		case VT_STACKc_DECLARATION: this->unparser=&unparser_STACKc_DECLARATION; break;
		case VT_STACKi_DECLARATION: this->unparser=&unparser_STACKi_DECLARATION; break;
		case VT_STACKf_DECLARATION: this->unparser=&unparser_STACKf_DECLARATION; break;

		case VT_SPL_STACKfpush: this->unparser=&unparser_spl_STACKfpush; break;
		case VT_SPL_STACKipush: this->unparser=&unparser_spl_STACKipush; break;
		case VT_SPL_STACKcpush: this->unparser=&unparser_spl_STACKcpush; break;
		case VT_SPL_STACKfpop: this->unparser=&unparser_spl_STACKfpop; break;
		case VT_SPL_STACKipop: this->unparser=&unparser_spl_STACKipop; break;
		case VT_SPL_STACKcpop: this->unparser=&unparser_spl_STACKcpop; break;
		case VT_SPL_STACKcempty: this->unparser=&unparser_spl_STACKcempty; break;

		case VT_SPL_identifier: this->unparser=&unparser_spl_identifier; break;
		case VT_SPL_array_index: this->unparser=&unparser_spl_array_index; break;
		case VT_SPL_expr_in_brackets: this->unparser=&unparser_spl_expr_in_brackets; break;
		case VT_SPL_expr_negative: this->unparser=&unparser_spl_expr_negative; break;

		case VT_SPL_float_add: this->unparser=&unparser_spl_float_add; break;
		case VT_SPL_float_sub: this->unparser=&unparser_spl_float_sub; break;
		case VT_SPL_float_mul: this->unparser=&unparser_spl_float_mul; break;
		case VT_SPL_float_div: this->unparser=&unparser_spl_float_div; break;
		case VT_SPL_float_sin: this->unparser=&unparser_spl_float_sin; break;
		case VT_SPL_float_cos: this->unparser=&unparser_spl_float_cos; break;
		case VT_SPL_float_exp: this->unparser=&unparser_spl_float_exp; break;
		case VT_SPL_float_const: this->unparser=&unparser_spl_float_const; break;
		case VT_SPL_STACKftop: this->unparser=&unparser_spl_STACKftop; break;

		case VT_SPL_int_add: this->unparser=&unparser_spl_int_add; break;
		case VT_SPL_int_sub: this->unparser=&unparser_spl_int_sub; break;
		case VT_SPL_int_mul: this->unparser=&unparser_spl_int_mul; break;
		case VT_SPL_int_div: this->unparser=&unparser_spl_int_div; break;
		case VT_SPL_int_mudolo: this->unparser=&unparser_spl_int_mudolo; break;
		case VT_SPL_int_const: this->unparser=&unparser_spl_int_const; break;
		case VT_SPL_STACKctop: this->unparser=&unparser_spl_STACKctop; break;
		case VT_SPL_STACKitop: this->unparser=&unparser_spl_STACKitop; break;

		case VT_SPL_boolean_or:  this->unparser=&unparser_spl_boolean_or; break;
		case VT_SPL_boolean_and: this->unparser=&unparser_spl_boolean_and; break;
		case VT_SPL_eq: this->unparser=&unparser_spl_eq; break;
		case VT_SPL_neq: this->unparser=&unparser_spl_neq; break;
		case VT_SPL_leq: this->unparser=&unparser_spl_leq; break;
		case VT_SPL_geq: this->unparser=&unparser_spl_geq; break;
		case VT_SPL_lower: this->unparser=&unparser_spl_lower; break;
		case VT_SPL_greater: this->unparser=&unparser_spl_greater; break;
		case VT_SPL_not: this->unparser=&unparser_spl_not; break;

		case VT_CLAUSES: this->unparser=&unparser_clauses; break;
		case VT_ATOMIC: this->unparser=&unparser_omp_ATOMIC; break;
		case VT_EXCLUSIVE_READ_FAILURE: this->unparser=&unparser_ad_exclusive_read_failure; break;
		case VT_PARALLEL_REGION : this->unparser=&unparser_PARALLEL_REGION; break;
		case VT_PARALLEL_FOR_REGION : this->unparser=&unparser_PARALLEL_FOR_REGION; break;
		case VT_SPL_cond_if: this->unparser=&unparser_spl_cond_if; break;
		case VT_SPL_cond_while: this->unparser=&unparser_spl_cond_while; break;
		case VT_SPL_int_assign: this->unparser=&unparser_spl_int_assign; break;
		case VT_S: this->unparser=&unparser_S; break;
		case VT_SPL_float_assign: this->unparser=&unparser_spl_float_assign; break;
		case VT_SPL_float_plus_assign: this->unparser=&unparser_spl_float_plus_assign; break;
		case VT_THREADPRIVATE: this->unparser=&unparser_spl_omp_threadprivate; break;
		case VT_OMP_FOR: this->unparser=&unparser_spl_omp_for; break;
		case VT_LIST_OF_VARS: this->unparser=&unparser_list_of_vars; break;
		case VT_CFG_ENTRY: this->unparser=&unparser_cfg_entry; break; 
		case VT_CFG_EXIT: this->unparser=&unparser_cfg_exit; break; 
		case VT_OMP_RUNTIME_ROUTINE: this->unparser=&unparser_OMP_RUNTIME_ROUTINE; break; 
		case VT_BARRIER: this->unparser=&unparser_barrier; break;
		case VT_ASSERT: this->unparser=&unparser_assert; break;
		case VT_BEFORE_REVERSE_MODE_HOOK: this->unparser=&unparser_before_reverse_mode_hook; break;
		case VT_DUMMY: this->unparser=&unparser_dummy; break;
		case VT_MASTER: this->unparser=&unparser_master; break;
		case VT_CRITICAL: this->unparser=&unparser_critical; break;

		case VT_OMP_PRAGMA:
		case VT_OMP_PARALLEL:
		case VT_ACC_PRAGMA:
		case VT_ACC_PARALLEL:
		case VT_PARALLEL:
		case VT_PRIVATE:
		case VT_FIRSTPRIVATE:
		case VT_LASTPRIVATE:
		case VT_REDUCTION:
		case VT_SINGLE:
		case VT_TASK:
		case VT_TASKWAIT:
		case VT_READ:
		case VT_WRITE:
		case VT_UPDATE:
		case VT_CAPTURE:
		case VT_FLUSH:
		case VT_ORDERED:
		case VT_NOWAIT: 
		default: 
			UNKNOWN_TYPE(this);  // unknown vertex
	}
}

bool 
ast_vertex::is_short_code() 
{
	switch(this->type) {
		case VT_SPL_STACKi:
		case VT_SPL_STACKf:
		case VT_SPL_STACKc:
		case VT_FOR_LOOP_HEADER:
		case VT_FOR_LOOP_HEADER_INIT:
		case VT_FOR_LOOP_HEADER_VAR_DEF:
		case VT_FOR_LOOP_HEADER_TEST:
		case VT_FOR_LOOP_HEADER_UPDATE:
		case VT_INT_DECLARATION:
		case VT_UINT_DECLARATION:
		case VT_FLOAT_DECLARATION:
		case VT_INT_POINTER_DECLARATION:
		case VT_FLOAT_POINTER_DECLARATION:
		case VT_STACKc_DECLARATION:
		case VT_STACKi_DECLARATION:
		case VT_STACKf_DECLARATION:
		case VT_SPL_STACKfpush:
		case VT_SPL_STACKipush:
		case VT_SPL_STACKcpush:
		case VT_SPL_STACKfpop:
		case VT_SPL_STACKipop:
		case VT_SPL_STACKcpop:
		case VT_SPL_STACKcempty:
		case VT_SPL_identifier:
		case VT_SPL_array_index:
		case VT_SPL_float_add:
		case VT_SPL_float_sub:
		case VT_SPL_float_mul:
		case VT_SPL_float_div:
		case VT_SPL_float_sin:
		case VT_SPL_float_cos:
		case VT_SPL_float_exp:
		case VT_SPL_float_const:
		case VT_SPL_STACKftop:
		case VT_SPL_int_add:
		case VT_SPL_int_sub:
		case VT_SPL_int_mul:
		case VT_SPL_int_div:
		case VT_SPL_int_mudolo:
		case VT_SPL_int_const:
		case VT_SPL_STACKctop:
		case VT_SPL_STACKitop:
		case VT_SPL_int_omp_runtime_func:
		case VT_SPL_eq:
		case VT_SPL_boolean_or: 
		case VT_SPL_boolean_and: 
		case VT_SPL_neq:
		case VT_SPL_leq:
		case VT_SPL_geq:
		case VT_SPL_lower:
		case VT_SPL_greater:
		case VT_SPL_not:
		case VT_SPL_int_assign:
		case VT_SPL_float_assign:
		case VT_SPL_float_plus_assign:
		case VT_OMP_RUNTIME_ROUTINE:
			return true;

		case VT_SPL_CODE:
		case VT_GLOBAL_DECLARATIONS:
		case VT_SEQ_DECLARATIONS:
		case VT_SPL_expr_in_brackets:
		case VT_SPL_expr_negative:
		case VT_CLAUSES:
		case VT_ATOMIC:
		case VT_EXCLUSIVE_READ_FAILURE:
		case VT_PARALLEL_REGION :
		case VT_PARALLEL_FOR_REGION :
		case VT_SPL_cond_if:
		case VT_SPL_cond_while:
		case VT_S:
		case VT_THREADPRIVATE:
		case VT_OMP_FOR:
		case VT_LIST_OF_VARS:
		case VT_CFG_ENTRY: 
		case VT_CFG_EXIT: 
		case VT_OMP_PRAGMA:
		case VT_OMP_PARALLEL:
		case VT_ACC_PRAGMA:
		case VT_ACC_PARALLEL:
		case VT_PARALLEL:
		case VT_PRIVATE:
		case VT_FIRSTPRIVATE:
		case VT_LASTPRIVATE:
		case VT_REDUCTION:
		case VT_SINGLE:
		case VT_TASK:
		case VT_MASTER:
		case VT_CRITICAL:
		case VT_TASKWAIT:
		case VT_READ:
		case VT_WRITE:
		case VT_UPDATE:
		case VT_CAPTURE:
		case VT_FLUSH:
		case VT_ORDERED:
		case VT_NOWAIT: 
		case VT_BARRIER: 
		case VT_ASSERT: 
		case VT_BEFORE_REVERSE_MODE_HOOK: 
		case VT_DUMMY: 
		default: 
			return false; 
	}
}

void 
ast_vertex::create_thread_local_stacks(ast_vertex* global_decl, derivative_model der_model)
{
	// Look in global declaration part for existing STACKc, STACKf, STACKi and the associated threadprivate directive
	int STACKc=0;
	int STACKi=0;
	int STACKf=0;
	bool newTHREADPRIVATE=0;
	ast_vertex* THREADPRIVATE=NULL;
	string prefix;
	switch(der_model) {
		case tangent_linear:
			prefix=tl_prefix;
			break;
		case adjoint:
			prefix=adj_prefix;
			break;
		default: break;
	}
	for(list<ast_vertex*>::const_iterator it=global_decl->children.begin(); it!=global_decl->children.end();it++) {
		switch( (*it)->type ) {
			case VT_STACKc_DECLARATION: STACKc=-1; if( (*it)->str==prefix ) STACKc=1; break;
			case VT_STACKi_DECLARATION: STACKi=-1; if( (*it)->str==prefix ) STACKi=1; break;
			case VT_STACKf_DECLARATION: STACKf=-1; if( (*it)->str==prefix ) STACKf=1; break;
			case VT_THREADPRIVATE: 		THREADPRIVATE=*it; break;
			default: break;
		}
	}
	if( THREADPRIVATE==NULL ) {
		newTHREADPRIVATE=1;
		THREADPRIVATE = new ast_vertex(0, VT_THREADPRIVATE, ""); 
		ast_vertex* list_of_vars = new ast_vertex(0, VT_LIST_OF_VARS, ""); add_child(THREADPRIVATE, list_of_vars);
	}
	// Find iterator position of threadprivate directive to put definitions in front of this directive.
	list<ast_vertex*>::iterator it=global_decl->children.begin();
	while(it!=global_decl->children.end() && (*it)!=THREADPRIVATE )
		it++;
	ast_vertex* v=THREADPRIVATE;
	ast_vertex* list=FIRST;
	// Depending on what derivative model we apply and whether there is a stack in the original code
	// we have to differ between generating a stack or not.

	switch(STACKc) {
		case -1:
			{
				ast_vertex* stack_decl = new ast_vertex(0, VT_STACKc_DECLARATION, prefix);
				global_decl->children.insert(it, stack_decl);
				add_child(list, new ast_vertex(0, VT_SPL_STACKc, prefix) );
			}
			break;
		case  0:
			switch(der_model) {
				case tangent_linear:
					// Original code does not contain any STACKc then do not create one in TL code.
					break;
				case adjoint:
					{
						// Original code does not contain any STACKc then create one in ADJ code.
						ast_vertex* stack_decl = new ast_vertex(0, VT_STACKc_DECLARATION, prefix);
						global_decl->children.insert(it, stack_decl);
						add_child(list, new ast_vertex(0, VT_SPL_STACKc, prefix) );
					}
					break;
				default: break;
			}
			break;
		case  1:
			// In this case the stack for the current derivative order is already defined.
			break;
	}
	switch(STACKi) {
		case -1:
			{
				ast_vertex* stack_decl = new ast_vertex(0, VT_STACKi_DECLARATION, prefix);
				global_decl->children.insert(it, stack_decl);
				add_child(list, new ast_vertex(0, VT_SPL_STACKi, prefix) );
			}
			break;
		case  0:
			switch(der_model) {
				case tangent_linear:
					// Original code does not contain any STACKi then do not create one in TL code.
					break;
				case adjoint:
					{
						// Original code does not contain any STACKi then create one in ADJ code.
						ast_vertex* stack_decl = new ast_vertex(0, VT_STACKi_DECLARATION, prefix);
						global_decl->children.insert(it, stack_decl);
						add_child(list, new ast_vertex(0, VT_SPL_STACKi, prefix) );
					}
					break;
				default: break;
			}
			break;
		case  1:
			// In this case the stack for the current derivative order is already defined.
			break;
	}
	switch(STACKf) {
		case -1:
			{
				ast_vertex* stack_decl = new ast_vertex(0, VT_STACKf_DECLARATION, prefix);
				global_decl->children.insert(it, stack_decl);
				add_child(list, new ast_vertex(0, VT_SPL_STACKf, prefix) );
			}
			break;
		case  0:
			switch(der_model) {
				case tangent_linear:
					// Original code does not contain any STACKf then do not create one in TL code.
					break;
				case adjoint:
					{
						// Original code does not contain any STACKf then create one in ADJ code.
						ast_vertex* stack_decl = new ast_vertex(0, VT_STACKf_DECLARATION, prefix);
						global_decl->children.insert(it, stack_decl);
						add_child(list, new ast_vertex(0, VT_SPL_STACKf, prefix) );
					}
					break;
				default: break;
			}
			break;
		case  1:
			// In this case the stack for the current derivative order is already defined.
			break;
	}
	if( newTHREADPRIVATE && list->children.size()>0) 
		add_child(global_decl, THREADPRIVATE);
}


void 
ast_vertex::print_statement_tail(ofstream& ofs) 
{
	switch(this->type) {
		case VT_SPL_STACKi: 
		case VT_SPL_STACKf: 
		case VT_SPL_STACKc: 
		case VT_SPL_STACKfpush: 
		case VT_SPL_STACKipush: 
		case VT_SPL_STACKcpush: 
		case VT_SPL_STACKfpop: 
		case VT_SPL_STACKipop: 
		case VT_SPL_STACKcpop: 
		case VT_SPL_int_assign: 
		case VT_SPL_float_assign: 
		case VT_SPL_float_plus_assign: 
		case VT_ATOMIC: 
		case VT_EXCLUSIVE_READ_FAILURE: 
		case VT_ASSERT: 
		case VT_BEFORE_REVERSE_MODE_HOOK: 
			ofs << ";";
			break;
		default:
			break;
	}
}


void
ast_vertex::partition_seq_into_slc_and_cfstmts(list<ast_vertex*> &seqs)
{
	ast_vertex *S=NULL;
	int counter=0;
	ASSUME_TYPE(this, VT_S);
	for(list<ast_vertex*>::const_iterator it=children.begin();it!=children.end();it++) {
		counter++;
		if(S==NULL) S=createS(); assert(S);
		if( (*it)->slc ) 
			add_child(S, *it);
		else { // If we have reached a conditional statement put the previous sequence into a list and afterward the conditional statement.
			if(S->children.size()>0) {
				S->slc=1;
				seqs.push_back(S);	  // Put SLC into seqs
				S=NULL;
			}
			seqs.push_back(*it); // Put CFSTMT statement into seqs
		}
	}
	if(S!=NULL  &&  S->children.size()>0) {
		S->slc=1;
		seqs.push_back(S);	  // Put SLC into seqs
		S=NULL;
	}
	// sanity check 
	int counter2=0;
	for(list<ast_vertex*>::const_iterator it=seqs.begin();it!=seqs.end();it++) {
		if((*it)->type==VT_S) counter2+=(*it)->children.size();
		else counter2++;
	}
	assert(counter==counter2);
}

void 
ast_vertex:: well_formed_loop_test()
{
	string identifier_init;
	string identifier_test;
	string identifier_update;
	assert(type==VT_OMP_FOR);
	// Test init
	ASSUME_TYPE(FIRSTOF(this), VT_SPL_int_assign);
	identifier_init=FIRSTOF(FIRSTOF(this))->get_str();
	// Test test expression
	ASSUME_TYPE(SECONDOF(this), VT_SPL_cond_while);
	ast_vertex *while_loop=SECONDOF(this);
	// Only two children are allowed for test expression, e.g. i<n
	assert(FIRSTOF(while_loop)->children.size()==2);
	identifier_test = FIRSTOF(FIRSTOF(while_loop))->get_str();
	// Test update
	ast_vertex *S=SECONDOF(while_loop);
	ASSUME_TYPE(S, VT_S);  
	assert(S->children.size()>=1);
	ast_vertex *last_stmt=LASTOF(S);
	ASSUME_TYPE(last_stmt, VT_SPL_int_assign);
	identifier_update=FIRSTOF(last_stmt)->get_str();
	// Test that all have the same identifier
	if( identifier_init!=identifier_test ) {
		cerr << "error: Loop well formed test fails: init and test variable are not the same.\n";
		assert(0);
	}
	if( identifier_init!=identifier_update ) {
		cerr << "error: Loop well formed test fails: init and update variable are not the same.\n";
		assert(0);
	}
}


ast_vertex* 
ast_vertex::createSTACKcpush(int label, const string &stack_prefix)
{
	ostringstream oss; oss << label;
	ast_vertex *STACKcpush=new ast_vertex(0, VT_SPL_STACKcpush, stack_prefix);
	add_child(STACKcpush, new ast_vertex(0, VT_SPL_int_const, oss.str()) );
	return STACKcpush;
}

ast_vertex* 
ast_vertex::createSTACKcpush(const string variable_name, const string &stack_prefix)
{
	ast_vertex *STACKcpush=new ast_vertex(0, VT_SPL_STACKcpush, stack_prefix);
	add_child(STACKcpush, new ast_vertex(0, VT_SPL_int_const, variable_name) );
	return STACKcpush;
}

ast_vertex* 
ast_vertex::createBranchSTACKcTop_equal_label(int label, const string &stack_prefix, ast_vertex *subseq)
{
	ostringstream oss; oss << label;
	ast_vertex* astv_label=new ast_vertex( 0, VT_SPL_int_const, oss.str() );
	ast_vertex* testexpr = new ast_vertex(0, VT_SPL_eq, "");
	ast_vertex* STACKctop = new ast_vertex(0, VT_SPL_STACKctop, stack_prefix);
	add_child(testexpr, STACKctop);
	add_child(testexpr, astv_label);
	ast_vertex* branch = new ast_vertex(0, VT_SPL_cond_if, "");
	add_child(branch, testexpr);
	add_child(branch, subseq);
	return branch;
}

ast_vertex* 
ast_vertex::clone_node()
{
	ast_vertex* v=new ast_vertex(line_number, type, str);
	v->connect2unparser();
	v->valid();
	return v;
}

ast_vertex* 
ast_vertex::clone()
{
	ast_vertex* v=new ast_vertex(line_number, type, str);
	for(list<ast_vertex*>::const_iterator it=children.begin();it!=children.end();it++)  {
		add_child( v, (*it)->clone() );
	}
	v->connect2unparser();
	v->valid();
	return v;
}

void 
ast_vertex::appendS(ast_vertex* S) 
{
	assert(this->type==VT_S);
	ASSUME_TYPE(S, VT_S);
	for(list<ast_vertex*>::const_iterator it=S->children.begin();it!=S->children.end();it++)  {
		this->children.push_back(*it);
	}
	delete S;
}

void 
ast_vertex::appendD(ast_vertex* D) 
{
	assert(this->type==VT_SEQ_DECLARATIONS);
	assert(D->type==VT_SEQ_DECLARATIONS);
	for(list<ast_vertex*>::const_iterator it=D->children.begin();it!=D->children.end();it++)  {
		this->children.push_back(*it);
	}
	delete D;
}


const string 
ast_vertex::get_lhs_name()
{
	ast_vertex* c;
	switch(type) {
	case VT_INT_POINTER_DECLARATION:
	case VT_FLOAT_POINTER_DECLARATION:
		c=(*children.begin());
		if(c->type==VT_SPL_identifier)
			return c->str;
		else
			assert(0);
		break;
	case VT_INT_DECLARATION:
	case VT_UINT_DECLARATION:
		c=(*children.begin());
		if(c->type==VT_SPL_int_assign)
			return c->get_lhs_name();
		else if(c->type==VT_SPL_identifier)
			return c->str;
		else
			assert(0);
		break;
	case VT_FLOAT_DECLARATION:
		c=(*children.begin());
		if(c->type==VT_SPL_float_assign)
			return c->get_lhs_name();
		else if(c->type==VT_SPL_identifier)
			return c->str;
		else
			assert(0);
		break;
	case VT_SPL_int_assign:
	case VT_SPL_float_assign:
	case VT_SPL_float_plus_assign:
		return (*children.begin())->str;
		break;
	case VT_STACKc_DECLARATION: { string s(get_str() + "STACKc"); return s; }
	case VT_STACKi_DECLARATION: { string s(get_str() + "STACKi"); return s; }
	case VT_STACKf_DECLARATION: { string s(get_str() + "STACKf"); return s; }
		break;
	default:
		cerr << "error: type: " << vertex_type_2_string() << endl;
		assert(0);
	}
}

void 
ast_vertex::collect_labels_from(list<string> &l)
{
	if(type==VT_SPL_STACKcpush) { l.push_back( FIRSTOF(this)->get_str() ); }
	FOR_EACH_it_IN(children) { (*it)->collect_labels_from(l); }
}

ast_vertex*
ast_vertex::get_complement_test()
{
	ast_vertex* tmp = this->clone();
	switch( tmp->get_type() ) {
		case VT_SPL_eq:      tmp->type = VT_SPL_neq; break;
		case VT_SPL_neq:     tmp->type = VT_SPL_eq;  break;
		case VT_SPL_leq:     tmp->type = VT_SPL_greater;  break;
		case VT_SPL_geq:     tmp->type = VT_SPL_lower;  break;
		case VT_SPL_lower:   tmp->type = VT_SPL_geq;  break;
		case VT_SPL_greater: tmp->type = VT_SPL_leq;  break;
		case VT_SPL_not:     delete tmp; 
							 tmp = FIRSTOF(this)->clone();  
							 break;
		default:
			UNKNOWN_TYPE(tmp); // This should not happen
	}
	tmp->connect2unparser();
	return tmp;
}

/******************* GLOBAL *************************/

void 
insert_child(ast_vertex *parent, list<ast_vertex*>::iterator it, ast_vertex *child)
{
  child->predecessor=parent;
  parent->children.insert(it, child);
}

void 
add_child(ast_vertex *p, ast_vertex *c) 
{ 
	p->children.push_back(c); 
	c->predecessor = p; 
	// Set init node for parent node
	if(p->type==VT_S && p->children.size()==1 ) 
		p->init = c;
}


bool
operator_equal(const ast_vertex * const l, const ast_vertex * const r)
{
	if( l->children.size() == r->children.size() ) {
		list<ast_vertex*>::const_iterator it_l=l->children.begin();
		list<ast_vertex*>::const_iterator it_r=r->children.begin();
		while(it_l!=l->children.end()) {
			if( !operator_equal(*it_l, *it_r) )
				return false;
			it_l++; it_r++;
		}
		// Now we can be sure that the sub trees are equal.
		if( l->get_type()==r->get_type() && l->get_str()==r->get_str() ) 
			return true;
		else 
			return false;
	}
	else
		return false;
}

uint
ast_vertex::depth()
{
	uint length=0;
   	depth_worker(0, length) ;
	return length;
}

void
ast_vertex::depth_worker(uint level, uint &max)
{
	//cerr << "level: " << level << "  max: " << max << endl;
	FOR_EACH_it_IN(children) {
		(*it)->depth_worker(level+1, max);
	}
	if(level>max) max=level;
}


void
ast_vertex::replace(ast_vertex* subst_node)
{
	bool found=false;
	// May not hold: assert(this->predecessor!=NULL);
	if(this->predecessor!=NULL) { // If there is a predecessor.
		list<ast_vertex*> kids_of_predecessor=this->predecessor->children;
		this->predecessor->children.clear();
		FOR_EACH_it_IN(kids_of_predecessor) {
			if( *it==this ) {
				found=true;
				subst_node->predecessor=this->predecessor;
				this->predecessor->children.push_back(subst_node);
			}
			else {
				(*it)->predecessor=this->predecessor;
				this->predecessor->children.push_back(*it);
			}
		}
		assert(found==true);
	}
	else {
		*this=*subst_node;
	}
}

void 
ast_vertex::normalize_arithmetic_expression()
{
	FOR_EACH_it_IN(children) {
		(*it)->normalize_arithmetic_expression();
	}
	if(     ( get_type()==VT_SPL_int_add || get_type()==VT_SPL_int_sub )
		&&  SECONDOF(this)->get_type()==VT_SPL_int_const 
	  ) {
		istringstream iss_c(SECONDOF(this)->get_str()); long c; iss_c>>c;
		if(c==0) {
			replace( FIRSTOF(this) );
		}
	}
	if(     ( get_type()==VT_SPL_int_mul || get_type()==VT_SPL_int_div )
		&&  SECONDOF(this)->get_type()==VT_SPL_int_const 
	  ) {
		istringstream iss_c(SECONDOF(this)->get_str()); long c; iss_c>>c;
		if(c==1) {
			replace( FIRSTOF(this) );
		}
	}
	if(     ( get_type()==VT_SPL_int_add || get_type()==VT_SPL_int_sub )
		&&  SECONDOF(this)->get_type()==VT_SPL_int_const 
		&&  (FIRSTOF(this)->get_type()==VT_SPL_int_add || FIRSTOF(this)->get_type()==VT_SPL_int_sub)
		&&  SECONDOF(FIRSTOF(this))->get_type()==VT_SPL_int_const 
	  ) {
		ast_vertex* first_op=this;
		ast_vertex* second_op=FIRSTOF(this);
		ast_vertex* S=FIRSTOF(FIRSTOF(this));
		istringstream iss_c1(SECONDOF(FIRSTOF(this))->get_str()); long c1; iss_c1>>c1;
		istringstream iss_c2(SECONDOF(this)->get_str()); long c2; iss_c2>>c2;
		long c1_op_c2;
		ast_vertex* normalized_op=NULL;
		switch( first_op->get_type() ) {
			case VT_SPL_int_add:
				switch( second_op->get_type() ) {
					case VT_SPL_int_add:
						c1_op_c2 = c1+c2;
						normalized_op = new ast_vertex(0, VT_SPL_int_add, "");
						break;
					case VT_SPL_int_sub:
						c1_op_c2 = c1-c2;
						if(c1_op_c2>=0)
							normalized_op = new ast_vertex(0, VT_SPL_int_sub, "");
						else {
							c1_op_c2 *= -1;
							normalized_op = new ast_vertex(0, VT_SPL_int_add, "");
						}
						break;
					default: break ;
				}
				break;
			case VT_SPL_int_sub:
				switch( second_op->get_type() ) {
					case VT_SPL_int_add:
						c1_op_c2 = c1-c2;
						if(c1_op_c2>=0)
							normalized_op = new ast_vertex(0, VT_SPL_int_add, "");
						else {
							c1_op_c2 *= -1;
							normalized_op = new ast_vertex(0, VT_SPL_int_sub, "");
						}
						break;
					case VT_SPL_int_sub:
						c1_op_c2 = c1+c2;
						normalized_op = new ast_vertex(0, VT_SPL_int_sub, "");
						break;
					default: break ;
				}
				break;
			default: break ;
		}
		assert(normalized_op);
		if( c1_op_c2==0 ) {
			delete normalized_op;
			replace(S);
		}
		else {
			ostringstream oss; oss << c1_op_c2;
			ast_vertex* astv_c1_op_c2 = new ast_vertex(0, VT_SPL_int_const, oss.str());
			add_child(normalized_op, S);
			add_child(normalized_op, astv_c1_op_c2);
			replace(normalized_op);
		}
	}
}



void
ast_vertex::are_there_only_uints(set<ast_vertex*> &s)
{
	FOR_EACH_it_IN(children) { (*it)->are_there_only_uints(s); }
	if( type==VT_SPL_identifier ) {
		ast_vertex* decl = sym_tbl.get_decl(str);
		if( decl->get_type()==VT_UINT_DECLARATION ) 
			s.insert(decl);
	}
}


