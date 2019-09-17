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

#ifndef _EXCLUSIVE_READ_H_
#define _EXCLUSIVE_READ_H_

#include <climits>
#include "ast_vertex.h"
#include "AST.h"

#define DEBUG                                        0
#define NARROWING                                    1
#define DFA_OF_CONDITIONAL_BRANCHES                  1

#define PRINT_LINE  	for(int _i=0;_i<80;_i++) oss << "="; oss << "\n"

typedef  ast_vertex*   				     er_basic_type  ;
#define  ER_DOMAIN_TYPE     	         pair<er_basic_type, er_basic_type>  
#define  ER_DOMAIN_TYPE_BOTTOM     	     make_pair(BOTTOM, BOTTOM)  
#define  ER_DOMAIN_TYPE_TOP     	     make_pair(MINUS_INFINITY, PLUS_INFINITY)  
#define  ER_DELTA_TYPE     	             map< ast_vertex*, ER_DOMAIN_TYPE > 
#define  ER_FOR_EACH_it_IN_DOMAIN(D)     for(ER_DELTA_TYPE::const_iterator it=D.begin();it!=D.end();it++)
#define  ER_RETURN_BOTTOM_WHEN_l_OR_r_IS_BOTTOM(LEFT,RIGHT)     \
				{ \
					ER_DOMAIN_TYPE bottom = ER_DOMAIN_TYPE_BOTTOM;  \
					if( LEFT==bottom || RIGHT==bottom )  return bottom;    \
				}

class exclusive_read {
private:
	static vector< ast_vertex* > equations;
	static bool widening_was_used;
public:
	exclusive_read(): astv(NULL) {};
	exclusive_read(ast_vertex* p, ast_vertex* t): astv(p) , target(t)
	{
		D = ((exclusive_read*)p->AI)->D;
	};

	ast_vertex* astv; // associated AST vertex
	ast_vertex* target; // Caller who is target for the current AI
	set<ast_vertex*>  data_flow_predecessors;
	set< ast_vertex* > list_of_assertions;
	ER_DELTA_TYPE D;	

	static map< ast_vertex* , bool >  is_memory_read_exclusive;
	static void init_prg_analysis(ast_vertex *astv) ;
	static void delete_prg_analysis(ast_vertex *astv) ;
	static ER_DOMAIN_TYPE operator_join(const ER_DOMAIN_TYPE &l, const ER_DOMAIN_TYPE &r) ;
	static ER_DOMAIN_TYPE operator_widening(exclusive_read& ai, 
		                                    ast_vertex *decl,
		                                    ER_DOMAIN_TYPE &l, 
								            ER_DOMAIN_TYPE &r
								           ) ;
	static bool operator_lower(const ER_DOMAIN_TYPE &l, const ER_DOMAIN_TYPE &r) ;
	static void operator_join(exclusive_read& l, exclusive_read& r, exclusive_read& result, bool widening) ;
	static bool operator_equal(exclusive_read& l, exclusive_read& r) ;
	static ER_DOMAIN_TYPE operator_add(const ER_DOMAIN_TYPE &l, const ER_DOMAIN_TYPE &r) ;
	static ER_DOMAIN_TYPE operator_sub(const ER_DOMAIN_TYPE &l, const ER_DOMAIN_TYPE &r) ;
	static ER_DOMAIN_TYPE operator_mul(const ER_DOMAIN_TYPE &l, const ER_DOMAIN_TYPE &r) ;
	static ER_DOMAIN_TYPE operator_div(const ER_DOMAIN_TYPE &l, const ER_DOMAIN_TYPE &r) ;
	static void print_results(bool intermediate_result=0) ;
	static const string value_to_string(ER_DOMAIN_TYPE l) ;
	static const string AI_to_string(ast_vertex *astv) ;
	static const string basic_type_value_to_string(er_basic_type btv) ;
	static void operator_min( er_basic_type l, er_basic_type r, er_basic_type &min);
	static void operator_max( er_basic_type l, er_basic_type r, er_basic_type &max);
	static er_basic_type min_S_or_const_plus_S(er_basic_type l, er_basic_type r) ;
	static er_basic_type min_S_or_S_minus_const(er_basic_type l, er_basic_type r);
	static er_basic_type special_case_001(er_basic_type l, er_basic_type r);
	static er_basic_type special_case_002(er_basic_type l, er_basic_type r);
	static const string dag_to_string(ast_vertex* v);
	static const string test_expr_to_string(ast_vertex* v);
	static er_basic_type operator_arithmetic(er_basic_type l, er_basic_type r, const char op) ;
	static bool is_common_constant(er_basic_type c) ;
	static void collect_read_accesses(ast_vertex* v, list<ast_vertex*> &l);
	static bool is_array_index_thread_unique(ast_vertex* v, ER_DELTA_TYPE &D) ;
	static bool is_THREAD_ID_present(ast_vertex* v) ;
	static er_basic_type get_lower_bound_of_t_plus_one(er_basic_type lower_bound_of_t, ER_DELTA_TYPE &D) ;
	static er_basic_type get_upper_bound_of_t_minus_one(er_basic_type upper_bound_of_t, ER_DELTA_TYPE &D) ;
	static er_basic_type get_lower_bound_of_t(er_basic_type lower_bound_of_t, ER_DELTA_TYPE &D) ;
	static er_basic_type get_upper_bound_of_t(er_basic_type lower_bound_of_t, ER_DELTA_TYPE &D) ;
	static void replace_TID_by_TID_plus_one(er_basic_type v);
	static bool is_interval_top(const ER_DOMAIN_TYPE &d) ;
	static void ensure_monotoncity(exclusive_read& l, exclusive_read& r) ;
	static void apply_distributivity(ast_vertex*& v) ;
	static void adjust_AI_depending_on_assertion(exclusive_read& ai, 
			                                     ast_vertex* current_decl, 
												 ER_DOMAIN_TYPE interval_of_var, 
												 ast_vertex* test_expression);

	static void entry_point_for_prg_analysis() ;
	static ER_DOMAIN_TYPE eval(ast_vertex *assign, ast_vertex *v) ;
	static void phi(void *argv)  ;
}  ;


#endif
