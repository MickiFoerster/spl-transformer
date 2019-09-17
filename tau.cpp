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
#include "symbol_table.h"
#include "ast_vertex.h"

extern symbol_table sym_tbl;

ast_vertex*
ast_vertex::tau()
{
	ast_vertex *v=this;
	ast_vertex* pendant=NULL;
	ast_vertex* D=NULL;
	ast_vertex* S=NULL;
	switch(v->type) {
		case VT_SPL_CODE : 
		{
			ast_vertex* result_from_tauGlobalDecl = FIRST->tau(); 
			assert(result_from_tauGlobalDecl->type==VT_SEQ_DECLARATIONS);
			result_from_tauGlobalDecl->type=VT_GLOBAL_DECLARATIONS;
			result_from_tauGlobalDecl->unparser=&unparser_global_declarations;
			global_seq_decls=result_from_tauGlobalDecl;
			create_thread_local_stacks(result_from_tauGlobalDecl, tangent_linear);

			assert(global_seq_decls);
			ast_vertex* result_from_tauParallelRegion = SECOND->tau();
			assert(result_from_tauParallelRegion->type==VT_PARALLEL_REGION || result_from_tauParallelRegion->type==VT_PARALLEL_FOR_REGION );
			// Build new spl_code node with the transformations as successors.
			ast_vertex* spl_code = new ast_vertex(0, VT_SPL_CODE, "");
			add_child(spl_code, result_from_tauGlobalDecl);
			add_child(spl_code, result_from_tauParallelRegion);
			global_seq_decls=NULL;
			local_seq_decls=NULL;
			return spl_code;
		}
		case VT_PARALLEL_REGION : 
		{   // 1st:  decl, 2nd: seq_of_stmts
			D=FIRST;
			S=SECOND;
			pendant = new ast_vertex(v->line_number, VT_PARALLEL_REGION, "");
			break;
		}
		case VT_PARALLEL_FOR_REGION :
		{ // 1st: loop, 2nd: decl, 3rd: seq_of_stmts
			D=SECOND;
			S=THIRD;
			pendant = new ast_vertex(v->line_number, VT_PARALLEL_FOR_REGION, "");
			add_child( pendant, FIRST->clone() ); /* Take for-loop as given in the origin parallel region. */
			break;
		}
		case VT_GLOBAL_DECLARATIONS:
		case VT_SEQ_DECLARATIONS:
			return tau_seq_decl();
		case VT_S:
			return tau_seq_of_stmts();
		case VT_SPL_cond_if:
		case VT_SPL_cond_while:
			return tau_conditional();
			break;
		case VT_SPL_STACKfpush:
		case VT_SPL_STACKipush:
		case VT_SPL_STACKcpush:
		case VT_SPL_STACKfpop:
		case VT_SPL_STACKipop:
		case VT_SPL_STACKcpop:
			return tau_stack();
			break;
		case VT_SPL_int_assign:
		case VT_SPL_float_assign:
		case VT_SPL_float_plus_assign:
			return tau_assign();
			break;
		case VT_OMP_FOR: 
			return tau_omp_for(); break;
		case VT_BARRIER:  
			return tau_barrier(); break;
		case VT_DUMMY: 
			return tau_dummy(); break;
		case VT_ATOMIC:  
			return tau_atomic(); break;
		case VT_EXCLUSIVE_READ_FAILURE:  
			return FIRSTOF(this)->tau_assign(); break;
		case VT_MASTER:  
			return tau_master(); break;
		case VT_CRITICAL:  
			return tau_critical(); break;
		case VT_ASSERT: 
			return tau_assert(); break;
		default:
			UNKNOWN_TYPE(v);
	}
	ASSUME_TYPE(D,VT_SEQ_DECLARATIONS);
	ASSUME_TYPE(S,VT_S);
	/* Now create the calls as defined in the rule for tau(P) */
	ast_vertex* result_from_tauD = D->tau(); assert(result_from_tauD->type==VT_SEQ_DECLARATIONS);
	ast_vertex* result_from_tauS = S->tau(); assert(result_from_tauS->type==VT_S);
	/* Build together P = D S  */
	D=result_from_tauD;
	// Collect parts for the final S
	S=result_from_tauS;
	// Add D and S as childs for pendant for parallel region.
	add_child(pendant, D);
	add_child(pendant, S);
	return pendant;
}

ast_vertex*
ast_vertex::tau_omp_for()
{
	ast_vertex* v=this;
	ast_vertex* loop_init=FIRST;
	ast_vertex* loop=SECOND;
	ast_vertex* S=NULL;
	ast_vertex* tau_omp_for=NULL;
	ast_vertex* tau_loop=NULL;
	ast_vertex* tau_loop_init=NULL;

	ASSUME_TYPE(FIRST, VT_SPL_int_assign);
	ASSUME_TYPE(SECOND, VT_SPL_cond_while);
	S=createS();
	tau_omp_for = new ast_vertex(0, VT_OMP_FOR, "");
	tau_loop_init = loop_init->tau() ;
	tau_loop      = loop->tau() ;

	add_child( tau_omp_for, (*tau_loop_init->children.begin()) );
	add_child( tau_omp_for, tau_loop );
	add_child( S, tau_omp_for );

	return S;
}

ast_vertex* 
ast_vertex::tau_assign()
{
	string i;
	string der_i;
	ast_vertex* v=this;
	ast_vertex* S=createS();
	switch(type) {
		case VT_SPL_int_assign:
			add_child( S, this->clone() ); 
			return S;
			break;
		case VT_SPL_float_assign:
		case VT_SPL_float_plus_assign:
		{
			ast_vertex* lhs=NULL;
			ast_vertex* rhs=NULL;
			ast_vertex* assign=NULL;

			if( SECOND->type==VT_SPL_STACKftop ) { // If right-hand side is stack operation
				// Build assignment that assigns tangent-linear value of RHS to tangent-linear variable of origin LHS variable.
				lhs = FIRST->clone();
				lhs->str = tl_prefix + lhs->str;
				rhs = SECOND->clone();
				rhs->str = tl_prefix;
				assign = new ast_vertex(0, type, "");
				add_child(assign, lhs);
				add_child(assign, rhs);
				add_child(S, assign);

				// Build assignment that assigns value of RHS to origin LHS variable.
				add_child( S, this->clone() ); 
			}
			else { // If right-hand side is phi(x)
				S->appendS( build_tl_rhs(i,der_i) );
				// Build assignment that assigns tangent-linear value of RHS to tangent-linear variable of origin LHS variable.
				lhs = FIRST->clone();
				lhs->str = tl_prefix + lhs->str;
				rhs = new ast_vertex(0, VT_SPL_identifier, tl_prefix+i);
				assign = new ast_vertex(0, type, "");
				add_child(assign, lhs);
				add_child(assign, rhs);
				add_child(S, assign);

				// Build assignment that assigns value of RHS to origin LHS variable.
				lhs = FIRST->clone();
				rhs = new ast_vertex(0, VT_SPL_identifier, i);
				assign = new ast_vertex(0, type, "");
				add_child(assign, lhs);
				add_child(assign, rhs);
				add_child(S, assign);
			}
			break;
		}
		default:
			UNKNOWN_TYPE(this);
	}
	assert(S->type==VT_S);
	return S;
}

ast_vertex* 
ast_vertex::tau_conditional()
{
	ast_vertex* S=createS();
	ast_vertex* conditional_stmt=NULL;
	ast_vertex* conditional_testexpr=NULL;
	ast_vertex* subseq=NULL;
	switch(type) {
		case VT_SPL_cond_if:
			conditional_stmt = new ast_vertex(0, VT_SPL_cond_if, "");
			break;
		case VT_SPL_cond_while:
			conditional_stmt = new ast_vertex(0, VT_SPL_cond_while, "");
			break;
		default:
			assert(0);
	}
	conditional_testexpr = (*children.begin())->clone();
	subseq = (*++children.begin())->tau(); assert(subseq->type==VT_S);
	add_child( conditional_stmt, conditional_testexpr );
	add_child( conditional_stmt, subseq );
	add_child( S, conditional_stmt );
	assert(S->type==VT_S);
	return S;
}

ast_vertex* 
ast_vertex::tau_seq_decl()
{
	switch(type) {
		case VT_SEQ_DECLARATIONS:
		case VT_GLOBAL_DECLARATIONS:
			break;
		default:
			UNKNOWN_TYPE(this);
	}
	ast_vertex* D = createD();
	for(list<ast_vertex*>::const_iterator it=children.begin();it!=children.end();it++)  {
		D->appendD( (*it)->tau_decl() );
	}
	assert(D->type==VT_SEQ_DECLARATIONS);
	local_seq_decls=D;
	return D;
}

ast_vertex* 
ast_vertex::tau_decl()
{
	ast_vertex* D = createD();
	ast_vertex* decl;
	// First filter correct type values.
	switch(type) {
		case VT_INT_DECLARATION:
		case VT_UINT_DECLARATION:
		case VT_FLOAT_DECLARATION:
		case VT_INT_POINTER_DECLARATION:
		case VT_FLOAT_POINTER_DECLARATION:
		case VT_STACKc_DECLARATION: 
		case VT_STACKi_DECLARATION:
		case VT_STACKf_DECLARATION: 
		case VT_THREADPRIVATE:
			break;
		default:
			cerr << "error: unknown type " << vertex_type_2_string()<<endl;
			assert(0);
	}
	switch(type) {
	case VT_INT_POINTER_DECLARATION:
	case VT_INT_DECLARATION:
	case VT_UINT_DECLARATION:
		decl = clone();
		add_child(D, decl);
		break;
	case VT_FLOAT_POINTER_DECLARATION: 
		{
			symbol *sym=sym_tbl.get_sym(get_lhs_name());
			decl = clone();
			add_child(D, decl);
			ast_vertex* decl2 = clone();
			(*decl2->children.begin())->str = tl_prefix + (*decl2->children.begin())->str;
			add_child(D, decl2);
			bool var_is_not_intermediate=false;
			bool var_is_a_new_derivative_component=true;
			sym_tbl.insert(decl2, sym->shared, var_is_not_intermediate, var_is_a_new_derivative_component);
			break;
		}
	case VT_FLOAT_DECLARATION: 
		{
			symbol *sym=sym_tbl.get_sym(get_lhs_name());
			// decl = 'double v'
			decl = clone();
			// decl2 = 'double t1_v=0.'
			ast_vertex* decl2 = new ast_vertex(0, VT_FLOAT_DECLARATION, ""); 
			ast_vertex* decl2floatass = new ast_vertex(0, VT_SPL_float_assign, ""); 
			add_child(decl2, decl2floatass);
			ast_vertex* lhs = new ast_vertex(0, VT_SPL_identifier, tl_prefix + decl->get_lhs_name());
			add_child(decl2floatass, lhs);
			ast_vertex* rhs = new ast_vertex(0, VT_SPL_float_const, "0.");
			add_child(decl2floatass, rhs);

			add_child(D, decl);
			add_child(D, decl2);
			bool var_is_not_intermediate=false;
			bool var_is_a_new_derivative_component=true;
			sym_tbl.insert(decl2, sym->shared , var_is_not_intermediate, var_is_a_new_derivative_component);
		}
		break;
	case VT_STACKc_DECLARATION: 
	case VT_STACKi_DECLARATION:
	case VT_STACKf_DECLARATION: 
	case VT_THREADPRIVATE:
		decl = clone();
		add_child(D, decl);
		break;
	default:
		assert(0);
	}
	assert(D->type==VT_SEQ_DECLARATIONS);
	return D;
}

ast_vertex* 
ast_vertex::tau_seq_of_stmts()
{
	assert(type==VT_S);
	ast_vertex* S = createS();
	for(list<ast_vertex*>::const_iterator it=children.begin();it!=children.end();it++)  {
		S->appendS( (*it)->tau() );
	}
	assert(S->type==VT_S);
	return S;
}

ast_vertex* 
ast_vertex::tau_stack()
{
	ast_vertex* v;
	ast_vertex* Sf;
	switch(type) {
		case VT_SPL_STACKipush:
		case VT_SPL_STACKcpush:
		case VT_SPL_STACKipop:
		case VT_SPL_STACKcpop:
		case VT_SPL_STACKfpush:
		case VT_SPL_STACKfpop:
			break;
		default:
			assert(0);
	}
	ast_vertex* S=createS(); 
	switch(type) {
		case VT_SPL_STACKipush:
		case VT_SPL_STACKcpush:
		case VT_SPL_STACKipop:
		case VT_SPL_STACKcpop:
			break;
		case VT_SPL_STACKfpush:
			Sf= clone();
			v=Sf;
			FIRST->str = tl_prefix + FIRST->str;
			Sf->str = tl_prefix; // The stack gets only the current prefix not possible previous prefixes.
			add_child(S, Sf);
			break;
		case VT_SPL_STACKfpop:
			Sf= clone();
			Sf->str = tl_prefix;
			add_child(S, Sf);
			break;
		default:
			assert(0);
	}
	add_child(S, clone()); // Identity mapping
	assert(S->type==VT_S);	
	return S; 
}


ast_vertex*  
ast_vertex::build_tl_rhs(string& n, string& der_n)
{
	ast_vertex* S=NULL;
	ast_vertex* v=this;
	ast_vertex* assign=NULL;
	ast_vertex* lhs=NULL;
	ast_vertex* rhs=NULL;
	ast_vertex* tl_assign=NULL;
	ast_vertex* tl_lhs=NULL;
	ast_vertex* tl_rhs=NULL;
	ast_vertex* tmp1=NULL;
	ast_vertex* tmp2=NULL;
	string l,r;
	string der_l,der_r;

	S=createS();
	if(type==VT_SPL_float_assign || type==VT_SPL_float_plus_assign) {
		set_intermediate_name_counter_to_zero();
		S->appendS( SECOND->build_tl_rhs(l,der_l) );
		n=l; der_n=der_l;
	}
	else {
		switch(type) {
			case VT_SPL_float_add:
				S->appendS( FIRST->build_tl_rhs(l,der_l) );
				S->appendS( SECOND->build_tl_rhs(r,der_r) );
				get_intermediate_name(tangent_linear, n, der_n);
				// Build tangent-linear assignment
				tl_assign = new ast_vertex(0, VT_SPL_float_assign, "");
				tl_lhs = new ast_vertex(0, VT_SPL_identifier, der_n);
				add_child(tl_assign, tl_lhs);
				tl_rhs = new ast_vertex(0, VT_SPL_float_add, "");
				add_child(tl_rhs, new ast_vertex(0, VT_SPL_identifier, der_l));
				add_child(tl_rhs, new ast_vertex(0, VT_SPL_identifier, der_r));
				add_child(tl_assign, tl_rhs);

				// Build assignment v2=v0+v1
				assign = new ast_vertex(0, VT_SPL_float_assign, "");
				lhs = new ast_vertex(0, VT_SPL_identifier, n);
				add_child(assign, lhs);
				rhs = new ast_vertex(0, VT_SPL_float_add, "");
				add_child(rhs, new ast_vertex(0, VT_SPL_identifier, l));
				add_child(rhs, new ast_vertex(0, VT_SPL_identifier, r));
				add_child(assign, rhs);

				add_child(S, tl_assign);
				add_child(S, assign);
				break;
			case VT_SPL_float_sub:
				S->appendS( FIRST->build_tl_rhs(l, der_l) );
				S->appendS( SECOND->build_tl_rhs(r, der_r) );
				get_intermediate_name(tangent_linear, n, der_n);
				// Build tangent-linear assignment
				tl_assign = new ast_vertex(0, VT_SPL_float_assign, "");
				tl_lhs = new ast_vertex(0, VT_SPL_identifier, der_n);
				add_child(tl_assign, tl_lhs);
				tl_rhs = new ast_vertex(0, VT_SPL_float_sub, "");
				add_child(tl_rhs, new ast_vertex(0, VT_SPL_identifier, der_l));
				add_child(tl_rhs, new ast_vertex(0, VT_SPL_identifier, der_r));
				add_child(tl_assign, tl_rhs);

				// Build assignment v2=v0-v1
				assign = new ast_vertex(0, VT_SPL_float_assign, "");
				lhs = new ast_vertex(0, VT_SPL_identifier, n);
				add_child(assign, lhs);
				rhs = new ast_vertex(0, VT_SPL_float_sub, "");
				add_child(rhs, new ast_vertex(0, VT_SPL_identifier, l));
				add_child(rhs, new ast_vertex(0, VT_SPL_identifier, r));
				add_child(assign, rhs);

				add_child(S, tl_assign);
				add_child(S, assign);
				break;
			case VT_SPL_float_mul:
				S->appendS( FIRST->build_tl_rhs(l, der_l) );
				S->appendS( SECOND->build_tl_rhs(r, der_r) );
				get_intermediate_name(tangent_linear, n, der_n);
				// Build tangent-linear assignment
				// t1_x*y + x*t1_y
				tl_assign = new ast_vertex(0, VT_SPL_float_assign, "");
				tl_lhs = new ast_vertex(0, VT_SPL_identifier, der_n);
				add_child(tl_assign, tl_lhs);
				tmp1 = new ast_vertex(0, VT_SPL_float_mul, "");
				add_child(tmp1, new ast_vertex(0, VT_SPL_identifier, der_l));
				add_child(tmp1, new ast_vertex(0, VT_SPL_identifier, r));
				tl_rhs = new ast_vertex(0, VT_SPL_float_add, "");
				add_child(tl_rhs, tmp1);
				tmp1 = new ast_vertex(0, VT_SPL_float_mul, "");
				add_child(tmp1, new ast_vertex(0, VT_SPL_identifier, l));
				add_child(tmp1, new ast_vertex(0, VT_SPL_identifier, der_r));
				add_child(tl_rhs, tmp1);
				add_child(tl_assign, tl_rhs);

				// Build assignment v2=v0*v1
				assign = new ast_vertex(0, VT_SPL_float_assign, "");
				lhs = new ast_vertex(0, VT_SPL_identifier, n);
				add_child(assign, lhs);
				rhs = new ast_vertex(0, VT_SPL_float_mul, "");
				add_child(rhs, new ast_vertex(0, VT_SPL_identifier, l));
				add_child(rhs, new ast_vertex(0, VT_SPL_identifier, r));
				add_child(assign, rhs);

				add_child(S, tl_assign);
				add_child(S, assign);
				break;
			case VT_SPL_float_sin:
				S->appendS( FIRST->build_tl_rhs(l, der_l) );
				get_intermediate_name(tangent_linear, n, der_n);
				// Build tangent-linear assignment
				tl_assign = new ast_vertex(0, VT_SPL_float_assign, "");
				tl_lhs = new ast_vertex(0, VT_SPL_identifier, der_n);
				add_child(tl_assign, tl_lhs);
				tl_rhs = new ast_vertex(0, VT_SPL_float_mul, "");
				add_child(tl_rhs, new ast_vertex(0, VT_SPL_identifier, der_l));
				tmp1 = new ast_vertex(0, VT_SPL_float_cos, "");
				add_child(tmp1, new ast_vertex(0, VT_SPL_identifier, l));
				add_child(tl_rhs, tmp1);
				add_child(tl_assign, tl_rhs);

				// Build assignment v1=sin(v0)
				assign = new ast_vertex(0, VT_SPL_float_assign, "");
				lhs = new ast_vertex(0, VT_SPL_identifier, n);
				add_child(assign, lhs);
				rhs = new ast_vertex(0, VT_SPL_float_sin, "");
				add_child(rhs, new ast_vertex(0, VT_SPL_identifier, l));
				add_child(assign, rhs);

				add_child(S, tl_assign);
				add_child(S, assign);
				break;
			case VT_SPL_float_cos:
				S->appendS( FIRST->build_tl_rhs(l, der_l) );
				get_intermediate_name(tangent_linear, n, der_n);
				// Build tangent-linear assignment
				tl_assign = new ast_vertex(0, VT_SPL_float_assign, "");
				tl_lhs = new ast_vertex(0, VT_SPL_identifier, der_n);
				add_child(tl_assign, tl_lhs);
				tmp1 = new ast_vertex(0, VT_SPL_float_sub, "");
				add_child(tmp1, new ast_vertex(0, VT_SPL_float_const, "0."));
				tmp2 = new ast_vertex(0, VT_SPL_float_sin, "");
				add_child(tmp2, new ast_vertex(0, VT_SPL_identifier, l));
				add_child(tmp1, tmp2);
				// ( 0 - sin(x) )
				tmp2 = new ast_vertex(0, VT_SPL_expr_in_brackets, "");
				add_child(tmp2, tmp1);
				tl_rhs = new ast_vertex(0, VT_SPL_float_mul, "");
				add_child(tl_rhs, new ast_vertex(0, VT_SPL_identifier, der_l));
				add_child(tl_rhs, tmp2);
				add_child(tl_assign, tl_rhs);

				// Build assignment v1=cos(v0)
				assign = new ast_vertex(0, VT_SPL_float_assign, "");
				lhs = new ast_vertex(0, VT_SPL_identifier, n);
				add_child(assign, lhs);
				rhs = new ast_vertex(0, VT_SPL_float_cos, "");
				add_child(rhs, new ast_vertex(0, VT_SPL_identifier, l));
				add_child(assign, rhs);

				add_child(S, tl_assign);
				add_child(S, assign);
				break;
			case VT_SPL_float_exp:
				S->appendS( FIRST->build_tl_rhs(l, der_l) );
				get_intermediate_name(tangent_linear, n, der_n);
				// Build tangent-linear assignment: t1_v1 = t1_v0 * exp(v0)
				tl_assign = new ast_vertex(0, VT_SPL_float_assign, "");
				tl_lhs = new ast_vertex(0, VT_SPL_identifier, der_n);
				add_child(tl_assign, tl_lhs);
				tl_rhs = new ast_vertex(0, VT_SPL_float_mul, "");
				add_child(tl_rhs, new ast_vertex(0, VT_SPL_identifier, der_l));
				tmp1 = new ast_vertex(0, VT_SPL_float_exp, "");
				add_child(tmp1, new ast_vertex(0, VT_SPL_identifier, l));
				add_child(tl_rhs, tmp1);
				add_child(tl_assign, tl_rhs);

				// Build assignment v1=exp(v0)
				assign = new ast_vertex(0, VT_SPL_float_assign, "");
				lhs = new ast_vertex(0, VT_SPL_identifier, n);
				add_child(assign, lhs);
				rhs = new ast_vertex(0, VT_SPL_float_exp, "");
				add_child(rhs, new ast_vertex(0, VT_SPL_identifier, l));
				add_child(assign, rhs);

				add_child(S, tl_assign);
				add_child(S, assign);
				break;
			case VT_SPL_identifier: 
				get_intermediate_name(tangent_linear, n, der_n);
				// Build tangent-linear assignment
				tl_assign = new ast_vertex(0, VT_SPL_float_assign, "");
				tl_lhs = new ast_vertex(0, VT_SPL_identifier, der_n);
				add_child(tl_assign, tl_lhs);
				tl_rhs = clone();
				tl_rhs->str = tl_prefix + tl_rhs->str;
				add_child(tl_assign, tl_rhs);

				assign = new ast_vertex(0, VT_SPL_float_assign, "");
				lhs = new ast_vertex(0, VT_SPL_identifier, n);
				add_child(assign, lhs);
				rhs = clone();
				add_child(assign, rhs);
				
				add_child(S, tl_assign);
				add_child(S, assign);
				break;
			case VT_SPL_float_const:
				get_intermediate_name(tangent_linear, n, der_n);
				// Build tangent-linear assignment
				tl_assign = new ast_vertex(0, VT_SPL_float_assign, "");
				tl_lhs = new ast_vertex(0, VT_SPL_identifier, der_n);
				add_child(tl_assign, tl_lhs);
				tl_rhs = new ast_vertex(0, VT_SPL_float_const, "0.");
				add_child(tl_assign, tl_rhs);

				assign = new ast_vertex(0, VT_SPL_float_assign, "");
				lhs = new ast_vertex(0, VT_SPL_identifier, n);
				add_child(assign, lhs);
				rhs = clone();
				add_child(assign, rhs);
				// Append assignments to S
				add_child(S, tl_assign);
				add_child(S, assign);
				break;
			case VT_SPL_expr_in_brackets:
				S->appendS( FIRST->build_tl_rhs(n, der_n) );
				break;
			case VT_SPL_array_index:
			case VT_SPL_STACKftop:
				cerr << "error: This type should not occur: " << vertex_type_2_string() << " (node id "<< get_id() << ")" << endl;
				assert(0);
				break;
			default: 
				cerr << "error: Do not know how to differentiate vertex type " << v->vertex_type_2_string() << endl; assert(0);
		}
	}
	assert(S->type==VT_S);
	return S;
}

ast_vertex*
ast_vertex::tau_barrier()
{
	ast_vertex* S=NULL;
	ASSUME_TYPE(this, VT_BARRIER);
	S=createS();
	add_child(S, clone());
	ASSUME_TYPE(S, VT_S);
	return S;
}


ast_vertex*
ast_vertex::tau_dummy()
{
	ast_vertex* v=this;
	ast_vertex* S=NULL;
	list<ast_vertex*> l;
	ast_vertex* list_of_vars=FIRST;
	ast_vertex* result_tau_dummy=clone();

	list<ast_vertex*>::const_iterator begin_iterator=list_of_vars->get_children().begin();
	for(list<ast_vertex*>::const_iterator it=begin_iterator;it!=list_of_vars->get_children().end();it++) 
	{ 
		ast_vertex* tl_pendant = (*it)->clone();
		tl_pendant->str = tl_prefix + tl_pendant->str;
		l.push_back( tl_pendant );
	}
	// Add new allocated childs to the new allocated list of variables.
	v=result_tau_dummy;
	list_of_vars=FIRST;
	for(list<ast_vertex*>::const_iterator it=l.begin();it!=l.end();it++) {
		add_child(list_of_vars, *it);
	}
	S=createS();
	add_child(S, result_tau_dummy);
	ASSUME_TYPE(S, VT_S);
	return S;
}


ast_vertex*
ast_vertex::tau_master()
{
	ASSUME_TYPE_AND_CLONE_ROOT_AND_APPLY_TRANS_TO_FIRST(VT_MASTER, tau);
}


ast_vertex*
ast_vertex::tau_atomic()
{ 
	ast_vertex* v=this;
	ast_vertex* S=NULL;
	ASSUME_TYPE(this, VT_ATOMIC);
	ast_vertex* lhs=FIRSTOF(FIRST);  // lhs of assignment
	ast_vertex* result_tau=FIRST->tau(); ASSUME_TYPE(result_tau, VT_S);
	ast_vertex* second_last_assignment = SECOND_LASTOF(result_tau); assert( FIRSTOF(second_last_assignment)->get_str()==tl_prefix+lhs->get_str() );
	ast_vertex* last_assignment        = LASTOF(result_tau);        assert( FIRSTOF(last_assignment)->get_str()==lhs->get_str() );
	// Erase the last two assignments because they will be replaced with atomic constructs.
	result_tau->children.pop_back();   result_tau->children.pop_back();
	// Build atomic statements
	ast_vertex* atomic1=clone_node();
	ast_vertex* atomic2=clone_node();
	add_child(atomic1, second_last_assignment);
	add_child(atomic2, last_assignment);
	add_child(result_tau, atomic1);
	add_child(result_tau, atomic2);
	S=result_tau;
	ASSUME_TYPE(S, VT_S);
	return S;
}


ast_vertex*
ast_vertex::tau_critical()
{
	ASSUME_TYPE_AND_CLONE_ROOT_AND_APPLY_TRANS_TO_FIRST(VT_CRITICAL, tau);
}

ast_vertex*
ast_vertex::tau_assert()
{
	ast_vertex* S=NULL;

	S=createS();
	add_child(S, clone());
	ASSUME_TYPE(S, VT_S);
	return S;
}

