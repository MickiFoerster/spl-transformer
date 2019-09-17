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
#include <map>
#include <cassert>
#include <cstdlib>

#include "constant-propagation.h"

using namespace std;

const string RESULT_FILE = "constant-propagation-results.txt";
#define PRINT_LINE  	for(int _i=0;_i<80;_i++) ofs << "="; ofs << "\n"

vector< ast_vertex* > constant_propagation::equations;

void
constant_propagation::phi(void *argv) 
{
	constant_propagation *ai = (constant_propagation*) argv;

	// Parameter ai is meant as temporary data where the astv data component is associated with the ast vertex 
	// that should be processed by phi function.
	// Each ast vertex has a phi and AI component. The associated AI information should NOT be changed here in phi.
	assert(ai->astv);

	switch( ai->astv->get_type() ) {
		case VT_INT_DECLARATION:
			ai->D[ ai->astv ] = BOTTOM; 
			break;
		case VT_SPL_int_assign:
		{
			ast_vertex *lhs = FIRSTOF(ai->astv);
			ASSUME_TYPE(lhs, VT_SPL_identifier);
			const string lhs_name = lhs->get_str();
			ast_vertex *decl = sym_tbl.get_decl(lhs_name);
			ai->D[ decl ] = eval( SECONDOF(ai->astv) ); 
			break;
		}
		default:
			break;
	}
}

long 
constant_propagation::eval(ast_vertex *v) 
{
	switch(v->get_type()) {
		case VT_OMP_RUNTIME_ROUTINE:
			if( v->get_str()=="omp_get_thread_num()" )
				return THREAD_ID;
			else if( v->get_str()=="omp_get_num_threads()" )
				return NUM_OF_THREADS;
			assert(0);
			return 0;
		case VT_SPL_int_const:
			return atoi(v->get_str().c_str());
		case VT_SPL_identifier:
		{
			const string identifier = v->get_str();
			ast_vertex *decl = sym_tbl.get_decl(identifier);
			return ((constant_propagation*)v->AI)->D[decl];
		}
		default:
			return TOP;
	}
}

void 
constant_propagation::entry_point_for_prg_analysis() 
{
	// Init cfg_entry and cfg_exit
	ast.cfg_entry->phi = &phi;	
	ast.cfg_exit->phi = &phi;	

	// Initializing all nodes with phi and AI data
	init_prg_analysis(ast.get_root());

	bool fixpoint_reached=false;
	// Perform program analysis 
	int counter=0;
	while( !fixpoint_reached ) {
		counter++;
		fixpoint_reached=true;
		for(unsigned i=0;i<equations.size();i++) {
			constant_propagation *ai = (constant_propagation*)equations[i]->AI;
			assert(ai->astv==equations[i]);
			assert(equations[i]->phi);

			constant_propagation old_ai( equations[i] ) ;  // Last result of equation.
			constant_propagation new_ai ;
			new_ai.astv = equations[i];
			if(ai->astv!=ast.cfg_entry)
				assert( ai->data_flow_predecessors.size()>0 );
			for(set<ast_vertex*>::const_iterator it=ai->data_flow_predecessors.begin();it!=ai->data_flow_predecessors.end();it++) {
				assert(((constant_propagation*)(*it)->AI)->astv==*it);

				constant_propagation tmp_ai(*it);
				(*it)->phi(&tmp_ai); 
				operator_join(new_ai, tmp_ai, new_ai);
			}
			bool sets_equal = operator_equal(old_ai, new_ai);
			if( !sets_equal ) 
				ai->D = new_ai.D; 
			fixpoint_reached &= sets_equal;
		}
	}
	print_results();

	// Set all phi and AI values back to zero
	for(unsigned i=0;i<equations.size();i++) {
		if(equations[i]->AI) free(equations[i]->AI);
		equations[i]->AI=NULL;
	}
	delete_prg_analysis(ast.get_root());
}

inline void 
constant_propagation::print_results() 
{
	ofstream ofs(RESULT_FILE.c_str()); assert( ofs.good() );
	for(unsigned i=0;i<equations.size();i++) {
		PRINT_LINE;
		ofs << "Node " << equations[i]->get_id() << " ("<< equations[i]->vertex_type_2_string() << "):\n";	
		CP_FOR_EACH_it_IN_DOMAIN(((constant_propagation*)equations[i]->AI)->D) {
			ast_vertex *d=it->first; CP_DOMAIN_TYPE v=it->second;
			ofs << "Variable '" << d->get_lhs_name() << "' declared in " << d->get_id() << " has value " << value_to_string(v) << ".\n";
		}

		PRINT_LINE;
		ofs << endl << endl;
	}
}

inline const string
constant_propagation::value_to_string(long l) 
{
	ostringstream oss;
	switch(l) {
	case TOP: 
		oss << "TOP";
		break;
	case BOTTOM:
		oss << "BOTTOM";
		break;
	case THREAD_ID:
		oss << "THREAD_ID";
		break;
	case NUM_OF_THREADS:
		oss << "NUM_OF_THREADS";
		break;
	default:
		oss << l;
		break;
	}
	return oss.str();
}

bool 
constant_propagation::operator_equal(constant_propagation& l, constant_propagation& r) 
{
	assert(l.astv == r.astv);
	if( l.D.size()==0  &&  r.D.size()==0 ) {
		// cerr << "DEBUG: size from both sets is zero.\n";
		return true;
	}
	if( l.D.size()!=r.D.size() ) {
		// cerr << "DEBUG: size from both sets is different.\n";
		return false;
	}
	CP_FOR_EACH_it_IN_DOMAIN(l.D) {
		ast_vertex *d=it->first; CP_DOMAIN_TYPE v=it->second;
		if( r.D[d]!=v )  {
			// cerr << "DEBUG: element " << value_to_string(r.D[d]) << " and " << value_to_string(v) << " are different.\n";
			return false;
		}
	}
	// cerr << "DEBUG: ai sets are both the same.\n";
	return true;
}


void
constant_propagation::operator_join(constant_propagation& l, 
		                            constant_propagation& r, 
			                        constant_propagation& result) 
{ 
  // Distinguish the cases
  // 1. element is in both sets
  // 2. element is only contained in L
  // 3. element is only contained in R

  // The first loop is for cases one and two.
  CP_FOR_EACH_it_IN_DOMAIN(l.D) { 
  	ast_vertex *d=it->first; CP_DOMAIN_TYPE v=it->second;
	if( r.D.count(d)>0 ) { // case one
		result.D[d] = operator_join( v, r.D[d] );
	}
	else  {  // case two
		result.D[d] = v;
	}
  }
  CP_FOR_EACH_it_IN_DOMAIN(r.D) {  // case three
  	ast_vertex *d=it->first; CP_DOMAIN_TYPE v=it->second;
	if( l.D.count(d)==0 ) {  // Otherwise the element is already handled by the previous loop.
		result.D[d] = v;
	}
  }
}

inline long 
constant_propagation::operator_join(const CP_DOMAIN_TYPE &l, const CP_DOMAIN_TYPE &r) 
{
	if( l==TOP || r==TOP )
		return TOP;
	else if( l==BOTTOM && r==BOTTOM )
		return BOTTOM;
	else if( l!=BOTTOM && r==BOTTOM )
		return l;
	else if( l==BOTTOM && r!=BOTTOM )
		return r;
	else if( l==r )
		return l;
	else if( l!=r )
		return TOP;
	assert(0);
	return 0;
}

void 
constant_propagation::init_prg_analysis(ast_vertex *astv) 
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
				from->AI = new constant_propagation;
				((constant_propagation*) from->AI)->astv = from;
			}
			for(i=0;i<equations.size() && equations[i]!=to;i++) 
				;
			if(i==equations.size()) {
				equations.push_back(to);
				assert( !to->AI );
				to->AI = new constant_propagation;
				((constant_propagation*) to->AI)->astv = to;
			}
			if(to->AI)
				((constant_propagation*) to->AI)->data_flow_predecessors.insert(from);
		}
	}
	FOR_EACH_it_IN(astv->get_children()) {
		init_prg_analysis(*it);
	}
}

void 
constant_propagation::delete_prg_analysis(ast_vertex *astv) 
{
	FOR_EACH_it_IN(astv->get_children()) {
		delete_prg_analysis(*it);
	}
	astv->AI=NULL;
	astv->phi = NULL;	
}
