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
#include "symbol_table.h"
#include "AST.h"


ast_vertex* 
ast_vertex::sigma()
{
	ast_vertex* v=this;
	ast_vertex* pendant=NULL;
	ast_vertex* D=NULL;
	ast_vertex* S=NULL;
	switch(v->type) {
		case VT_SPL_CODE : 
		{
			ast_vertex* result_from_sigmaGlobalDecl = FIRST->sigma(); 
			assert(result_from_sigmaGlobalDecl->type==VT_SEQ_DECLARATIONS);
			result_from_sigmaGlobalDecl->type=VT_GLOBAL_DECLARATIONS;
			result_from_sigmaGlobalDecl->unparser=&unparser_global_declarations;
			global_seq_decls=result_from_sigmaGlobalDecl;
			create_thread_local_stacks(global_seq_decls, adjoint);

			assert(global_seq_decls);
			ast_vertex* result_from_sigmaParallelRegion = SECOND->sigma();
			assert(result_from_sigmaParallelRegion->type==VT_PARALLEL_REGION || result_from_sigmaParallelRegion->type==VT_PARALLEL_FOR_REGION );
			ast_vertex* spl_code = new ast_vertex(0, VT_SPL_CODE, "");
			add_child(spl_code, result_from_sigmaGlobalDecl);
			add_child(spl_code, result_from_sigmaParallelRegion);
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
			return sigma_seq_decl();
		case VT_S:
			return sigma_seq_of_stmts(); break;
		case VT_SPL_cond_if:
		case VT_SPL_cond_while:
			return sigma_conditional(); break;
		case VT_SPL_STACKfpush:
		case VT_SPL_STACKipush:
		case VT_SPL_STACKcpush:
		case VT_SPL_STACKfpop:
		case VT_SPL_STACKipop:
		case VT_SPL_STACKcpop:
			return sigma_stack(); break;
		case VT_SPL_int_assign:
		case VT_SPL_float_assign:
		case VT_SPL_float_plus_assign:
			return sigma_assign(); break;
		case VT_OMP_FOR: 
			return sigma_omp_for(); break;
		case VT_BARRIER:  
			return sigma_barrier(); break;
		case VT_DUMMY: 
			return sigma_dummy(); break;
		case VT_ATOMIC:  
			return sigma_atomic(); break;
		case VT_EXCLUSIVE_READ_FAILURE:  
			return FIRSTOF(this)->sigma_assign(); break;
		case VT_MASTER:  
			return sigma_master(); break;
		case VT_CRITICAL:  
			return sigma_critical(); break;
		case VT_ASSERT: 
			return sigma_assert(); break;
		default:
			UNKNOWN_TYPE(v);
	}
	assert(D->type==VT_SEQ_DECLARATIONS);	
	assert(S->type==VT_S);	
	assert( ast.code_to_insert_before_forward_section->children.size()==0 );
	assert( ast.code_to_insert_before_reverse_section->children.size()==0 );
	/* Now create the calls as defined in the rule for sigma(P) */
	ast_vertex* result_from_sigmaD = D->sigma(); assert(result_from_sigmaD->type==VT_SEQ_DECLARATIONS);
	ast_vertex* result_from_sigmaS = S->sigma(); assert(result_from_sigmaS->type==VT_S);
	// Build loop for reverse section
	ast_vertex* testexpr=new ast_vertex(0, VT_SPL_not, "");  
	add_child( testexpr, new ast_vertex(0, VT_SPL_STACKcempty, adj_prefix) );
	ast_vertex* result_from_rhoS = S->rho();  assert(result_from_rhoS->type==VT_S);
	ast_vertex* loop = new ast_vertex(0, VT_SPL_cond_while, ""); 
	add_child(loop, testexpr); 
	add_child(loop, result_from_rhoS);
	ast.adj_reverse_section_root = loop;

	/* Build together P = D S  */
	D=result_from_sigmaD;
	// Collect parts for the final S
	S=createS();
	ast.adj_forward_section_root = S;

	if( ast.code_to_insert_before_forward_section->children.size()>0 ) { S->appendS( ast.code_to_insert_before_forward_section ); }
	S->appendS( result_from_sigmaS );

	// Add code that should be inserted before the reverse section
	if( option_mem_statistic ) { add_child( S, new ast_vertex(0, VT_BEFORE_REVERSE_MODE_HOOK, "") ); }
	if( ast.code_to_insert_before_reverse_section->children.size()>0 ) { S->appendS( ast.code_to_insert_before_reverse_section ); }
	// Add reverse section to S
	add_child(S, loop);

	// Add D and S as childs for pendant for parallel region.
	add_child(pendant, D);
	add_child(pendant, S);
	return pendant;
}

ast_vertex* 
ast_vertex::sigma_seq_of_stmts()
{
	assert(type==VT_S);
	ast_vertex* S;
	list<ast_vertex*> seqs;
	set_slc(); // Despite the fact if the SLC flag is set or not we test this property again.

	if(!slc) { // If sequence is not SLC then divide it into partitions of SLC and conditional statements.
		partition_seq_into_slc_and_cfstmts(seqs);
		// Now we apply source transformation sigma to all sequences and conditional statements in SEQS.
		S=createS();
		for(list<ast_vertex*>::const_iterator it=seqs.begin();it!=seqs.end();it++) {
			ast_vertex* resultSigma=(*it)->sigma() ;  ASSUME_TYPE(resultSigma, VT_S);
			S->appendS( resultSigma ); 
		}
	}
	else { // S is SLC sequence
		ostringstream oss;
		oss << (*children.begin())->id;
		S=createS();
		// Create STACKcpush(const)
		ast_vertex* label=new ast_vertex( 0, VT_SPL_int_const, oss.str() );
		ast_vertex* STACKcpush=new ast_vertex(0, VT_SPL_STACKcpush, adj_prefix);
		add_child(STACKcpush, label);
		add_child(S, STACKcpush);
		// Add sigma(s1), ..., sigma(sq)
		for(list<ast_vertex*>::const_iterator it=children.begin();it!=children.end();it++) {
			ast_vertex* resultSigma=(*it)->sigma() ;  ASSUME_TYPE(resultSigma, VT_S);
			S->appendS( resultSigma ); 
		}
	}
	return S;
}

ast_vertex*
ast_vertex::sigma_omp_for()
{
	ast_vertex* v=this;
	ast_vertex* loop_init=FIRST;
	ast_vertex* loop=SECOND;
	ast_vertex* S=NULL;
	ast_vertex* sigma_omp_for=NULL;
	ast_vertex* sigma_loop=NULL;
	ast_vertex* sigma_loop_init=NULL;

	ASSUME_TYPE(FIRST, VT_SPL_int_assign);
	ASSUME_TYPE(SECOND, VT_SPL_cond_while);
	S=createS();
	sigma_omp_for = new ast_vertex(0, VT_OMP_FOR, "");
	sigma_loop_init = loop_init->sigma() ;
	sigma_loop      = loop->sigma() ;

	add_child( sigma_omp_for, (*sigma_loop_init->children.begin()) );
	add_child( sigma_omp_for, sigma_loop );
	add_child( S, sigma_omp_for );

	return S;
}

ast_vertex*
ast_vertex::sigma_stack()
{
	ast_vertex* S=NULL;
	switch(type) {
		case VT_SPL_STACKfpush:
		case VT_SPL_STACKipush:
		case VT_SPL_STACKcpush:
		case VT_SPL_STACKfpop:
		case VT_SPL_STACKipop:
		case VT_SPL_STACKcpop:
			S=createS();
			add_child( S, this->clone() );
			break;
		default:
			assert(0);
	}
	assert(S->type==VT_S);
	return S;
}

ast_vertex* 
ast_vertex::sigma_assign()
{
	ast_vertex* lhs=NULL;
	ast_vertex* STACKpush=NULL;

	switch(type) {
		case VT_SPL_int_assign:
			STACKpush = new ast_vertex(0, VT_SPL_STACKipush, adj_prefix);
			break;
		case VT_SPL_float_assign:
		case VT_SPL_float_plus_assign:
			STACKpush = new ast_vertex(0, VT_SPL_STACKfpush, adj_prefix);
			break;
		default:
			assert(0);
	}
	lhs = (*children.begin())->clone();
	add_child(STACKpush, lhs);
	ast_vertex* S=createS();
	add_child( S, STACKpush );
	add_child( S, this->clone() ); // Assignment itself occurs also in the output code.
	assert(S->type==VT_S);
	return S;
}

ast_vertex* 
ast_vertex::sigma_conditional()
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
	subseq = (*++children.begin())->sigma(); assert(subseq->type==VT_S);
	add_child( conditional_stmt, conditional_testexpr );
	add_child( conditional_stmt, subseq );
	add_child( S, conditional_stmt );
	assert(S->type==VT_S);
	return S;
}

ast_vertex* 
ast_vertex::sigma_seq_decl()
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
		D->appendD( (*it)->sigma_decl() );
	}
	assert(D->type==VT_SEQ_DECLARATIONS);
	local_seq_decls=D;
	return D;
}

ast_vertex* 
ast_vertex::sigma_decl()
{
	ast_vertex* D = createD();
	ast_vertex* decl;
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
			(*decl2->children.begin())->str = adj_prefix + (*decl2->children.begin())->str;
			add_child(D, decl2);
			bool var_is_not_intermediate=false;
			bool var_is_a_new_derivative_component=true;
			sym_tbl.insert(decl2, sym->shared, var_is_not_intermediate, var_is_a_new_derivative_component);
			break;
		}
	case VT_FLOAT_DECLARATION: 
		{
			symbol *sym=sym_tbl.get_sym(get_lhs_name());
			decl = clone();
			ast_vertex* decl2 = new ast_vertex(0, VT_FLOAT_DECLARATION, ""); 
			ast_vertex* decl2floatass = new ast_vertex(0, VT_SPL_float_assign, ""); 
			add_child(decl2, decl2floatass);
			ast_vertex* lhs = new ast_vertex(0, VT_SPL_identifier, adj_prefix + decl->get_lhs_name());
			add_child(decl2floatass, lhs);
			ast_vertex* rhs = new ast_vertex(0, VT_SPL_float_const, "0.");
			add_child(decl2floatass, rhs);

			add_child(D, decl);
			add_child(D, decl2);
			bool var_is_not_intermediate=false;
			bool var_is_a_new_derivative_component=true;
			sym_tbl.insert(decl2, sym->shared, var_is_not_intermediate, var_is_a_new_derivative_component);
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
ast_vertex::sigma_barrier()
{
	ast_vertex* S=NULL;
	ASSUME_TYPE(this, VT_BARRIER);
	S=createS();
	add_child(S, clone());
	ASSUME_TYPE(S, VT_S);
	return S;
}

ast_vertex*
ast_vertex::sigma_dummy()
{
	//ast_vertex* v=this;
	ast_vertex* S=NULL;
	S=createS();
	add_child(S, clone());
	ASSUME_TYPE(S, VT_S);
	return S;
}

ast_vertex*
ast_vertex::sigma_master()
{
	ASSUME_TYPE_AND_CLONE_ROOT_AND_APPLY_TRANS_TO_FIRST(VT_MASTER, sigma);
}


ast_vertex*
ast_vertex::sigma_atomic()
{ 
	ast_vertex* S=NULL;
	ASSUME_TYPE(this, VT_ATOMIC);
	S=createS();
	ast_vertex *critical_region = new ast_vertex(line_number, VT_CRITICAL, "");
	ast_vertex *critical_region_subseq = createS();
	add_child(critical_region, critical_region_subseq);
	add_child(S, critical_region);

	// create atomic_flag variable
	ast_vertex* atomic_flag_decl = new ast_vertex(line_number, VT_INT_DECLARATION, ""); 
	ast_vertex* atomic_flag_decl_assign = new ast_vertex(line_number, VT_SPL_int_assign, ""); 
	add_child(atomic_flag_decl, atomic_flag_decl_assign);
	ostringstream oss; oss << atomic_flag_prefix << this->id ;
	ast.atomic_flag_names[id] = oss.str() ;
	ast_vertex* lhs = new ast_vertex(line_number, VT_SPL_identifier, oss.str() );
	add_child(atomic_flag_decl_assign, lhs);
	ast_vertex* rhs = new ast_vertex(line_number, VT_SPL_int_const, "0");
	add_child(atomic_flag_decl_assign, rhs);
	add_child(global_seq_decls, atomic_flag_decl);
	sym_tbl.insert(atomic_flag_decl, true);

	// create atomic_storage variable
	ast_vertex* atomic_storage_decl = new ast_vertex(line_number, VT_FLOAT_DECLARATION, ""); 
	oss.str(""); oss << atomic_storage_prefix << this->id ;
	ast_vertex* identifier = new ast_vertex(line_number, VT_SPL_identifier, oss.str() );
	add_child(atomic_storage_decl, identifier);
	ast.atomic_storage_names[id] = oss.str() ;
	add_child(global_seq_decls, atomic_storage_decl);
	sym_tbl.insert(atomic_storage_decl, true);

	// Establish critical region 

	ast_vertex* if_stmt = new ast_vertex(0, VT_SPL_cond_if, "");
	ast_vertex* if_stmt_test = new ast_vertex(0, VT_SPL_eq, ""); 
	ast_vertex* zero = new ast_vertex(0, VT_SPL_int_const, "0");
	add_child(if_stmt_test, lhs);
	add_child(if_stmt_test, zero);
	ast_vertex* if_stmt_S = createS(); 
	add_child(if_stmt, if_stmt_test);
	add_child(if_stmt, if_stmt_S);

	ast_vertex* atomic_flag_assign = new ast_vertex(0, VT_SPL_int_assign, "");
	ast_vertex* one = new ast_vertex(0, VT_SPL_int_const, "1");
	add_child( atomic_flag_assign, lhs );
	add_child( atomic_flag_assign, one );

	ast_vertex* atomic_storage_assign = new ast_vertex(0, VT_SPL_float_assign, "");
	add_child(atomic_storage_assign, identifier);
	add_child( atomic_storage_assign, FIRSTOF(FIRSTOF(this)) );

	ast_vertex *incremental_assign = FIRSTOF(this);  ASSUME_TYPE(incremental_assign, VT_SPL_float_plus_assign);
	add_child( if_stmt_S, atomic_flag_assign );
	add_child( if_stmt_S, atomic_storage_assign );
	add_child( critical_region_subseq, if_stmt );
	add_child( critical_region_subseq, incremental_assign->clone() );

	// Add the initialization of the flag before entering forward section 
	ast_vertex *master_construct = new ast_vertex(0, VT_MASTER, "");
	ast_vertex *master_construct_S = createS();
	add_child( master_construct, master_construct_S );
	add_child( master_construct_S, atomic_flag_decl_assign->clone() );
	add_child( ast.code_to_insert_before_forward_section, master_construct );
	add_child( ast.code_to_insert_before_forward_section, new ast_vertex(line_number, VT_BARRIER, "") );

	// Add a barrier between forward and reverse section
	add_child( ast.code_to_insert_before_reverse_section, new ast_vertex(line_number, VT_BARRIER, "") );

	ASSUME_TYPE(S, VT_S);
	return S;
}


ast_vertex*
ast_vertex::sigma_critical()
{
	ast_vertex* v=this; 
	ast_vertex* S=NULL; 
	ASSUME_TYPE(this, VT_CRITICAL); 
	S=createS(); 
	add_child( S, createSTACKcpush(id, adj_prefix) );
	ast_vertex* new_critical_region=clone_node(); 

	ast_vertex* critical_counter_decl = new ast_vertex(line_number, VT_INT_DECLARATION, ""); 
	ast_vertex* critical_counter_decl_assign = new ast_vertex(line_number, VT_SPL_int_assign, ""); 
	add_child(critical_counter_decl, critical_counter_decl_assign);
	ostringstream oss; oss << critical_counter_prefix << new_critical_region->id ;
	ast.critical_counter_names[id] = make_pair( oss.str(), new_critical_region->id+1 );
	ast_vertex* lhs = new ast_vertex(line_number, VT_SPL_identifier, oss.str() );
	add_child(critical_counter_decl_assign, lhs);
	oss.str(""); oss << new_critical_region->id+1 ;
	ast_vertex* rhs = new ast_vertex(line_number, VT_SPL_int_const, oss.str());
	add_child(critical_counter_decl_assign, rhs);
	add_child(global_seq_decls, critical_counter_decl);
	sym_tbl.insert(critical_counter_decl, true);

	ast_vertex* result_trans=FIRST->sigma(); ASSUME_TYPE(result_trans, VT_S); 
	// Add STACKc.push(a_l)
	add_child(result_trans, createSTACKcpush(ast.critical_counter_names[id].first, adj_prefix));
	// Add a_l = a_l + 1
	lhs = new ast_vertex(line_number, VT_SPL_identifier, ast.critical_counter_names[id].first );
	ast_vertex* expr_add = new ast_vertex(line_number, VT_SPL_int_add, ""); 
	add_child( expr_add, new ast_vertex(line_number, VT_SPL_identifier, ast.critical_counter_names[id].first ) );
	add_child( expr_add, new ast_vertex(line_number, VT_SPL_int_const, "1") );
	rhs = expr_add;
	ast_vertex* increment_for_a_l = new ast_vertex(line_number, VT_SPL_int_assign, ""); 
	add_child( increment_for_a_l, lhs);
	add_child( increment_for_a_l, rhs);
	add_child( result_trans, increment_for_a_l);

	add_child(new_critical_region, result_trans);  
	add_child(S, new_critical_region); 
	add_child( S, createSTACKcpush(id, adj_prefix) );

	// Add a barrier between forward and reverse section
	add_child( ast.code_to_insert_before_reverse_section, new ast_vertex(line_number, VT_BARRIER, "") );

	ASSUME_TYPE(S, VT_S);
	return S;
}


ast_vertex*
ast_vertex::sigma_assert()
{
	ast_vertex* S=NULL;

	S=createS();
	add_child(S, clone());
	ASSUME_TYPE(S, VT_S);
	return S;
}


