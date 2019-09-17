#ifndef _CONSTANT_PROPAGATION_
#define _CONSTANT_PROPAGATION_

#include <climits>
#include "ast_vertex.h"
#include "AST.h"

#define  CP_DOMAIN_TYPE     	long 
#define  CP_DELTA_TYPE     	    map< ast_vertex*, CP_DOMAIN_TYPE > 
#define  CP_FOR_EACH_it_IN_DOMAIN(D)  for(CP_DELTA_TYPE::const_iterator it=D.begin();it!=D.end();it++)

enum special_values {
	TOP=INT_MAX,
	BOTTOM=INT_MIN,
	THREAD_ID,
	NUM_OF_THREADS
};


class constant_propagation {
private:
	static vector< ast_vertex* > equations;
public:
	constant_propagation(): astv(NULL) {};
	constant_propagation(ast_vertex *p): astv(p) 
	{
		D = ((constant_propagation*)p->AI)->D;
	};

	ast_vertex *astv;
	set<ast_vertex*>  data_flow_predecessors;
	CP_DELTA_TYPE D;	


	static void init_prg_analysis(ast_vertex *astv) ;
	static void delete_prg_analysis(ast_vertex *astv) ;
	static CP_DOMAIN_TYPE operator_join(const CP_DOMAIN_TYPE &l, const CP_DOMAIN_TYPE &r) ;
	static void operator_join(constant_propagation& l, constant_propagation& r, constant_propagation& result) ;
	static bool operator_equal(constant_propagation& l, constant_propagation& r) ;
	static void print_results() ;
	static const string value_to_string(CP_DOMAIN_TYPE l) ;
	static void entry_point_for_prg_analysis() ;
	//static void iota(void *argv) ;
	static void phi(void *argv)  ;
	static CP_DOMAIN_TYPE  eval(ast_vertex *v) ;
} ;



#endif
