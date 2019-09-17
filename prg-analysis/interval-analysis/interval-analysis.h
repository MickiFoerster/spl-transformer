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

#ifndef _INTERVAL_ANALYSIS_H_
#define _INTERVAL_ANALYSIS_H_

#include <climits>
#include "ast_vertex.h"
#include "AST.h"

typedef  long   				         ia_basic_type  ;
#define  IA_DOMAIN_TYPE     	         pair<long, long>  
#define  IA_DOMAIN_TYPE_BOTTOM     	     make_pair(BOTTOM, BOTTOM)  
#define  IA_DOMAIN_TYPE_TOP     	     make_pair(0, PLUS_INFINITY)  
#define  IA_DELTA_TYPE     	             map< ast_vertex*, IA_DOMAIN_TYPE > 
#define  IA_FOR_EACH_it_IN_DOMAIN(D)     for(IA_DELTA_TYPE::const_iterator it=D.begin();it!=D.end();it++)
#define  IA_RETURN_BOTTOM_WHEN_l_OR_r_IS_BOTTOM(LEFT,RIGHT)     \
				{ \
					IA_DOMAIN_TYPE bottom = IA_DOMAIN_TYPE_BOTTOM;  \
					if( LEFT==bottom || RIGHT==bottom )  return bottom;    \
				}

class interval_analysis {
private:
	static vector< ast_vertex* > equations;
public:
	enum special_values {
		PLUS_INFINITY=INT_MAX,
		TOP=INT_MAX-1,
		BOTTOM=INT_MIN,
		THREAD_ID,
		NUM_OF_THREADS,
	};

	interval_analysis(): astv(NULL) {};
	interval_analysis(ast_vertex *p): astv(p) 
	{
		D = ((interval_analysis*)p->AI)->D;
	};

	ast_vertex *astv;
	set<ast_vertex*>  data_flow_predecessors;
	IA_DELTA_TYPE D;	

	static void init_prg_analysis(ast_vertex *astv) ;
	static void delete_prg_analysis(ast_vertex *astv) ;
	static IA_DOMAIN_TYPE operator_join(const IA_DOMAIN_TYPE &l, const IA_DOMAIN_TYPE &r) ;
	static IA_DOMAIN_TYPE operator_widening(const IA_DOMAIN_TYPE &l, const IA_DOMAIN_TYPE &r) ;
	static void operator_join(interval_analysis& l, interval_analysis& r, interval_analysis& result, bool widening) ;
	static bool operator_equal(interval_analysis& l, interval_analysis& r) ;
	static IA_DOMAIN_TYPE operator_add(const IA_DOMAIN_TYPE &l, const IA_DOMAIN_TYPE &r) ;
	static IA_DOMAIN_TYPE operator_sub(const IA_DOMAIN_TYPE &l, const IA_DOMAIN_TYPE &r) ;
	static IA_DOMAIN_TYPE operator_mul(const IA_DOMAIN_TYPE &l, const IA_DOMAIN_TYPE &r) ;
	static IA_DOMAIN_TYPE operator_div(const IA_DOMAIN_TYPE &l, const IA_DOMAIN_TYPE &r) ;
	static void print_results() ;
	static const string value_to_string(IA_DOMAIN_TYPE l) ;
	static const string AI_to_string(ast_vertex *astv) ;
	static const string basic_type_value_to_string(ia_basic_type btv) ;

	static void entry_point_for_prg_analysis() ;
	static IA_DOMAIN_TYPE eval(ast_vertex *assign, ast_vertex *v) ;
	static void phi(void *argv)  ;
}  ;


#endif
