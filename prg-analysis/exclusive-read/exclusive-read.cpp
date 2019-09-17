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
#include <iomanip>
#include <utility>
#include <map>
#include <limits>
#include <cassert>
#include <cstdlib>

#include "exclusive-read.h"

using namespace std;

map< ast_vertex* , bool >  exclusive_read::is_memory_read_exclusive;

const string RESULT_FILE = "exclusive-read-results";

vector< ast_vertex* > exclusive_read::equations;

ast_vertex *BOTTOM           = NULL;
ast_vertex *TOP              = NULL;
ast_vertex *ZERO             = NULL;
ast_vertex *MINUS_INFINITY   = NULL;
ast_vertex *PLUS_INFINITY    = NULL;
ast_vertex *THREAD_ID        = NULL;
ast_vertex *NUM_OF_THREADS   = NULL;
	

void 
exclusive_read::entry_point_for_prg_analysis() 
{
	BOTTOM           = new ast_vertex(0, VT_SPL_int_const, "BOTTOM");
	TOP              = new ast_vertex(0, VT_SPL_int_const, "TOP");
	ZERO             = new ast_vertex(0, VT_SPL_int_const, "0");
	MINUS_INFINITY   = new ast_vertex(0, VT_SPL_int_const, "-infinity");
	PLUS_INFINITY    = new ast_vertex(0, VT_SPL_int_const, "infinity");
	THREAD_ID        = new ast_vertex(0, VT_SPL_int_const, "THREAD_ID");
	NUM_OF_THREADS   = new ast_vertex(0, VT_SPL_int_const, "NUM_OF_THREADS");

	// Init cfg_entry and cfg_exit
	ast.cfg_entry->phi = &phi;	
	ast.cfg_exit->phi  = &phi;	

	// Initializing all nodes with phi and AI data
	init_prg_analysis(ast.get_root());

	uint fixpoint_reached=0;
	// Perform program analysis 
	int counter=0;
	while( !fixpoint_reached ) {
		counter++;
		fixpoint_reached=1;
		for(unsigned i=0;i<equations.size();i++) {
			exclusive_read *ai = (exclusive_read*)equations[i]->AI;
			assert(ai->astv==equations[i]);
			assert(equations[i]->phi);

			exclusive_read old_ai( equations[i], NULL ) ;  // Last result of equation.
			exclusive_read new_ai ;
			new_ai.astv = equations[i];

			for(set<ast_vertex*>::const_iterator it=ai->data_flow_predecessors.begin();it!=ai->data_flow_predecessors.end();it++) {
				assert(((exclusive_read*)(*it)->AI)->astv==*it);
				exclusive_read tmp_ai(*it, equations[i]);
				(*it)->phi(&tmp_ai); 
				operator_join(new_ai, tmp_ai, new_ai, 0);
			}
			//ensure_monotoncity(old_ai, new_ai);
			#if DEBUG
			cerr << "Use WIDENING to obtain next result ... \n";
			#endif 
			operator_join(old_ai, new_ai, *ai, 1);
			bool sets_equal = operator_equal(old_ai, *ai);
			if( !sets_equal ) {
				#if DEBUG
				cerr << "new ai is different to the old.";
				#endif 
				//ai->D = new_ai.D; 
			}
			fixpoint_reached &= sets_equal;
			#if DEBUG
			cerr << "DEBUG: fixpoint in " << equations[i]->get_id() << " reached: "<< sets_equal<< "\n";
			cerr << AI_to_string(equations[i]); 
			#endif
		}
		print_results(1);

		#if DEBUG > 1
		if(!fixpoint_reached ) 
			cout << "DEBUG: HIT KEY TO CONTINUE for next fixpoint iteration ... \n";
		else 
			cout << " fixpointer iteration is finished\n";
		char c; cin >> c;
		#endif

	}
	fixpoint_reached = counter;
#if NARROWING
	// NARROWING.
	uint narrowing_fixpoint_reached=0;
	counter=0;
	while( !narrowing_fixpoint_reached && counter<10 ) {
		counter++;
		narrowing_fixpoint_reached=1;
		for(unsigned i=0;i<equations.size();i++) {
			exclusive_read *ai = (exclusive_read*)equations[i]->AI;
			assert(ai->astv==equations[i]);
			assert(equations[i]->phi);

			exclusive_read old_ai( equations[i], NULL ) ;  // Last result of equation.
			exclusive_read new_ai ;
			new_ai.astv = equations[i];
			for(set<ast_vertex*>::const_iterator it=ai->data_flow_predecessors.begin();it!=ai->data_flow_predecessors.end();it++) {
				assert(((exclusive_read*)(*it)->AI)->astv==*it);
				exclusive_read tmp_ai(*it, equations[i]);
				(*it)->phi(&tmp_ai); 
				operator_join(new_ai, tmp_ai, new_ai, 0);
			}

			bool sets_equal = operator_equal(old_ai, new_ai);
			ai->D = new_ai.D; 
			narrowing_fixpoint_reached &= sets_equal;
			#if DEBUG
			{ cerr << AI_to_string(equations[i]); }
			#endif
		}

		print_results(1);

		#if DEBUG > 1
		if(!narrowing_fixpoint_reached ) 
			cout << "DEBUG: HIT KEY TO CONTINUE NARROWING\n";
		else 
			cout << " narrowhing has been finished\n";
		char c; cin >> c;
		#endif
	}
	narrowing_fixpoint_reached = counter;
	//cerr << "Fixpoint iteration is finished. We needed " << fixpoint_reached << " rounds for reaching the fixpoint and afterwards " << narrowing_fixpoint_reached<< " rounds of narrowing.\n";
#endif
	// Now, we look for SPL_array_indexes and their intervals
	for(uint i=0;i<equations.size();i++) {
		list< ast_vertex* > l;
		if(    equations[i]->get_type()==VT_SPL_float_assign 
			|| equations[i]->get_type()==VT_SPL_float_plus_assign ) {
			collect_read_accesses(SECONDOF(equations[i]), l);
			for(list<ast_vertex*>::const_iterator it=l.begin();it!=l.end();it++) {
				ast_vertex* v=*it;
				assert( sym_tbl.is_float( v->get_str()) );
				if( v->get_children().size()==1 && FIRSTOF(v)->get_type()==VT_SPL_array_index ) {
					if( sym_tbl.is_shared(v->get_str()) ) // Mem access is base+offset where base pointer is shared among threads.
						is_memory_read_exclusive[v] = is_array_index_thread_unique(FIRSTOF(FIRSTOF(v)), ((exclusive_read*)equations[i]->AI)->D);
					else                                  // Mem access is base+offset where base pointer is private for each threads.
						is_memory_read_exclusive[v] = true; 
				}
				else {
					if( sym_tbl.is_shared(v->get_str()) ) 
						is_memory_read_exclusive[v] = false;
					else
						is_memory_read_exclusive[v] = true;
					
				}

			}
		}
	}

	// Print overall results to file.
	print_results();

	// Finish exclusive read analysis
	// Set all phi and AI values back to zero
	for(unsigned i=0;i<equations.size();i++) {
		if(equations[i]->AI) free(equations[i]->AI);
		equations[i]->AI=NULL;
	}
	delete_prg_analysis(ast.get_root());
}


void
exclusive_read::phi(void *argv) 
{
	exclusive_read *ai = (exclusive_read*) argv;
	string lhs_name;

	// Parameter ai is meant as temporary data where the astv data component is associated with the ast vertex 
	// that should be processed by phi function.
	// Each ast vertex has a phi and AI component. 
	// The associated AI information of the ast vertex should NOT be changed here in phi.
	assert(ai->astv);
	#if DEBUG
	cerr << "phi of cfg node " << ai->astv->get_id() << " type(" << ai->astv->vertex_type_2_string() << ")" << endl;
	#endif 

	switch( ai->astv->get_type() ) {
		case VT_UINT_DECLARATION:
		case VT_INT_DECLARATION:
		{
			string identifier = ai->astv->get_lhs_name();
			if( sym_tbl.is_shared(identifier) )  { // Shared variables are assumed to be defined outside
				ASSUME_TYPE( FIRSTOF(ai->astv), VT_SPL_identifier);
				ai->D[ ai->astv ] = make_pair(FIRSTOF(ai->astv), FIRSTOF(ai->astv)) ; 
			}
			else  { // Private variables do only have value when they are defined per initialization.
				if( FIRSTOF(ai->astv)->get_type()==VT_SPL_int_assign ) {
					ast_vertex *rhs=SECONDOF( FIRSTOF(ai->astv) ); ASSUME_TYPE(rhs, VT_SPL_int_const);
					ai->D[ ai->astv ] = make_pair(rhs, rhs); 
				}
				else
					ai->D[ ai->astv ] = ER_DOMAIN_TYPE_BOTTOM; 
			}
			break;
		}
		case VT_SPL_int_assign:
		{
			ast_vertex *lhs = FIRSTOF(ai->astv);
			ASSUME_TYPE(lhs, VT_SPL_identifier);
			lhs_name = lhs->get_str();
			ast_vertex *decl = sym_tbl.get_decl(lhs_name);
			ASSUME_TYPES( decl, VT_INT_DECLARATION, VT_UINT_DECLARATION);
			ai->D[ decl ] = eval( ai->astv, SECONDOF(ai->astv) ); 

			break;
		}
		case VT_SPL_cond_if:
		case VT_SPL_cond_while:
		{
			if( ast.test_expr_to_true_false_successors.count( ai->astv )>0 ) {
				#if DEBUG
				cerr << "This node has assertions that are going to influence the analysis: " << ai->astv->get_id() << endl;
				#endif 
				ast_vertex* true_branch  = ast.test_expr_to_true_false_successors[ai->astv].first;
				ast_vertex* false_branch = ast.test_expr_to_true_false_successors[ai->astv].second;
				if( true_branch==ai->target ) {
					/*
					   The true branch is only fulfilled if the test expression is true. 
					   In case of test (i<ub) we can look what value range ub has and if it has a upper bound different to infinity than we can take this upper boundary as
					   upper boundary for i as well.
					   */
					// Set boundaries according to the branch or loop body 
					#if DFA_OF_CONDITIONAL_BRANCHES
					ER_FOR_EACH_it_IN_DOMAIN(ai->D) {                 // For comments we take the test expression (i<ub) as an example.
						ast_vertex* current_decl=it->first; 
						string current_var_name=current_decl->get_lhs_name(); 
						ER_DOMAIN_TYPE interval_of_var=it->second;
						#if DEBUG
						cerr << "Current variable is: " << current_var_name << " with interval " <<  value_to_string(interval_of_var) << endl;
						#endif 
						ast_vertex* test_expression = FIRSTOF(ai->astv);
						adjust_AI_depending_on_assertion(*ai, current_decl, interval_of_var, test_expression);

					}
					#endif

					// Here, we insert the current test expression of the loop/if statement to list of assertions.
					// So if (j<n) is the test expression then insert (j<n) to the list of assertions since the current target is
					// ths node that is traversed in case that the condition is true.
					bool found=false;	
					for(set< ast_vertex* >::const_iterator it=((exclusive_read*)ai->target->AI)->list_of_assertions.begin();it!=((exclusive_read*)ai->target->AI)->list_of_assertions.end();it++) {
						if(     FIRSTOF(ai->astv)->get_type()==(*it)->get_type() 
                            &&  FIRSTOF(FIRSTOF(ai->astv))->get_str()==FIRSTOF(*it)->get_str()
                            &&  SECONDOF(FIRSTOF(ai->astv))->get_str()==SECONDOF(*it)->get_str()
						  ) found=true;
					}
					if(!found) ((exclusive_read*)ai->target->AI)->list_of_assertions.insert( FIRSTOF(ai->astv)->clone() );
				}
				else if( false_branch==ai->target ) { 
					// This code inserts the complement of the current test expression to the list of assertions.
					// Hence, if (j<n) is the test expression, we insert (j>=n)
					ast_vertex* complement_of_test_expression = FIRSTOF(ai->astv)->get_complement_test();
					assert( complement_of_test_expression );
					bool found=false;	
					for(set< ast_vertex* >::const_iterator it=((exclusive_read*)ai->target->AI)->list_of_assertions.begin();it!=((exclusive_read*)ai->target->AI)->list_of_assertions.end();it++) {
						if(     complement_of_test_expression->get_type()==(*it)->get_type()   // is type the same?
                            &&  FIRSTOF(complement_of_test_expression)->get_str()==FIRSTOF(*it)->get_str()  // is lhs of test the same?
                            &&  SECONDOF(complement_of_test_expression)->get_str()==SECONDOF(*it)->get_str()  // is the rhs of test the same?
						  ) found=true;
					}
					if(!found) ((exclusive_read*)ai->target->AI)->list_of_assertions.insert( complement_of_test_expression );
				}
			}
			break;
		}
		default:
			break;
	}

	// Transfer the assertions to the target.
	for(set< ast_vertex* >::const_iterator it=((exclusive_read*)ai->astv->AI)->list_of_assertions.begin();
		it!=((exclusive_read*)ai->astv->AI)->list_of_assertions.end();it++) {
		// Was current statement an assignment? If so then test if the lhs impacts the assertions.
		if( lhs_name!="" ) {
			ast_vertex* assertion = *it;
			ast_vertex* assertion_lhs = FIRSTOF(assertion);
			ast_vertex* assertion_rhs = SECONDOF(assertion);
			if( assertion_lhs->get_str()!=lhs_name && assertion_rhs->get_str()!=lhs_name )
				((exclusive_read*)ai->target->AI)->list_of_assertions.insert(*it);
		}
		else 
			((exclusive_read*)ai->target->AI)->list_of_assertions.insert(*it);
	}

}

ER_DOMAIN_TYPE 
exclusive_read::eval(ast_vertex *assign, ast_vertex *v) 
{
	ER_DOMAIN_TYPE result;
	ASSUME_TYPE(assign, VT_SPL_int_assign);
	exclusive_read *ai = (exclusive_read*) assign->AI; assert(assign->AI);
	switch(v->get_type()) {
		case VT_OMP_RUNTIME_ROUTINE:
			if( v->get_str()=="omp_get_thread_num()" ) 
				result = make_pair( THREAD_ID, THREAD_ID );
			else if( v->get_str()=="omp_get_num_threads()" )
				result = make_pair( NUM_OF_THREADS, NUM_OF_THREADS );
			else 
				assert(0); // Unsuported runtime function
			break;
		case VT_SPL_int_const:
		{
			result = make_pair( v, v );
			break;
		}
		case VT_SPL_identifier:
		{
			// In case of an identifier, we have to look for the AI of this identifier. 
			const string identifier = v->get_str();
			ast_vertex *decl = sym_tbl.get_decl(identifier);
			if( ai->D.count(decl)>0 ) { // In case that there is AI about this identifier, we take this info.
				ASSUME_TYPES( decl, VT_INT_DECLARATION, VT_UINT_DECLARATION);
				result = ai->D[decl];
			}
			else  // otherwise, we have to take bottom as information.
				result = ER_DOMAIN_TYPE_BOTTOM;
			break;
		}
		case VT_SPL_int_add:
		{
			ER_DOMAIN_TYPE l = eval(assign, FIRSTOF(v));
			ER_DOMAIN_TYPE r = eval(assign, SECONDOF(v));
			result = operator_add(l, r);
			#if DEBUG
				cerr << "eval: case addition in " << v->get_id() << " : \n"; 
				cerr << "l: " << value_to_string(l) << "\n";
				cerr << "r: " << value_to_string(r) << "\n";
				cerr << "result: " << value_to_string(result) << "\n";
			#endif
			break;
		}
		case VT_SPL_int_sub:
		{
			ER_DOMAIN_TYPE l = eval(assign, FIRSTOF(v));
			ER_DOMAIN_TYPE r = eval(assign, SECONDOF(v));
			result = operator_sub(l, r);
			break;
		}
		case VT_SPL_int_mul:
		{
			ER_DOMAIN_TYPE l = eval(assign, FIRSTOF(v));
			ER_DOMAIN_TYPE r = eval(assign, SECONDOF(v));
			result = operator_mul(l, r);
			#if DEBUG
				cerr << "mul l: " << value_to_string(l) << endl;
				cerr << "mul r: " << value_to_string(r) << endl;
				cerr << "mul result: " << value_to_string(result) << endl;
			#endif
			break;
		}
		case VT_SPL_int_div:
		{
			ER_DOMAIN_TYPE l = eval(assign, FIRSTOF(v));
			ER_DOMAIN_TYPE r = eval(assign, SECONDOF(v));
			result = operator_div(l, r);
			#if DEBUG
				cerr << "div l: " << value_to_string(l) << endl;
				cerr << "div r: " << value_to_string(r) << endl;
				cerr << "div result: " << value_to_string(result) << endl;
			#endif
			break;
		}
		case VT_SPL_int_mudolo:
			assert(0); // This is not implmented yet!
			break;
		case VT_SPL_STACKitop:
		case VT_SPL_STACKctop:
			result = ER_DOMAIN_TYPE_TOP; 
			break;
		case VT_SPL_expr_in_brackets:
			result = eval(assign, FIRST);
			break;
		default:
			cerr << v->vertex_type_2_string() << endl;
			assert(0); // This should not happen!
			result = ER_DOMAIN_TYPE_TOP; 
			break;
	}
	return result;
}

void
exclusive_read::ensure_monotoncity(exclusive_read& l, exclusive_read& r) 
{
	ER_FOR_EACH_it_IN_DOMAIN(l.D) {
		ast_vertex *d=it->first; ER_DOMAIN_TYPE v=it->second;
		if( r.D.count(d)>0 ) { // If true then d is contained in both sets (case one)
			if( is_interval_top(v) ) {
				r.D[d] = ER_DOMAIN_TYPE_TOP;
			}
		}
	}
}

bool 
exclusive_read::operator_equal(exclusive_read& l, exclusive_read& r) 
{
	#if DEBUG
	cerr << "operator_equal(exclusive_read& l, exclusive_read& r)\n";
	#endif 
	assert(l.astv == r.astv);
	if( l.D.size()==0  &&  r.D.size()==0 ) {
		#if DEBUG
		cerr << "DEBUG: size from both sets is zero.\n";
		#endif 
		return true;
	}
	if( l.D.size()!=r.D.size() ) {
		#if DEBUG
		cerr << "DEBUG: size from both sets is different.\n";
		#endif 
		return false;
	}
	ER_FOR_EACH_it_IN_DOMAIN(l.D) {
		// We cannot compare ast_vertex pointers. Instead we have to compare the DAGs where
		// the pointers point to.
		ast_vertex *current_var=it->first; 
		ER_DOMAIN_TYPE interval_of_var_l=it->second;
		if( r.D.count(current_var)>0 ) { // Is information about the current VAR available?
			ER_DOMAIN_TYPE interval_of_var_r=r.D[current_var];
			er_basic_type a = interval_of_var_l.first;
			er_basic_type b = interval_of_var_l.second;
			er_basic_type c = interval_of_var_r.first;
			er_basic_type d = interval_of_var_r.second;
			if( !::operator_equal(a,c) || !::operator_equal(b,d) )
				return false;
		}
		else 
			return false;
	}
	#if DEBUG
	cerr << "DEBUG: ai sets are both the same.\n";
	#endif 
	return true;
}

// Join two abstract informations AI1 and AI2 which are vectors with n components from domain D.
inline void
exclusive_read::operator_join(exclusive_read& l, // l is the current AI in target node
		                      exclusive_read& r, // r is the new AI info for the target node
							  exclusive_read& result,
							  bool widening) 
{ 
  #if DEBUG
  if(r.astv->get_type()==VT_SPL_cond_if || r.astv->get_type()==VT_SPL_cond_while ) { 
	  ast_vertex* true_branch = ast.test_expr_to_true_false_successors[r.astv].first;
	  ast_vertex* false_branch = ast.test_expr_to_true_false_successors[r.astv].second;
	  cerr << "true branch: " << true_branch->get_id() << endl;
	  cerr << "false branch: " << false_branch->get_id() << endl;
  }
  #endif 

  #if DEBUG
  if(widening)
  	cerr << "DEBUG: operator_join with widening " << l.astv->get_id() << "  and  "  << r.astv->get_id() << "\n";
  else
  	cerr << "DEBUG: operator_join " << l.astv->get_id() << "  and  "  << r.astv->get_id() << "\n";
  #endif
  // Distinguish the cases
  // 1. element is in both sets
  // 2. element is only contained in L
  // 3. element is only contained in R
  // 

  // The first loop is for cases one and two.
  ER_FOR_EACH_it_IN_DOMAIN(l.D) { 
		ast_vertex *d=it->first; ER_DOMAIN_TYPE v=it->second;
		// d is the definition statement of the current variable.
		// v is the current interval for this variable.
		ASSUME_TYPES( d, VT_INT_DECLARATION, VT_UINT_DECLARATION);
		if( r.D.count(d)>0 ) { // If true then d is contained in both sets (case one)
			// The first version used the common join operation with risk
			// of no termination. Therefore, use widening operator as 
			// explained in course, see lecture 8 slide 12.

			// In case that one interval is TOP we return TOP as result.
			if( is_interval_top(v) || is_interval_top(r.D[d]) ) {
				result.D[d] = ER_DOMAIN_TYPE_TOP;
			}
			else {
				result.D[d] = operator_join(v, r.D[d]);
			}
			#if DEBUG
			cerr << "result for " << d->get_lhs_name() << ": " << value_to_string(result.D[d]) << endl;
			#endif
		}
		else  {  // Otherwise both sets do not have the same content (case two)
			result.D[d] = v;
		}
  }
  // After join we use widening 
  if( widening ) {
	  ER_FOR_EACH_it_IN_DOMAIN(l.D) { 
		  ast_vertex *d=it->first; ER_DOMAIN_TYPE v=it->second;
		  // d is the definition statement of the current variable.
		  // v is the current interval for this variable.
		  ASSUME_TYPES( d, VT_INT_DECLARATION, VT_UINT_DECLARATION);
		  if( r.D.count(d)>0 ) { // If true then d is contained in both sets (case one)
			  result.D[d] = operator_widening(result, d, v, result.D[d]); 
		  }
	  }
  }


  ER_FOR_EACH_it_IN_DOMAIN(r.D) {  // The remaining is only to put elements that are only contained in R into account (case three).
	  ast_vertex *d=it->first; ER_DOMAIN_TYPE v=it->second;
	  ASSUME_TYPES( d, VT_INT_DECLARATION, VT_UINT_DECLARATION);
	  if( l.D.count(d)==0 ) {  // Otherwise the element is already handled by the previous loop.
		  result.D[d] = v;
	  }
  }
}

// Join two elements of domain D, that is [a,b] join [c,d]
inline ER_DOMAIN_TYPE 
exclusive_read::operator_join( const ER_DOMAIN_TYPE &l,
		                       const ER_DOMAIN_TYPE &r) 
{	
	#if DEBUG
	cerr << "DEBUG: operator_join:" << value_to_string(l) << " join " << value_to_string(r) << endl;
	#endif

	// The join of two intervals is NOT bottom when one interval is bottom.
	// Instead, undefined  join  defined is defined.
	// The goal of IA is to get more precise intervals, therefore
	// is: undefined  join  [1,1] = [1,1]
	// wrong: ER_RETURN_BOTTOM_WHEN_l_OR_r_IS_BOTTOM(l,r);
	ER_DOMAIN_TYPE bottom = ER_DOMAIN_TYPE_BOTTOM;
	ER_DOMAIN_TYPE top    = ER_DOMAIN_TYPE_TOP;
	if( l==bottom ) return r;
	if( r==bottom ) return l;
	if( l==top || r==top ) return top;
 
	// Otherwise, we do not have a bottom element in L or R.

	// In all cases holds [a,b]  o [c,d]  = [ min{a,c}, max{b,d} ]
	er_basic_type a = l.first;
	er_basic_type b = l.second;
	er_basic_type c = r.first;
	er_basic_type d = r.second;
	er_basic_type min = NULL;
	operator_min(a, c, min);
	#if DEBUG
	cerr << " operator_min result: " << dag_to_string(min) << endl;
	#endif 
	er_basic_type max = NULL;
	operator_max(b, d, max);
	#if DEBUG
	cerr << "join result: [" << dag_to_string(min)  <<"  ,  " << dag_to_string(max) << "]\n";
	#endif 
	return make_pair( min, max );
}

// Widening of two elements, that is [a,b] nabla [c,d]
inline ER_DOMAIN_TYPE 
exclusive_read::operator_widening(exclusive_read& ai, 
		                          ast_vertex *decl,
		                          ER_DOMAIN_TYPE &l, 
								  ER_DOMAIN_TYPE &r
								 ) 
{
	string current_var_name=decl->get_lhs_name(); 
	#if DEBUG
	cerr << "\noperator_widening:" << value_to_string(l) << " widening " << value_to_string(r) << endl;
	cerr << "Current variable is: " << current_var_name << ":\n";

	if( ai.list_of_assertions.size()>0 ) {
		int counter=1;
		cerr  << "list of assertions:\n";
		for(set< ast_vertex* >::const_iterator it=ai.list_of_assertions.begin();it!=ai.list_of_assertions.end();it++) {
			cerr << "Assertion:  " << counter++ << ". '" << test_expr_to_string(*it) << "'\n";
		}
	}


	#endif
	// The join of two intervals is NOT bottom when one interval is bottom.
	// undefined  join  defined is defined.
	// The goal of IA is to get more precise intervals, therefore
	// is: undefined join [1,1] = [1,1]
	// wrong: ER_RETURN_BOTTOM_WHEN_l_OR_r_IS_BOTTOM(l,r);
	ER_DOMAIN_TYPE bottom = ER_DOMAIN_TYPE_BOTTOM;
	if( l==bottom ) return r;
	if( r==bottom ) return l;
 
	// Otherwise, we do not have a bottom element in L or R.

	// Distinguish the cases of   l o r = ?
	// 1. l cap r = l or l cap r = r  (contained)
	// 2. l cap r = emptyset
	// 3. l cap r != emptyset   and   l cap r != l  and  l cap r != r
	// In all cases holds [a,b]  o [c,d]  = [ min{a,c}, max{b,d} ]
	er_basic_type a = l.first;
	er_basic_type b = l.second;
	er_basic_type c = r.first;
	er_basic_type d = r.second;

	er_basic_type min = NULL;
	operator_min(a, c, min);
	if( a==min || ::operator_equal(a,c) )
		min=a;
	else {
		if(decl->get_type()==VT_UINT_DECLARATION)
			min=ZERO;
		else
			min=MINUS_INFINITY;
	}
	er_basic_type max = NULL;
	operator_max(b, d, max);
	if( b==max  ||  ::operator_equal(b,d) )
		max=b;
	else 
		max=PLUS_INFINITY;

	// Now, we look if there is the possibility to reduce the current maximum through a valid constraint.
	for(set< ast_vertex* >::const_iterator it=ai.list_of_assertions.begin();it!=ai.list_of_assertions.end();it++) {
		ast_vertex* assertion = *it;
		ast_vertex* assertion_lhs = FIRSTOF(assertion);
		ast_vertex* assertion_rhs = SECONDOF(assertion);
		if(  
				( assertion->get_type()==VT_SPL_leq  || assertion->get_type()==VT_SPL_lower )   
            &&  current_var_name==assertion_lhs->get_str()
			&&  assertion_rhs->get_type()==VT_SPL_identifier
			&&  sym_tbl.is_int( assertion_rhs->get_str() )
	  	  ) { // We have found a candidate that potentially reduces the current maximum.
			er_basic_type ub_of_rhs = ai.D[ sym_tbl.get_decl( assertion_rhs->get_str() ) ].second;
			// Compare max with upper bound of assertion.
			er_basic_type comparison_min = NULL;
			operator_min(max, ub_of_rhs, comparison_min);
			if( ::operator_equal(comparison_min, ub_of_rhs) ) {
				#if DEBUG
				cerr << "We found a new upper bound but this boundary has to be bigger or equal than the current lower boundary. ... ";
				#endif
				operator_min(min, ub_of_rhs, comparison_min);
				if( ::operator_equal(comparison_min, min) ) {
					#if DEBUG
					cerr << "It is!\n";
					#endif
					max = ub_of_rhs;
				}
				else {
					#if DEBUG
					cerr << "It is NOT!\n";
					#endif
				}
			}
		}
	}

	#if DEBUG
	cerr << "operator_widening result:" << value_to_string(make_pair( min, max )) << endl;
	#endif
	return make_pair( min, max );
}


// Add two elements of domain D, that is [a,b] + [c,d]
inline ER_DOMAIN_TYPE 
exclusive_read::operator_add(  const ER_DOMAIN_TYPE &l
		                     , const ER_DOMAIN_TYPE &r) 
{
	#if DEBUG
	cerr << "DEBUG: _add: " << value_to_string(l) << " (+) " << value_to_string(r) << endl;
	#endif
	ER_RETURN_BOTTOM_WHEN_l_OR_r_IS_BOTTOM(l,r);
	er_basic_type a = l.first; er_basic_type b = l.second;
	er_basic_type c = r.first; er_basic_type d = r.second;
	er_basic_type min;
	er_basic_type max;
	// The best case would be if all boundaries are constant values
	if( a->get_type()==VT_SPL_int_const  &&  b->get_type()==VT_SPL_int_const 
	    && is_common_constant(a) && is_common_constant(b)    
        && c->get_type()==VT_SPL_int_const  &&  d->get_type()==VT_SPL_int_const
	    && is_common_constant(c) && is_common_constant(d)
	  ) 
	{
		long a_val, b_val, c_val, d_val;
		istringstream iss_a(a->get_str()); iss_a>>a_val;
		istringstream iss_b(b->get_str()); iss_b>>b_val;
		istringstream iss_c(c->get_str()); iss_c>>c_val;
		istringstream iss_d(d->get_str()); iss_d>>d_val;
		ostringstream oss_l; oss_l << a_val+c_val;
		ostringstream oss_r; oss_r << b_val+d_val;
		if( a_val+c_val>INT_MAX ) {
			min=PLUS_INFINITY;
			max=PLUS_INFINITY;
		}
		else {
			min = new ast_vertex(0, VT_SPL_int_const, oss_l.str());
			if( b_val+d_val>INT_MAX ) 
				max=PLUS_INFINITY;
			else
				max = new ast_vertex(0, VT_SPL_int_const, oss_r.str());
		}
	}
	else {
		if( a!=PLUS_INFINITY && c!=PLUS_INFINITY ) {
			min = operator_arithmetic(a, c, '+');
			if( b!=PLUS_INFINITY && d!=PLUS_INFINITY ) 
				max = operator_arithmetic(b, d, '+');
			else                                       
				max = PLUS_INFINITY;
		}
		else {
			min = PLUS_INFINITY;
			max = PLUS_INFINITY;
		}
	}
	//assert( operator_lower(min,max) );
	#if DEBUG
	cerr << "DEBUG: _add: result: " << value_to_string(make_pair( min, max )) << endl;
	#endif 
	return make_pair( min, max );
}

// Subtract two elements of domain D, that is [a,b] - [c,d]
inline ER_DOMAIN_TYPE 
exclusive_read::operator_sub( const ER_DOMAIN_TYPE &l
	                        , const ER_DOMAIN_TYPE &r) 
{
	#if DEBUG
	cerr << "DEBUG: _sub: " << value_to_string(l) << " (-) " << value_to_string(r) << endl;
	#endif 
	ER_RETURN_BOTTOM_WHEN_l_OR_r_IS_BOTTOM(l,r);
	er_basic_type a = l.first; er_basic_type b = l.second;
	er_basic_type c = r.first; er_basic_type d = r.second;
	er_basic_type min;
	er_basic_type max;

	// The best case would be if all boundaries are constant values
	if( a->get_type()==VT_SPL_int_const  &&  b->get_type()==VT_SPL_int_const 
	    && is_common_constant(a) && is_common_constant(b)    
        && c->get_type()==VT_SPL_int_const  &&  d->get_type()==VT_SPL_int_const
	    && is_common_constant(c) && is_common_constant(d)
	  ) 
	{
		long a_val, b_val, c_val, d_val;
		istringstream iss_a(a->get_str()); iss_a>>a_val;
		istringstream iss_b(b->get_str()); iss_b>>b_val;
		istringstream iss_c(c->get_str()); iss_c>>c_val;
		istringstream iss_d(d->get_str()); iss_d>>d_val;
		long result_min=a_val-d_val;
		long result_max=b_val-c_val;
		ostringstream oss_l; oss_l << result_min;
		ostringstream oss_r; oss_r << result_max;
		if( result_min>INT_MAX ) {
			min=PLUS_INFINITY;
			max=PLUS_INFINITY;
		}
		else {
			min = new ast_vertex(0, VT_SPL_int_const, oss_l.str());
			if( result_max>INT_MAX ) 
				max=PLUS_INFINITY;
			else
				max = new ast_vertex(0, VT_SPL_int_const, oss_r.str());
		}
	}
	else { // Not all boundaries are constant values:
		if( a!=PLUS_INFINITY && d!=PLUS_INFINITY ) {
			min = operator_arithmetic(a, d, '-');
		}
		else { // a or d is infinity
			if(a==PLUS_INFINITY)
				min = PLUS_INFINITY;
			else 
				min = MINUS_INFINITY;
		}
		if( b!=PLUS_INFINITY && c!=PLUS_INFINITY ) {
			max = operator_arithmetic(b, c, '-');
		}
		else { // b or c is infinity
			if(b==PLUS_INFINITY)
				max = PLUS_INFINITY;
			else 
				max = MINUS_INFINITY;
		}
	}

	/* assert( operator_lower(min,max) ); */
	#if DEBUG
	cerr << "DEBUG: _sub: result: " << value_to_string(make_pair( min, max )) << endl;
	#endif 
	return make_pair( min, max );
}

// Multiply two elements of domain D, that is [a,b] * [c,d]
inline ER_DOMAIN_TYPE 
exclusive_read::operator_mul(  const ER_DOMAIN_TYPE &l
		                     , const ER_DOMAIN_TYPE &r) 
{
	ER_RETURN_BOTTOM_WHEN_l_OR_r_IS_BOTTOM(l,r);
	er_basic_type a = l.first; er_basic_type b = l.second;
	er_basic_type c = r.first; er_basic_type d = r.second;
	er_basic_type min;
	er_basic_type max;

	// Case that one interval represents a constant, eg [a,a] * [c,d]:
	if( ::operator_equal(a,b) || ::operator_equal(c,d) ) {
		min = operator_arithmetic(a, c, '*');
		max = operator_arithmetic(b, d, '*');
		return make_pair( min, max );
	}

	er_basic_type mul1; // a*c
	er_basic_type mul2; // a*d
	er_basic_type mul3; // b*c
	er_basic_type mul4; // b*d
	// a*c
	if( a==PLUS_INFINITY || c==PLUS_INFINITY ) mul1=PLUS_INFINITY;
	else                                       mul1=operator_arithmetic(a,c,'*');
	// a*d
	if( a==PLUS_INFINITY || d==PLUS_INFINITY ) mul2=PLUS_INFINITY;
	else                                       mul2=operator_arithmetic(a, d, '*');
	// b*c
	if( b==PLUS_INFINITY || c==PLUS_INFINITY ) mul3=PLUS_INFINITY;
	else                                       mul3=operator_arithmetic(b, c, '*');
	// b*d
	if( b==PLUS_INFINITY || d==PLUS_INFINITY ) mul4=PLUS_INFINITY;
	else                                       mul4=operator_arithmetic(b, d, '*');

	operator_min(mul1, mul2, min); 
	operator_min(min, mul3, min); 
	operator_min(min, mul4, min); 

	operator_max(mul1, mul2, max); 
	operator_max(max, mul3, max); 
	operator_max(max, mul4, max); 

	return make_pair( min, max );
}

// Divide two elements of domain D, that is [a,b] / [c,d]
inline ER_DOMAIN_TYPE 
exclusive_read::operator_div(  const ER_DOMAIN_TYPE &l
		                        , const ER_DOMAIN_TYPE &r) 
{
	ER_RETURN_BOTTOM_WHEN_l_OR_r_IS_BOTTOM(l,r);
	er_basic_type a = l.first; er_basic_type b = l.second;
	er_basic_type c = r.first; er_basic_type d = r.second;
	er_basic_type min;
	er_basic_type max;
	if(c->get_type()==VT_SPL_int_const  &&  c->get_str()=="0") { // In case that divisor is zero we return TOP as interval.
		return ER_DOMAIN_TYPE_TOP; // [-inf,inf]
	}

	// Case that one interval represents a constant, eg [a,b] / [c,c]:
	if( ::operator_equal(c,d) ) {
		min = operator_arithmetic(a, c, '/');
		max = operator_arithmetic(b, d, '/');
		return make_pair( min, max );
	}

	er_basic_type div1; // a/c
	er_basic_type div2; // a/d
	er_basic_type div3; // b/c
	er_basic_type div4; // b/d
	// a/c
	     if( a==PLUS_INFINITY && c==PLUS_INFINITY ) div1=PLUS_INFINITY;
	else if( a!=PLUS_INFINITY && c==PLUS_INFINITY ) div1=ZERO;
	else if( a==PLUS_INFINITY && c!=PLUS_INFINITY ) div1=PLUS_INFINITY;
	else                                            div1=operator_arithmetic(a,c,'/');

	// a/d
	     if( a==PLUS_INFINITY && d==PLUS_INFINITY ) div2=PLUS_INFINITY;
	else if( a!=PLUS_INFINITY && d==PLUS_INFINITY ) div2=ZERO;
	else if( a==PLUS_INFINITY && d!=PLUS_INFINITY ) div2=PLUS_INFINITY;
	else                                            div2=operator_arithmetic(a,d,'/');

	// b/c
	     if( b==PLUS_INFINITY && c==PLUS_INFINITY ) div3=PLUS_INFINITY;
	else if( b!=PLUS_INFINITY && c==PLUS_INFINITY ) div3=ZERO;
	else if( b==PLUS_INFINITY && c!=PLUS_INFINITY ) div3=PLUS_INFINITY;
	else                                            div3=operator_arithmetic(b, c, '/');

	// b/d
	     if( b==PLUS_INFINITY && d==PLUS_INFINITY ) div4=PLUS_INFINITY;
	else if( b!=PLUS_INFINITY && d==PLUS_INFINITY ) div4=ZERO;
	else if( b==PLUS_INFINITY && d!=PLUS_INFINITY ) div4=PLUS_INFINITY;
	else                                            div4=operator_arithmetic(b, d, '/');

	operator_min(div1, div2, min); 
	operator_min(min, div3, min); 
	operator_min(min, div4, min); 

	operator_max(div1, div2, max); 
	operator_max(max, div3, max); 
	operator_max(max, div4, max); 

	return make_pair( min, max );
}


inline 
bool 
exclusive_read::operator_lower(   const ER_DOMAIN_TYPE &l
		                        , const ER_DOMAIN_TYPE &r) 
{
	er_basic_type a = l.first; er_basic_type b = l.second;
	er_basic_type c = r.first; er_basic_type d = r.second;
	er_basic_type req1;
	er_basic_type req2;
	operator_min(a, c, req1);
	operator_min(b, d, req2);
	if( ::operator_equal(req1, c)  &&  ::operator_equal(req2, b) ) 
		return true;
	else
		return false;
}





inline void 
exclusive_read::init_prg_analysis(ast_vertex *astv) 
{
	astv->phi = &phi;	
	if(astv->get_type()==VT_SPL_CODE) { // Init program analysis depending on data flow of SPL code.
		for(set< pair<ast_vertex*,ast_vertex*> >::const_iterator it=astv->get_flow().begin(); it!=astv->get_flow().end();it++)  {
			ast_vertex *from=it->first;
			ast_vertex *to=it->second;
			assert(from);   assert(to);
			// If there is no equation for the current node put it into equation vector.
			unsigned i=0;
			for(i=0;i<equations.size() && equations[i]!=from;i++) 
				;
			if(i==equations.size()) {
				equations.push_back(from);
				assert( !from->AI );
				from->AI = new exclusive_read;
				((exclusive_read*) from->AI)->astv = from;
			}
			for(i=0;i<equations.size() && equations[i]!=to;i++) 
				;
			if(i==equations.size()) {
				equations.push_back(to);
				assert( !to->AI );
				to->AI = new exclusive_read;
				((exclusive_read*) to->AI)->astv = to;
			}
			if(to->AI)
				((exclusive_read*) to->AI)->data_flow_predecessors.insert(from);
		}
	}
	FOR_EACH_it_IN(astv->get_children()) {
		init_prg_analysis(*it);
	}
}

inline void 
exclusive_read::delete_prg_analysis(ast_vertex *astv) 
{
	FOR_EACH_it_IN(astv->get_children()) {
		delete_prg_analysis(*it);
	}
	astv->AI=NULL;
	astv->phi = NULL;	
}

void 
exclusive_read::operator_min( er_basic_type l, 
 		                      er_basic_type r, 
			                  er_basic_type &min)
{
	// Differ between natural number expressions and those from the domain
	// of integer numbers.
	bool only_unsigned_ints = 0;
	set<ast_vertex*> s;
	l->are_there_only_uints(s);
	r->are_there_only_uints(s);
	if( s.size()>0 )
		only_unsigned_ints = 1; 

	#if DEBUG
	cerr << "only_unsigned_ints: " << only_unsigned_ints << endl;
	cerr << "operator_min( " << dag_to_string(l) << ", " << dag_to_string(r) << ")\n";
	#endif 
	if( ::operator_equal(l,r) ) { 
		#if DEBUG
		cerr << "operator_min: equal\n";
		#endif 
		min=l; return; 
	}
	else {
		er_basic_type test1=NULL;
		er_basic_type test2=NULL;

		if( r->get_type()==VT_SPL_int_const && !is_common_constant(r) ) {
			#if DEBUG
			cerr << "operator_min: ( r->get_type()==VT_SPL_int_const && !is_common_constant(r) ): for " << dag_to_string(l) << " and " << dag_to_string(r)<< ".\n";
			#endif 
			if(  only_unsigned_ints  &&  ::operator_equal(r, ZERO) ) { min=r; return; }
			if( ::operator_equal(r, MINUS_INFINITY) ) { min=r; return; }
			if( ::operator_equal(r, PLUS_INFINITY) )   { min=l; return; }
			if( ::operator_equal(r, THREAD_ID) )       { min=ZERO; return; }
			if( ::operator_equal(r, NUM_OF_THREADS) )  { min=ZERO; return; }
			min=ZERO;
			#if DEBUG
			cerr << "operator_min: result: " << dag_to_string(min) << endl;
			#endif 
			return;
		}
		else if( l->get_type()==VT_SPL_int_const && !is_common_constant(l) ) {
			#if DEBUG
			cerr << "operator_min: ( l->get_type()==VT_SPL_int_const && !is_common_constant(l) ): for " << dag_to_string(l) << " and " << dag_to_string(r)<< ".\n";
			#endif 
			if(  only_unsigned_ints  &&  ::operator_equal(l, ZERO) ) { min=l; return; }
			if( ::operator_equal(l, MINUS_INFINITY) ) { min=l; return; }
			if( ::operator_equal(l, MINUS_INFINITY) )   { min=l; return; }
			if( ::operator_equal(l, PLUS_INFINITY) )   { min=r; return; }
			if( ::operator_equal(l, THREAD_ID) )       { min=ZERO; return; }
			if( ::operator_equal(l, NUM_OF_THREADS) )  { min=ZERO; return; }
			min=ZERO;
			#if DEBUG
			cerr << "operator_min: result: " << dag_to_string(min) << endl;
			#endif 
			return;
		}



		// l and r are different.
		if( l->get_type()==VT_SPL_int_const && r->get_type()==VT_SPL_int_const ) {
			if( !is_common_constant(l)  &&  !is_common_constant(r) ) {
				#if DEBUG
				cerr << "operator_min: ( is_common_constant(l)  &&  is_common_constant(r) ): for " << dag_to_string(l) << " and " << dag_to_string(r)<< ".\n";
				#endif 
				if(  only_unsigned_ints  &&  ::operator_equal(l, ZERO) ) { min=l; return; }
				if(  only_unsigned_ints  &&  ::operator_equal(r, ZERO) ) { min=r; return; }
				if( ::operator_equal(l, MINUS_INFINITY) ) { min=l; return; }
				if( ::operator_equal(r, MINUS_INFINITY) ) { min=r; return; }
				if( ::operator_equal(l, PLUS_INFINITY) ) { min=r; return; }
				if( ::operator_equal(r, PLUS_INFINITY) ) { min=l; return; }
				if( ::operator_equal(l, ZERO) && ::operator_equal(r, PLUS_INFINITY) ) { min=l; return; }
				if( ::operator_equal(r, ZERO) && ::operator_equal(l, PLUS_INFINITY) ) { min=r; return; }
				if( ::operator_equal(l, ZERO) && ::operator_equal(r, MINUS_INFINITY) ) { min=r; return; }
				if( ::operator_equal(r, ZERO) && ::operator_equal(l, MINUS_INFINITY) ) { min=l; return; }
				min=ZERO; 
				#if DEBUG
				cerr << "operator_min: result: " << dag_to_string(min) << endl;
				#endif 
				return; 
			}
			else if( is_common_constant(l)  &&  is_common_constant(r) ) {
				#if DEBUG
				cerr << "operator_min: ( is_common_constant(l)  &&  is_common_constant(r) ): for " << dag_to_string(l) << " and " << dag_to_string(r)<< ".\n";
				#endif 
				istringstream iss_l(l->get_str());  long val_l; iss_l >> val_l;
				istringstream iss_r(r->get_str());  long val_r; iss_r >> val_r;
				if(val_l<val_r) { min=l; }
				else            { min=r; }
				#if DEBUG
				cerr << "operator_min: result: " << dag_to_string(min) << endl;
				#endif 
				return;
			}
			else 
				assert(0);
		}

		#if 0
		// If there is the constant zero in L or in R.
		if( l->get_type()==VT_SPL_int_const && is_common_constant(l) ) {
			#if DEBUG
			cerr << "operator_min: If there is the constant zero in L .\n";
			#endif 
			istringstream iss(l->get_str());  
			uint val;
			iss >> val;
			if( val==0 ) {
				min=l;
				#if DEBUG
				cerr << "operator_min: result: " << dag_to_string(min) << endl;
				#endif 
				return;
			}
		}	
		if( r->get_type()==VT_SPL_int_const && is_common_constant(r) ) {
			#if DEBUG
			cerr << "operator_min: If there is the constant zero in R .\n";
			#endif 
			istringstream iss(r->get_str());  
			uint val;
			iss >> val;
			if( val==0 ) {
				min=r;
				#if DEBUG
				cerr << "operator_min: result: " << dag_to_string(min) << endl;
				#endif 
				return;
			}
		}	
		#endif

		// Otherwise
		ast_vertex *const_vertex=NULL;
		// Case min(S, const+S) ?
		if( (const_vertex=min_S_or_const_plus_S(l,r))!=NULL ) {
			#if DEBUG
			cerr << "operator_min: min_S_or_const_plus_S fulfilled for " << dag_to_string(l) << " and " << dag_to_string(r)<< ".\n";
			#endif
			istringstream iss(const_vertex->get_str());
			long astv_val; iss >> astv_val;
			if(astv_val>=0)
				// Minimum is S not S+const
				min = ( l->get_type()==VT_SPL_int_add ) ? r : l;
			else
				// Minimum is S+const not S
				min = ( l->get_type()==VT_SPL_int_add ) ? l : r;
			#if DEBUG
			cerr << "operator_min: result: " << dag_to_string(min) << endl;
			#endif 
			return;
		}	
		// 2 cases:
		// 1: S, S-const
		// 2: S-const, S
		else if(    r->get_type()==VT_SPL_int_sub  &&  ::operator_equal(l, FIRSTOF(r))
			     && SECONDOF(r)->get_type()==VT_SPL_int_const  &&  is_common_constant(SECONDOF(r)) ) { // case 1 
			#if DEBUG
			cerr << "operator_min: S, S-const\n";
			#endif 
			istringstream iss(SECONDOF(r)->get_str()); long val; iss >> val;
			if(val>=0) min = r; 
			else       min = l;
			#if DEBUG
			cerr << "operator_min: result: " << dag_to_string(min) << endl;
			#endif 
			return;
		}
		else if(    l->get_type()==VT_SPL_int_sub  &&  ::operator_equal(r, FIRSTOF(l))
			     && SECONDOF(l)->get_type()==VT_SPL_int_const  &&  is_common_constant(SECONDOF(l)) ) { // case 2 
			#if DEBUG
			cerr << "operator_min: S-const, S\n";
			#endif 
			istringstream iss(SECONDOF(l)->get_str()); long val; iss >> val;
			if(val>=0) min = l; 
			else       min = r;
			#if DEBUG
			cerr << "operator_min: result: " << dag_to_string(min) << endl;
			#endif 
			return;
		}
		else if( (test1=special_case_001(l,r))!=NULL  ||  (test2=special_case_001(r,l))!=NULL) {
			if(test1!=NULL) {
				#if DEBUG
				cerr << "operator_min: special_case_001 fulfilled for " << dag_to_string(l) << " and " << dag_to_string(r)<< ".\n";
				#endif
				min=test1;
			}
			else {
				#if DEBUG
				cerr << "operator_min: special_case_001 fulfilled for " << dag_to_string(r) << " and " << dag_to_string(l)<< ".\n";
				#endif
				min=test2;
			}
			#if DEBUG
			cerr << "operator_min: result: " << dag_to_string(min) << endl;
			#endif 
			return;
		}	
		else if( (test1=special_case_002(l,r))!=NULL  ||  (test2=special_case_002(r,l))!=NULL) {
			#if DEBUG
			cerr << "operator_min: special_case_002 fulfilled for " << dag_to_string(l) << " and " << dag_to_string(r)<< ".\n";
			#endif
			if(test1!=NULL) { min=test1; return; }
			else { min=test2; return; }
		}	
		else if(  // CASE min( S (+|-) C1, S (+|-) C2 )
		             ( l->get_type()==VT_SPL_int_add  ||  l->get_type()==VT_SPL_int_sub )
		         &&  ( r->get_type()==VT_SPL_int_add  ||  r->get_type()==VT_SPL_int_sub )
				 &&  ::operator_equal(FIRSTOF(l), FIRSTOF(r))
			     &&  SECONDOF(l)->get_type()==VT_SPL_int_const  &&  is_common_constant(SECONDOF(l)) 
			     &&  SECONDOF(r)->get_type()==VT_SPL_int_const  &&  is_common_constant(SECONDOF(r)) 
			   )
		{ 
			#if DEBUG
			cerr << "operator_min: CASE min( S (+|-) C1, S (+|-) C2 )  \n";
			#endif 
			long l_val, r_val;
			istringstream iss_l(SECONDOF(l)->get_str()); iss_l>>l_val;
			istringstream iss_r(SECONDOF(r)->get_str()); iss_r>>r_val;
			if( l->get_type()==VT_SPL_int_sub ) l_val *= -1;
			if( r->get_type()==VT_SPL_int_sub ) r_val *= -1;
			r_val -= l_val;
			#if DEBUG
			cerr << "operator_min: The decision is whether or not 0<="<<r_val<<endl;
			#endif 
			if(0<=r_val) min = l; 
			else         min = r;
			#if DEBUG
			cerr << "operator_min: result: " << dag_to_string(min) << endl;
			#endif 
			return;
		}
		// This case is redundant since it is a special case of the case above.
		#if 0
		   // Case (n (+|-) c1)   and   (n (+|-) c2)
		else if(    (l->get_type()==VT_SPL_int_add || l->get_type()==VT_SPL_int_sub)
			     && (r->get_type()==VT_SPL_int_add || r->get_type()==VT_SPL_int_sub)                            
				 && FIRSTOF(l)->get_type()==VT_SPL_identifier  &&  FIRSTOF(r)->get_type()==VT_SPL_identifier   
				 && FIRSTOF(l)->get_str()==FIRSTOF(r)->get_str()                                              
			     && SECONDOF(l)->get_type()==VT_SPL_int_const  &&  is_common_constant(SECONDOF(l))           
			     && SECONDOF(r)->get_type()==VT_SPL_int_const  &&  is_common_constant(SECONDOF(r)) 
			   ) 
		{ 
			#if DEBUG
			cerr << "operator_min: (identifier (+|-) constant1) and (identifier (+|-) constant2)  \n";
			#endif 
			long l_val, r_val;
			istringstream iss_l(SECONDOF(l)->get_str()); iss_l>>l_val;
			istringstream iss_r(SECONDOF(r)->get_str()); iss_r>>r_val;
			if( l->get_type()==VT_SPL_int_sub ) l_val *= -1;
			if( r->get_type()==VT_SPL_int_sub ) r_val *= -1;
			r_val -= l_val;
			#if DEBUG
			cerr << "operator_min: The decision is whether or not 0<="<<r_val<<endl;
			#endif 
			if(0<=r_val) min = l; 
			else         min = r;
			return;
		}
		#endif
	}
	#if DEBUG
	cerr << "operator_min: cannot be decided, so return -inf.\n";
	#endif
	if(only_unsigned_ints)
		min = ZERO;
	else
		min = MINUS_INFINITY;
}

void 
exclusive_read::operator_max( er_basic_type l, 
		                      er_basic_type r, 
			                  er_basic_type &max)
{
	#if DEBUG
	cerr << "operator_max( " << dag_to_string(l) << ", " << dag_to_string(r) << ")\n";
	#endif 
	if( ::operator_equal(l,r) ) { max=l; return; }
	else {
		er_basic_type min;
		operator_min(l, r, min);
		if( min==l ) { max=r; return; }
		if( min==r ) { max=l; return; }
	}
	max = PLUS_INFINITY;
}


er_basic_type
exclusive_read::min_S_or_const_plus_S(er_basic_type l, er_basic_type r)
{
	// 4 cases:
	// 1: S, const+S 
	// 2: S, S+const 
	// 3: S+const, S
	// 4: const+S, S
	er_basic_type result=NULL;
	if(    r->get_type()==VT_SPL_int_add  
		&& FIRSTOF(r)->get_type()==VT_SPL_int_const	
		&& ::operator_equal(l, SECONDOF(r))   ) // case 1 
		result = l;
	if(    r->get_type()==VT_SPL_int_add  
		&& SECONDOF(r)->get_type()==VT_SPL_int_const	
		&& ::operator_equal(l, FIRSTOF(r))   ) // case 2 
		result = l;
	if(    l->get_type()==VT_SPL_int_add  
		&& SECONDOF(l)->get_type()==VT_SPL_int_const	
		&& ::operator_equal(r, FIRSTOF(l))   ) // case 3
		result = r;
	if(    l->get_type()==VT_SPL_int_add  
		&& FIRSTOF(l)->get_type()==VT_SPL_int_const	
		&& ::operator_equal(r, SECONDOF(l))   ) // case 4
		result = r;
	return result;
}

er_basic_type
exclusive_read::special_case_001(er_basic_type l, er_basic_type r)  // Special case l=((t+1)*(n/p))-1  and r=(t*(n/p))
{
	if(    l->get_type()==VT_SPL_int_sub  
		&& SECONDOF(l)->get_type()==VT_SPL_int_const  &&  SECONDOF(l)->get_str()=="1"
		&& FIRSTOF(l)->get_type()==VT_SPL_int_mul 
	  ) {
		ast_vertex* mul=FIRSTOF(l);
		if( FIRSTOF(mul)->get_type()==VT_SPL_int_add  &&  SECONDOF(mul)->get_type()==VT_SPL_int_div ) {
			ast_vertex* add=FIRSTOF(mul);
			ast_vertex* div=SECONDOF(mul);
			if( ::operator_equal(FIRSTOF(add), THREAD_ID)  &&  SECONDOF(add)->get_type()==VT_SPL_int_const && SECONDOF(add)->get_str()=="1" ) {
				// Compare right operand
				if(    r->get_type()==VT_SPL_int_mul 
					&& ::operator_equal(FIRSTOF(r), THREAD_ID)
					&& ::operator_equal(div, SECONDOF(r)) 
				  ) {
					// Special case holds and left operand is minimum iff n < number_of_threads or in other words right operand is 
					// minimum iff n >= number_of_threads.
					cerr << "warning: special case 001 holds only iff n >= omp_get_num_threads()!\n";
					return r;
				}
			}
		}
	}
	return NULL;
}

er_basic_type
exclusive_read::special_case_002(er_basic_type l, er_basic_type r)  
	// Special case l=ub(t)   and r=lb(t+1)
{
	if( l->depth()==5 && r->depth()==3 ) {
		if( ::operator_equal( FIRSTOF(FIRSTOF(FIRSTOF(l))), FIRSTOF(r)) ) {
			if(    l->get_type()==VT_SPL_int_add  &&  r->get_type()==VT_SPL_int_mul // first level
				&& FIRSTOF(l)->get_type()==VT_SPL_int_mul && SECONDOF(l)->get_type()==VT_SPL_int_sub // 2nd level
				&& FIRSTOF(FIRSTOF(l))->get_type()==VT_SPL_int_sub 
				&& ::operator_equal( SECONDOF(FIRSTOF(l)) , SECONDOF(r) )
				&& ::operator_equal( FIRSTOF(SECONDOF(l)) , SECONDOF(r) )
				&& SECONDOF(SECONDOF(l))->get_type()==VT_SPL_int_const && SECONDOF(SECONDOF(l))->get_str()=="1"
				&& SECONDOF(FIRSTOF(FIRSTOF(l)))->get_type()==VT_SPL_int_const && SECONDOF(FIRSTOF(FIRSTOF(l)))->get_str()=="1"
			  ) {
				return l;
			}
		}
	}
	return NULL;
}

er_basic_type
exclusive_read::operator_arithmetic(er_basic_type l, er_basic_type r, const char op)
{
	#if DEBUG
	cerr << "operator_arithmetic: \n";
	cerr << "l->get_type(): " << l->vertex_type_2_string() << endl;
	cerr << "r->get_type(): " << r->vertex_type_2_string() << endl;
	cerr << "op: " << op << endl;
	#endif 
	ast_vertex* result = NULL; 
	if( l->get_type()==VT_SPL_int_const  &&  r->get_type()==VT_SPL_int_const 
	    && is_common_constant(l) && is_common_constant(r)    ) {
		long l_val, r_val;
		istringstream iss_l(l->get_str()); iss_l>>l_val;
		istringstream iss_r(r->get_str()); iss_r>>r_val;
		long result_long;
		switch( op ) {
			case '+': result_long=l_val+r_val; break;
			case '-': result_long=l_val-r_val; break;
			case '*': result_long=l_val*r_val; break;
			case '/': result_long=l_val/r_val; break;
			default:
					  assert(0);
		}
		ostringstream oss;
		oss << result_long;
		result = new ast_vertex(l->get_line_number(), VT_SPL_int_const, oss.str());
		assert(result);
		return result;
	}
	// Case : left is addition right operand is constand AND 2nd operand of addition is also a constant:
	else if(     op=='+'  &&   (l->get_type()==VT_SPL_int_add  || l->get_type()==VT_SPL_int_sub)
	    &&  r->get_type()==VT_SPL_int_const    &&   SECONDOF(l)->get_type()==VT_SPL_int_const
	    && is_common_constant(SECONDOF(l)) && is_common_constant(r)    ) {
		long l_val, r_val;
		istringstream iss_l(SECONDOF(l)->get_str()); iss_l>>l_val;
		istringstream iss_r(r->get_str()); iss_r>>r_val;
		long result_long=0;
		if(l->get_type()==VT_SPL_int_add)
			result_long=l_val+r_val;
		else if(l->get_type()==VT_SPL_int_sub) {
			l_val *= -1;
			result_long=l_val+r_val; // op is '+'
		}
		else 
			assert(0);
		if(result_long==0) {
			return FIRSTOF(l);
		}
		else if(result_long>0) {
			ostringstream oss;  oss << result_long;
			ast_vertex* result_c1_op_c2 = new ast_vertex(0, VT_SPL_int_const, oss.str()); assert(result_c1_op_c2);
			result = new ast_vertex(0, VT_SPL_int_add, "");
			add_child(result, FIRSTOF(l)->clone());
			add_child(result, result_c1_op_c2);
		}
		else if(result_long<0) {
			result_long*=-1;
			ostringstream oss;  oss << result_long;
			ast_vertex* result_c1_op_c2 = new ast_vertex(0, VT_SPL_int_const, oss.str()); assert(result_c1_op_c2);
			result = new ast_vertex(0, VT_SPL_int_sub, "");
			add_child(result, FIRSTOF(l)->clone());
			add_child(result, result_c1_op_c2);
		}
		else 
			assert(0);
		assert(result);
		return result;
	}
	else {
		switch( op ) {
			case '+': result=new ast_vertex(l->get_line_number(), VT_SPL_int_add, ""); break;
			case '-': result=new ast_vertex(l->get_line_number(), VT_SPL_int_sub, ""); break;
			case '*': result=new ast_vertex(l->get_line_number(), VT_SPL_int_mul, ""); break;
			case '/': result=new ast_vertex(l->get_line_number(), VT_SPL_int_div, ""); break;
			default:
					  assert(0);
		}
		add_child( result, l->clone() );
		add_child( result, r->clone() );
		assert(result);
		return result;
	}
}


bool 
exclusive_read::is_common_constant(er_basic_type c) 
{
	if(    c==BOTTOM  
		|| c==TOP 
	/*	|| c==ZERO  */
		|| c==MINUS_INFINITY
		|| c==PLUS_INFINITY
		|| c==THREAD_ID
		|| c==NUM_OF_THREADS  
      )
		return false;
	else
		return true;
}


inline void 
exclusive_read::print_results(bool intermediate_result) 
{
	static int counter = 1;
	ostringstream oss;
	if(counter==1) { 
		oss << "rm -f " << RESULT_FILE << "-*.txt";
		system(oss.str().c_str());
	}
	oss.str("");
	if(intermediate_result)
		oss << RESULT_FILE << "-" << setfill('0') << setw(3) << counter++ << ".txt";
	else
		oss << RESULT_FILE << "-endresult.txt";
	ofstream ofs(oss.str().c_str()); assert( ofs.good() );
	for(unsigned i=0;i<equations.size();i++) {
		ofs << AI_to_string(equations[i]);
	}

	if(!intermediate_result) {
		for(map< ast_vertex* , bool >::const_iterator it=is_memory_read_exclusive.begin();it!=is_memory_read_exclusive.end();it++) {
			ast_vertex* v=it->first; bool value=it->second;
			ast_vertex* associated_assign=v->get_predecessor();
			assert(v);
			while(    associated_assign->get_type()!=VT_SPL_float_assign 
				   && associated_assign->get_type()!=VT_SPL_float_plus_assign ) {
				associated_assign = associated_assign->get_predecessor();
			}
			string varname = v->get_str();
			if( v->get_children().size()>0 ) {
				string array_index = dag_to_string(FIRSTOF(FIRSTOF(v)));
				varname += "[" + array_index  + "]";
			}
			ofs << "Read access " << v->get_id() << " in float-assignment " <<  associated_assign->get_id() << " (line "<< associated_assign->get_line_number() <<") represents "<< setw(20) << varname << " type(" << v->vertex_type_2_string() <<") : exclusive read: ";
			if(value) ofs << "yes";
			else      ofs << "no";
			ofs << endl;
		}

		#if 1
		// Now, we have to connect the individual states of each array index with other indexes. For example if x[i] is exclusive read
		// and x[j] not then the read access of x[i] is exclusive but this reference may be read by another thread through reference x[j].
		for(map< ast_vertex* , bool >::const_iterator it=is_memory_read_exclusive.begin();it!=is_memory_read_exclusive.end();it++) {
			for(map< ast_vertex* , bool >::const_iterator it2=is_memory_read_exclusive.begin();it2!=is_memory_read_exclusive.end();it2++) {
				if( it!=it2 ) {
					ast_vertex* v1=it->first;  bool value1=it->second;
					ast_vertex* v2=it2->first; bool value2=it2->second;
					assert(v1); assert(v2);
					string varname1 = v1->get_str();
					string varname2 = v2->get_str();
					// Memory accesses with array indexes.
					if( v1->get_children().size()>0  &&  v2->get_children().size()>0 ) {
						ast_vertex* decl1 = sym_tbl.get_decl(varname1);
						ast_vertex* decl2 = sym_tbl.get_decl(varname2);
						if( decl1==decl2  &&  value1!=value2 ) {
							ast_vertex* associated_assign1=v1->get_predecessor();
							ast_vertex* associated_assign2=v2->get_predecessor();
							while(    associated_assign1->get_type()!=VT_SPL_float_assign 
									&& associated_assign1->get_type()!=VT_SPL_float_plus_assign ) {
								associated_assign1 = associated_assign1->get_predecessor();
							}
							while(    associated_assign2->get_type()!=VT_SPL_float_assign 
									&& associated_assign2->get_type()!=VT_SPL_float_plus_assign ) {
								associated_assign2 = associated_assign2->get_predecessor();
							}
							string array_index1 = dag_to_string(FIRSTOF(FIRSTOF(v1)));
							varname1 += "[" + array_index1  + "]";
							string array_index2 = dag_to_string(FIRSTOF(FIRSTOF(v2)));
							varname2 += "[" + array_index2  + "]";
							is_memory_read_exclusive[v1] = 0;
							is_memory_read_exclusive[v2] = 0;
							ofs << "Adjustment of read accesses " << v1->get_id() 
								<< " in float-assignment " <<  associated_assign1->get_id() 
								<< " (line "<< associated_assign1->get_line_number() << ") represents "
								<< setw(20) << varname1 << " : exclusive read: " ;
							if(is_memory_read_exclusive[v1]) ofs << "yes" << endl;
							else                             ofs << "no"  << endl;
							ofs << "Adjustment of read accesses " << v2->get_id() 
								<< " in float-assignment " <<  associated_assign2->get_id() 
								<< " (line "<< associated_assign2->get_line_number() << ") represents "
								<< setw(20) << varname2 << " : exclusive read: " ;
							if(is_memory_read_exclusive[v2]) ofs << "yes" << endl;
							else                             ofs << "no"  << endl;
						}
					}
				}
			}
		}
		#endif
	}
}

inline const string
exclusive_read::value_to_string(ER_DOMAIN_TYPE d) 
{
	ostringstream oss;
	er_basic_type lower_bound = d.first; 
	er_basic_type upper_bound = d.second;
	ER_DOMAIN_TYPE bottom = ER_DOMAIN_TYPE_BOTTOM;
	if( d==bottom )
		oss << "BOTTOM";
	else
		oss << "[ " << basic_type_value_to_string(lower_bound) << ", " << basic_type_value_to_string(upper_bound) << " ]";
	return oss.str();
}

inline const string
exclusive_read::basic_type_value_to_string(er_basic_type btv) 
{
	ostringstream oss;
	assert(btv);
	     if(btv==TOP)                           oss << "TOP";  
	else if(btv==BOTTOM)                        oss << "BOTTOM";
	else if(btv==THREAD_ID)                     oss << "THREAD_ID";
	else if(btv==NUM_OF_THREADS)                oss << "NUM_OF_THREADS";
	else if(btv==PLUS_INFINITY)                 oss << "PLUS_INFINITY";
    else if(btv->get_type()==VT_SPL_identifier) oss << btv->get_str();
    else if(btv->get_type()==VT_SPL_int_const)  oss << btv->get_str();
    else                                        oss << dag_to_string(btv);
	return oss.str();
}

const string 
exclusive_read::dag_to_string(ast_vertex* v)
{
	switch(v->get_type()) {
		case VT_SPL_identifier:
		case VT_SPL_int_const:
			return v->get_str();
		default:
			break;
	}
	assert(v->get_children().size()==2); 
	assert(FIRSTOF(v)); 
	assert(SECONDOF(v));
	string l; string r;
	if( FIRSTOF(v)->get_children().size()==0 ) 
		l = dag_to_string(FIRSTOF(v));
	else
		l = "(" + dag_to_string(FIRSTOF(v)) + ")";
	if( SECONDOF(v)->get_children().size()==0 ) 
		r = dag_to_string(SECONDOF(v));
	else 
		r = "(" + dag_to_string(SECONDOF(v)) + ")";
	switch(v->get_type()) {
	case VT_SPL_int_add:
		return  l + "+" + r;
	case VT_SPL_int_sub:
		return  l + "-" + r;
	case VT_SPL_int_mul:
		return  l + "*" + r;
	case VT_SPL_int_div:
		return  l + "/" + r;
	default:
		UNKNOWN_TYPE(v);
	}
	return "UNKNOWN";
}

const string 
exclusive_read::test_expr_to_string(ast_vertex* v)
{
	switch(v->get_type()) {
		case VT_SPL_identifier:
		case VT_SPL_int_const:
		case VT_SPL_float_const:
		case VT_SPL_STACKcempty:
		case VT_SPL_STACKctop:
			return v->get_str();
		default:
			break;
	}
	//DUMP_TREE(v);
	if( v->get_children().size()==2 ) {
		assert(v->get_children().size()==2); 
		assert(FIRSTOF(v)); 
		assert(SECONDOF(v));
		string l; string r;
		if( FIRSTOF(v)->get_children().size()==0 ) 
			l = test_expr_to_string(FIRSTOF(v));
		else
			l = "(" + test_expr_to_string(FIRSTOF(v)) + ")";
		if( SECONDOF(v)->get_children().size()==0 ) 
			r = test_expr_to_string(SECONDOF(v));
		else 
			r = "(" + test_expr_to_string(SECONDOF(v)) + ")";
		switch(v->get_type()) {
			case VT_SPL_eq:
				return  l + "==" + r;
			case VT_SPL_neq:
				return  l + "!=" + r;
			case VT_SPL_leq:
				return  l + "<=" + r;
			case VT_SPL_geq:
				return  l + ">=" + r;
			case VT_SPL_lower:
				return  l + "<" + r;
			case VT_SPL_greater:
				return  l + ">" + r;

			default:
				UNKNOWN_TYPE(v);
		}
	}
	else if( v->get_children().size()==1 ) {
		assert(FIRSTOF(v)); 
		string l; 
		if( FIRSTOF(v)->get_children().size()==0 ) 
			l = test_expr_to_string(FIRSTOF(v));
		else
			l = "(" + test_expr_to_string(FIRSTOF(v)) + ")";
		switch(v->get_type()) {
			case VT_SPL_not:
				return  "!" + l ;

			default:
				UNKNOWN_TYPE(v);
		}
	}
	else {
		cerr << "error: number of children is " << v->get_children().size() << endl;
		assert(0); // Number of children should be 1 or 2.
	}
	return "UNKNOWN";
}


inline const string
exclusive_read::AI_to_string(ast_vertex *astv) 
{
	ostringstream oss;
	PRINT_LINE;
	oss << "Node " << astv->get_id() << " ("<< astv->vertex_type_2_string() << "):\n";	
	ER_FOR_EACH_it_IN_DOMAIN(((exclusive_read*)astv->AI)->D) {
		ast_vertex *d=it->first; ER_DOMAIN_TYPE v=it->second;
		ASSUME_TYPES( d, VT_INT_DECLARATION, VT_UINT_DECLARATION);
		oss << "Variable '" << d->get_lhs_name() << "' defined in "<< d->get_line_number() 
			<< " (decl id:" << d->get_id() << ") has value " << value_to_string(v) << ".\n";
	}
	if( ((exclusive_read*)astv->AI)->list_of_assertions.size()>0 ) {
		int counter=1;
		oss << "list of assertions:\n";
		for(set< ast_vertex* >::const_iterator it=((exclusive_read*)astv->AI)->list_of_assertions.begin();it!=((exclusive_read*)astv->AI)->list_of_assertions.end();it++) {
			oss << "  " << counter++ << ". '" << test_expr_to_string(*it) << "'\n";
		}
	}
	PRINT_LINE;
	oss << endl << endl;
	return oss.str();
}


void 
exclusive_read::collect_read_accesses(ast_vertex* v, list<ast_vertex*> &l)
{
	if( v->get_type()==VT_SPL_identifier ) { l.push_back(v); return; }
	FOR_EACH_it_IN(v->get_children()) { collect_read_accesses(*it, l); }
}


void 
exclusive_read::apply_distributivity(ast_vertex*& v) 
{ 
	for(list<ast_vertex*>::iterator it=v->get_children().begin();it!=v->get_children().end();it++)
	{ apply_distributivity(*it); }
	if(    v->get_type()==VT_SPL_int_add 
        && FIRSTOF(v)->get_type()==VT_SPL_int_mul
        && ( SECONDOF(v)->get_type()==VT_SPL_int_add  ||  SECONDOF(v)->get_type()==VT_SPL_int_sub )
        && FIRSTOF(FIRSTOF(v))->get_type()==VT_SPL_int_sub
        && SECONDOF(FIRSTOF(v))->get_type()==VT_SPL_identifier
        && FIRSTOF(SECONDOF(v))->get_type()==VT_SPL_identifier
        && SECONDOF(SECONDOF(v))->get_type()==VT_SPL_int_const
        && SECONDOF(FIRSTOF(v))->get_str()==FIRSTOF(SECONDOF(v))->get_str()
		&& FIRSTOF(FIRSTOF(FIRSTOF(v)))->get_type()==VT_SPL_int_mul
		&& SECONDOF(FIRSTOF(FIRSTOF(v)))->get_type()==VT_SPL_int_const  &&  SECONDOF(FIRSTOF(FIRSTOF(v)))->get_str()=="1"
		&& FIRSTOF(FIRSTOF(FIRSTOF(FIRSTOF(v))))->get_type()==VT_SPL_int_add
		&& SECONDOF(FIRSTOF(FIRSTOF(FIRSTOF(v))))->get_type()==VT_SPL_int_div
		&& ::operator_equal(FIRSTOF(FIRSTOF(FIRSTOF(FIRSTOF(FIRSTOF(v))))), THREAD_ID) 
		&& SECONDOF(FIRSTOF(FIRSTOF(FIRSTOF(FIRSTOF(v)))))->get_type()==VT_SPL_int_const  
		&& SECONDOF(FIRSTOF(FIRSTOF(FIRSTOF(FIRSTOF(v)))))->get_str()=="1"
        && FIRSTOF(SECONDOF(FIRSTOF(FIRSTOF(FIRSTOF(v)))))->get_type()==VT_SPL_identifier
        && SECONDOF(FIRSTOF(v))->get_str()==FIRSTOF(SECONDOF(FIRSTOF(FIRSTOF(FIRSTOF(v)))))->get_str()
		&& ::operator_equal(SECONDOF(SECONDOF(FIRSTOF(FIRSTOF(FIRSTOF(v))))), NUM_OF_THREADS) 
	  ) 
	{
		// Replace expression  ( (tid+1)*n/p - 1 ) * n + (n+c2)  by  expression  (tid+1)*n/p*n + c2
		ast_vertex* subst = FIRSTOF(FIRSTOF(v)) ;
		ast_vertex* mul = new ast_vertex(0, VT_SPL_int_mul, "");
		add_child(mul, FIRSTOF(subst));
		add_child(mul, SECONDOF(FIRSTOF(v))->clone());
		subst->replace(mul);
		subst = new ast_vertex(0, SECONDOF(v)->get_type(), "");
		add_child(subst, mul);
		add_child(subst, SECONDOF(SECONDOF(v)));
		v = subst;
	}
}

bool 
exclusive_read::is_array_index_thread_unique(ast_vertex* v, ER_DELTA_TYPE &D)
{
	er_basic_type lower_bound, upper_bound, min;

	if (v->get_type()==VT_SPL_identifier && sym_tbl.is_int(v->get_str())) {
		ast_vertex* decl = sym_tbl.get_decl(v->get_str());
		assert( D.count(decl)>0 );
		ER_DOMAIN_TYPE d = D[decl];
		lower_bound=d.first;             upper_bound=d.second;
		if(::operator_equal(lower_bound, upper_bound) && ::operator_equal(upper_bound, THREAD_ID)) 
			return true;
	}



	// Test if  ub(t) < lb(t+1)
	lower_bound=get_lower_bound_of_t_plus_one(v, D);
	upper_bound=get_upper_bound_of_t(v, D);
	lower_bound->normalize_arithmetic_expression();
	upper_bound->normalize_arithmetic_expression();
	apply_distributivity(upper_bound);
	upper_bound->normalize_arithmetic_expression();
	if( is_THREAD_ID_present(lower_bound) && is_THREAD_ID_present(upper_bound) ) {
		if( ! ::operator_equal(lower_bound, upper_bound) ) { // ub(t) must be unequal to lb(t+1) 
			operator_min(upper_bound, lower_bound, min);
			if(min==upper_bound)
				return true;
		}
	}
	return false;
}

bool 
exclusive_read::is_THREAD_ID_present(ast_vertex* v)
{
	bool rc=false;
	if( ::operator_equal(v, THREAD_ID) ) { rc=true; }
	else {
		FOR_EACH_it_IN(v->get_children()) {
			rc |= is_THREAD_ID_present(*it);
		}
	}
	return rc;
}


void
exclusive_read::replace_TID_by_TID_plus_one(er_basic_type v)
{
	switch(v->get_type()) {
		case VT_SPL_array_index: 
			assert(0); break;
		case VT_SPL_int_add:
		case VT_SPL_int_sub:
		case VT_SPL_int_mul:
		case VT_SPL_int_div:
			assert(v->get_children().size()==2);
			assert(FIRSTOF(v));  assert(SECONDOF(v));
			replace_TID_by_TID_plus_one(FIRSTOF(v));
			replace_TID_by_TID_plus_one(SECONDOF(v));
			break;
		case VT_SPL_identifier: 
			break;
		case VT_SPL_int_const:
			if( ::operator_equal(v, THREAD_ID) ) { 
				ast_vertex* tid=v;
				ast_vertex* one= new ast_vertex(0, VT_SPL_int_const, "1"); assert(one);
				ast_vertex* add= new ast_vertex(0, VT_SPL_int_add, "");    assert(add);
				add_child(add, tid->clone());
				add_child(add, one);
				tid->replace(add);
			}
			break;
		default:
			UNKNOWN_TYPE(v); break;
	}
}

er_basic_type exclusive_read::get_lower_bound_of_t(er_basic_type v, ER_DELTA_TYPE &D) 
{ 
	er_basic_type l;
	er_basic_type r;
	er_basic_type op;
	switch(v->get_type()) {
		case VT_SPL_array_index: assert(0); break;
		case VT_SPL_int_add:
		case VT_SPL_int_sub:
		case VT_SPL_int_mul:
		case VT_SPL_int_div:
			l = get_lower_bound_of_t(FIRSTOF(v), D);
			r = get_lower_bound_of_t(SECONDOF(v), D);
			op = new ast_vertex(0, v->get_type(), "");
			add_child(op, l); 
			add_child(op, r);
			return op;
		case VT_SPL_identifier: {
			assert( sym_tbl.is_int(v->get_str()) );
			ast_vertex* decl = sym_tbl.get_decl(v->get_str());
			assert( D.count(decl)>0 );
			ER_DOMAIN_TYPE d = D[decl];
			er_basic_type lower_bound=d.first;
			return lower_bound;
		}
		case VT_SPL_int_const:
			return v->clone();
		default:
			UNKNOWN_TYPE(v);
			return NULL;
	}
}


er_basic_type 
exclusive_read::get_upper_bound_of_t(er_basic_type v, ER_DELTA_TYPE &D) 
{ 
	er_basic_type l;
	er_basic_type r;
	er_basic_type op;
	switch(v->get_type()) {
		case VT_SPL_array_index: assert(0); break;
		case VT_SPL_int_add:
		case VT_SPL_int_sub:
		case VT_SPL_int_mul:
		case VT_SPL_int_div:
			l = get_upper_bound_of_t(FIRSTOF(v), D);
			r = get_upper_bound_of_t(SECONDOF(v), D);
			op = new ast_vertex(0, v->get_type(), "");
			add_child(op, l); 
			add_child(op, r);
			return op;
		case VT_SPL_identifier: {
			assert( sym_tbl.is_int(v->get_str()) );
			ast_vertex* decl = sym_tbl.get_decl(v->get_str());
			assert( D.count(decl)>0 );
			ER_DOMAIN_TYPE d = D[decl];
			er_basic_type upper_bound=d.second;
			return upper_bound;
		}
		case VT_SPL_int_const:
			return v->clone();
		default:
			UNKNOWN_TYPE(v);
			return NULL;
	}
}

er_basic_type
exclusive_read::get_upper_bound_of_t_minus_one(er_basic_type v, ER_DELTA_TYPE &D)
{
	er_basic_type l;
	er_basic_type r;
	er_basic_type op;
	switch(v->get_type()) {
		case VT_SPL_array_index: assert(0); break;
		case VT_SPL_int_add:
		case VT_SPL_int_sub:
		case VT_SPL_int_mul:
		case VT_SPL_int_div:
			l = get_upper_bound_of_t_minus_one(FIRSTOF(v), D);
			r = get_upper_bound_of_t_minus_one(SECONDOF(v), D);
			op = new ast_vertex(0, v->get_type(), "");
			add_child(op, l); 
			add_child(op, r);
			return op;
		case VT_SPL_identifier: {
			assert( sym_tbl.is_int(v->get_str()) );
			ast_vertex* decl = sym_tbl.get_decl(v->get_str());
			assert( D.count(decl)>0 );
			ER_DOMAIN_TYPE d = D[decl];

			if( ::operator_equal(v, THREAD_ID) ) { 
				ast_vertex* tid=v;
				ast_vertex* one= new ast_vertex(0, VT_SPL_int_const, "1"); assert(one);
				ast_vertex* sub= new ast_vertex(0, VT_SPL_int_sub, "");    assert(sub);
				add_child(sub, tid->clone());
				add_child(sub, one);
				tid->replace(sub);
				return sub;
			}
			else {
				return d.second;
			}
		}
		case VT_SPL_int_const:
			return v->clone();
		default:
			UNKNOWN_TYPE(v);
			return NULL;
	}
}

er_basic_type
exclusive_read::get_lower_bound_of_t_plus_one(er_basic_type v, ER_DELTA_TYPE &D)
{
	er_basic_type l;
	er_basic_type r;
	er_basic_type op;
	switch(v->get_type()) {
		case VT_SPL_array_index: assert(0); break;
		case VT_SPL_int_add:
		case VT_SPL_int_sub:
		case VT_SPL_int_mul:
		case VT_SPL_int_div:
			l = get_lower_bound_of_t_plus_one(FIRSTOF(v), D);
			r = get_lower_bound_of_t_plus_one(SECONDOF(v), D);
			op = new ast_vertex(0, v->get_type(), "");
			add_child(op, l); 
			add_child(op, r);
			return op;
		case VT_SPL_identifier: {
			assert( sym_tbl.is_int(v->get_str()) );
			ast_vertex* decl = sym_tbl.get_decl(v->get_str());
			assert( D.count(decl)>0 );
			ER_DOMAIN_TYPE d = D[decl];
			er_basic_type lower_bound=d.first->clone();
			// Replace TID by TID+1 inside of the lower bound
			replace_TID_by_TID_plus_one( lower_bound );
			return lower_bound;
		}
		case VT_SPL_int_const:
			return v->clone();
		default:
			UNKNOWN_TYPE(v);
			return NULL;
	}
}

bool 
exclusive_read::is_interval_top(const ER_DOMAIN_TYPE &d)
{
	er_basic_type l=d.first;
	er_basic_type r=d.second;
	if( ::operator_equal(l, MINUS_INFINITY) && ::operator_equal(r, PLUS_INFINITY) )
		return true;
	return false;
}


/*
   v is the subtree that is going to be replaced according what node we encounter. 2nd parameter d is the value that is going to be part
   of all the multiplications. This means we have ( .. + .. ) * d and we traverse now the expression in brackets.
   */
void replace_according_to_distribution_rule(er_basic_type v, er_basic_type d)
{
	replace_according_to_distribution_rule(FIRSTOF(v), d);
	replace_according_to_distribution_rule(SECONDOF(v), d);
	switch( v->get_type() ) 
	{
	case VT_SPL_identifier: 
		return;
	case VT_SPL_int_add: 
	case VT_SPL_int_sub: 
	{
		ast_vertex* first_child = FIRSTOF(v);
		ast_vertex* second_child = SECONDOF(v);
		v->clear_children();
		for(int i=0;i<2;i++) {
			ast_vertex* node;
			ast_vertex* mul;
			switch(i) {
			case 1: node = second_child;
			case 0: node = first_child;
			}
			if( node->get_type()==VT_SPL_identifier ) {
				mul = new ast_vertex(node->get_line_number(), VT_SPL_int_mul, "");
				ast_vertex* lhs=node;
				ast_vertex* rhs=d;
				add_child(mul, lhs);
				add_child(mul, rhs);
			}
			else 
				mul = node;
			add_child(v, mul);
		}
		return;
	}
	case VT_SPL_int_mul:
	case VT_SPL_int_div:
	{
		ast_vertex* first_child = FIRSTOF(v);
		ast_vertex* second_child = SECONDOF(v);
		// Assume that there is not another distribution in the subtree
		assert(first_child->get_type()!=VT_SPL_int_add && first_child->get_type()!=VT_SPL_int_sub) ; 
		assert(second_child->get_type()!=VT_SPL_int_add && second_child->get_type()!=VT_SPL_int_sub) ; 

		ast_vertex* mul = new ast_vertex(v->get_line_number(), VT_SPL_int_mul, "");
		ast_vertex* lhs=first_child;
		ast_vertex* rhs=d;
		add_child(mul, lhs);
		add_child(mul, rhs);
		v->clear_children();
		add_child(v, mul);
		add_child(v, second_child);
		return;
	}
	default:
		UNKNOWN_TYPE(v);
	}
}




void exclusive_read::adjust_AI_depending_on_assertion(exclusive_read& ai, ast_vertex* current_decl, ER_DOMAIN_TYPE interval_of_var, ast_vertex* test_expression)
{
	string current_var_name=current_decl->get_lhs_name(); 
	ast_vertex* lower_boundary = interval_of_var.first;         // Look what the interval of i is, e.g. [u:infinity].
	ast_vertex* upper_boundary = interval_of_var.second;
	#if DEBUG
	cerr << "Current variable is: " << current_var_name << " with interval " <<  value_to_string(interval_of_var) << endl;
	#endif 

	// First case is that we have an assertion that e.g. j<k but the upper boundaries of j is bigger than the one for k.
	if(     upper_boundary!=PLUS_INFINITY   
			&&	test_expression->get_type()==VT_SPL_leq  
			&&  FIRSTOF(test_expression)->get_type()==VT_SPL_identifier              // lhs of test is identifier
			&&  current_var_name==FIRSTOF(test_expression)->get_str()              // lhs is current variable
			&&  SECONDOF(test_expression)->get_type()==VT_SPL_identifier           // rhs of test is some identifier
			&&  // the rhs of the test expression does not have infinity as upper boundary
			ai.D[ sym_tbl.get_decl( SECONDOF(test_expression)->get_str() ) ].second!=PLUS_INFINITY
	  )
	{
		#if DEBUG
		cerr << "DFA_OF_CONDITIONAL_BRANCHES: case identifier <= identifier and both upper boundaries are not infinity.\n" ;
		#endif
		er_basic_type test_lhs_upper_bound = upper_boundary;
		er_basic_type test_rhs_upper_bound = ai.D[ sym_tbl.get_decl( SECONDOF(test_expression)->get_str() ) ].second;
		er_basic_type min = NULL;
		operator_min(test_lhs_upper_bound, test_rhs_upper_bound, min);
		#if DEBUG
		cerr << " operator_min result: " << dag_to_string(min) << endl;
		#endif 
		if( ::operator_equal(min, test_rhs_upper_bound) ) {
			#if DEBUG
			cerr << " upper bound must be adjusted since it does not fulfill the assertions." << endl;
			#endif 
			operator_min(lower_boundary, test_rhs_upper_bound, min);
			if(::operator_equal(lower_boundary, test_rhs_upper_bound)  ||  ::operator_equal(min, lower_boundary))  
				ai.D[current_decl] = make_pair( lower_boundary, test_rhs_upper_bound );
		}
	}
	else if(     upper_boundary!=PLUS_INFINITY   
			&&	test_expression->get_type()==VT_SPL_lower
			&&  FIRSTOF(test_expression)->get_type()==VT_SPL_identifier              // lhs of test is identifier
			&&  current_var_name==FIRSTOF(test_expression)->get_str()              // lhs is current variable
			&&  SECONDOF(test_expression)->get_type()==VT_SPL_identifier           // rhs of test is some identifier
			&&  // the rhs of the test expression does not have infinity as upper boundary
			ai.D[ sym_tbl.get_decl( SECONDOF(test_expression)->get_str() ) ].second!=PLUS_INFINITY
		   )
	{
		#if DEBUG
		cerr << "DFA_OF_CONDITIONAL_BRANCHES: case identifier < identifier and both upper boundaries are not infinity.\n" ;
		#endif
		er_basic_type test_lhs_upper_bound = upper_boundary;
		er_basic_type test_rhs_upper_bound = ai.D[ sym_tbl.get_decl( SECONDOF(test_expression)->get_str() ) ].second;
		er_basic_type min = NULL;
		operator_min(test_lhs_upper_bound, test_rhs_upper_bound, min);
		#if DEBUG
		cerr << " operator_min result: " << dag_to_string(min) << endl;
		#endif 
		if(::operator_equal(test_lhs_upper_bound, test_rhs_upper_bound)  ||  ::operator_equal(min, test_rhs_upper_bound)) { 
			// if rhs.upper_bound = lhs.upper_bound  or   if rhs.upper.bound < lhs.upper_bound 
			#if DEBUG
			cerr << " upper bound must be adjusted since it does not fulfill the assertions." << endl;
			#endif 
			if( test_rhs_upper_bound->get_type()==VT_SPL_int_sub  &&  SECONDOF(test_rhs_upper_bound)->get_type()==VT_SPL_int_const ) 
			{
				ast_vertex* constant = SECONDOF(test_rhs_upper_bound);
				istringstream iss(constant->get_str());
				long constant_val; iss >> constant_val;
				constant_val++;  // Since we have the test if '<' we take one smaller than the upper boundary, e.g. n-4 leads to n-5.
				ostringstream oss;
				oss << constant_val;
				er_basic_type min = NULL;
				ast_vertex* sub = new ast_vertex(0, VT_SPL_int_sub, ""); 
				ast_vertex* constant_astv = new ast_vertex(0, VT_SPL_int_const, oss.str()); 
				add_child(sub, FIRSTOF(test_rhs_upper_bound));
				add_child(sub, constant_astv);
				operator_min(lower_boundary, sub, min);
				if(::operator_equal(lower_boundary, sub)  ||  ::operator_equal(min, lower_boundary))  
					ai.D[current_decl] = make_pair( lower_boundary, sub);
			}
			else {
				ast_vertex* sub = new ast_vertex(0, VT_SPL_int_sub, ""); 
				ast_vertex* one = new ast_vertex(0, VT_SPL_int_const, "1"); 
				add_child(sub, test_rhs_upper_bound);
				add_child(sub, one);
				operator_min(lower_boundary, sub, min);
				if(::operator_equal(lower_boundary, sub)  ||  ::operator_equal(min, lower_boundary))  
					ai.D[current_decl] = make_pair( lower_boundary, sub );
			}
		}
	}
	else if(    (upper_boundary==PLUS_INFINITY  ||  upper_boundary->get_type()==VT_SPL_int_const)
			&& ( test_expression->get_type()==VT_SPL_leq  || test_expression->get_type()==VT_SPL_lower )
			&&   SECONDOF(test_expression)->get_type()==VT_SPL_int_const  
		   )
	{
		#if DEBUG
		cerr << "DFA_OF_CONDITIONAL_BRANCHES: (upper_boundary==PLUS_INFINITY  ||  upper_boundary->get_type()==VT_SPL_int_const).\n" ;
		#endif 
		long test_expr_constant; 
		istringstream iss( SECONDOF(test_expression)->get_str() );
		iss >> test_expr_constant;
		if( upper_boundary->get_type()==VT_SPL_int_const  &&   is_common_constant(upper_boundary) ) {
			istringstream _iss( upper_boundary->get_str() );
			long upper_boundary_constant; _iss >> upper_boundary_constant;
			if( test_expression->get_type()==VT_SPL_leq  &&  upper_boundary_constant>test_expr_constant ) 
				ai.D[current_decl] = make_pair( lower_boundary, SECONDOF(test_expression)->clone() );
			else if( test_expression->get_type()==VT_SPL_lower  &&  upper_boundary_constant>=test_expr_constant ) {
				test_expr_constant--;
				ostringstream oss; oss << test_expr_constant;
				ast_vertex* new_upper_bound = new ast_vertex(0, VT_SPL_int_const, oss.str()) ;
				ai.D[current_decl] = make_pair( lower_boundary, new_upper_bound );
			}
		}
		else if ( upper_boundary==PLUS_INFINITY ){
			if( test_expression->get_type()==VT_SPL_leq ) 
				ai.D[current_decl] = make_pair( lower_boundary, SECONDOF(test_expression)->clone() );
			else if( test_expression->get_type()==VT_SPL_lower ) {
				test_expr_constant--;
				ostringstream oss; oss << test_expr_constant;
				ast_vertex* new_upper_bound = new ast_vertex(0, VT_SPL_int_const, oss.str()) ;
				ai.D[current_decl] = make_pair( lower_boundary, new_upper_bound );
			}
		}
	}
	// Look for upper boundary
	else if(    upper_boundary==PLUS_INFINITY          // If the current variable has infinity has its upper boundary, 
			// we examine the test expression.
			&& ( test_expression->get_type()==VT_SPL_leq  || test_expression->get_type()==VT_SPL_lower )
			// If test expression is <= or < 
			&&   current_var_name==FIRSTOF(test_expression)->get_str()       // Test if current var is equal to the left hand side of 
			&&   SECONDOF(test_expression)->get_type()==VT_SPL_identifier    // the test expression and if rhs of test expression 
			&&   sym_tbl.is_int( SECONDOF(test_expression)->get_str() )      // is an identifier that has an upper bound unequal to 
			&&  ai.D[ sym_tbl.get_decl( SECONDOF(test_expression)->get_str() ) ].second!=PLUS_INFINITY  // infinity.
		   )
	{
		#if DEBUG
		cerr << "DFA_OF_CONDITIONAL_BRANCHES: case i<j and i has infinity as upper bound and j has not.\n" ;
		if( test_expression->get_type()==VT_SPL_leq )
			cerr << "Test expression is " << current_var_name << " <= " << SECONDOF(test_expression)->get_str() << endl;
		else
			cerr << "Test expression is " << current_var_name << " <  " << SECONDOF(test_expression)->get_str() << endl;
		#endif
		//er_basic_type lower_bound_of_test_rhs = ai.D[ sym_tbl.get_decl( SECONDOF(test_expression)->get_str() ) ].first;
		er_basic_type upper_bound_of_test_rhs = ai.D[ sym_tbl.get_decl( SECONDOF(test_expression)->get_str() ) ].second;
		er_basic_type min = NULL;
		if( test_expression->get_type()==VT_SPL_leq ) {  // In case of test operator <= we can take the upper boundary as it is.
			operator_min(lower_boundary, upper_bound_of_test_rhs, min);
			if(::operator_equal(lower_boundary, upper_bound_of_test_rhs)  ||  ::operator_equal(min, lower_boundary))  
				ai.D[current_decl] = make_pair( lower_boundary, upper_bound_of_test_rhs );
		}
		else {    // In case of operator < we reduce the upper boundary by one.
			// In case that we have an upper bound (n-4) => (n-5)
			// Otherwise we create an expression (upperbound-1)
			if( upper_bound_of_test_rhs->get_type()==VT_SPL_int_sub  &&  SECONDOF(upper_bound_of_test_rhs)->get_type()==VT_SPL_int_const ) 
			{
				ast_vertex* constant = SECONDOF(upper_bound_of_test_rhs);
				istringstream iss(constant->get_str());
				long constant_val; iss >> constant_val;
				constant_val++;  // Since we have the test if '<' we take one smaller than the upper boundary, e.g. n-4 leads to n-5.
				ostringstream oss;
				oss << constant_val;
				ast_vertex* sub = new ast_vertex(0, VT_SPL_int_sub, ""); 
				ast_vertex* constant_astv = new ast_vertex(0, VT_SPL_int_const, oss.str()); 
				add_child(sub, FIRSTOF(upper_bound_of_test_rhs));
				add_child(sub, constant_astv);
				operator_min(lower_boundary, sub, min);
				if(::operator_equal(lower_boundary, sub)  ||  ::operator_equal(min, lower_boundary))
					ai.D[current_decl] = make_pair( lower_boundary, sub);
			}
			else {
				ast_vertex* sub = new ast_vertex(0, VT_SPL_int_sub, ""); 
				ast_vertex* one = new ast_vertex(0, VT_SPL_int_const, "1"); 
				add_child(sub, upper_bound_of_test_rhs);
				add_child(sub, one);
				operator_min(lower_boundary, sub, min);
				if(::operator_equal(lower_boundary, sub)  ||  ::operator_equal(min, lower_boundary))
					ai.D[current_decl] = make_pair( lower_boundary, sub);
			}
		}
	}
}
