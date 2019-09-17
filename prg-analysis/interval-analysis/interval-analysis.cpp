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
#include <utility>
#include <map>
#include <cassert>
#include <cstdlib>

#include "interval-analysis.h"

using namespace std;

const string RESULT_FILE = "interval-analysis-results.txt";
#define PRINT_LINE  	for(int _i=0;_i<80;_i++) oss << "="; oss << "\n"

vector< ast_vertex* > interval_analysis::equations;

void 
interval_analysis::entry_point_for_prg_analysis() 
{
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
			interval_analysis *ai = (interval_analysis*)equations[i]->AI;
			assert(ai->astv==equations[i]);
			assert(equations[i]->phi);

			interval_analysis old_ai( equations[i] ) ;  // Last result of equation.
			interval_analysis new_ai ;
			new_ai.astv = equations[i];
			if(ai->astv!=ast.cfg_entry)
				assert( ai->data_flow_predecessors.size()>0 );
			for(set<ast_vertex*>::const_iterator it=ai->data_flow_predecessors.begin();it!=ai->data_flow_predecessors.end();it++) {
				assert(((interval_analysis*)(*it)->AI)->astv==*it);
				interval_analysis tmp_ai(*it);
				(*it)->phi(&tmp_ai); 
				// Here we still use join operation instead 
				// of widening operator. The intervals themselves are
				// joined per widening operator.
				operator_join(new_ai, tmp_ai, new_ai, 1);
			}
			bool sets_equal = operator_equal(old_ai, new_ai);
			if( !sets_equal ) 
				ai->D = new_ai.D; 
			fixpoint_reached &= sets_equal;

{ cerr << AI_to_string(equations[i]); }

		}
		print_results();
		char c;
		cout << "DEBUG: HIT KEY TO CONTINUE\n";
		cin >> c;
	}
	fixpoint_reached = counter;

	// Narrowing 
	counter=0;
	uint narrowing_fixpoint_reached=0;
	while( !narrowing_fixpoint_reached ) {
		counter++;
		narrowing_fixpoint_reached=1;
		for(unsigned i=0;i<equations.size();i++) {
			interval_analysis *ai = (interval_analysis*)equations[i]->AI;
			assert(ai->astv==equations[i]);
			assert(equations[i]->phi);

			interval_analysis old_ai( equations[i] ) ;  // Last result of equation.
			interval_analysis new_ai ;
			new_ai.astv = equations[i];
			for(set<ast_vertex*>::const_iterator it=ai->data_flow_predecessors.begin();it!=ai->data_flow_predecessors.end();it++) {
				assert(((interval_analysis*)(*it)->AI)->astv==*it);
				interval_analysis tmp_ai(*it);
				(*it)->phi(&tmp_ai); 
				operator_join(new_ai, tmp_ai, new_ai, 0);
			}
			bool sets_equal = operator_equal(old_ai, new_ai);
			if( !sets_equal ) 
				ai->D = new_ai.D; 
			narrowing_fixpoint_reached &= sets_equal;

{ cerr << AI_to_string(equations[i]); }
		}
		print_results();
		char c;
		cout << "DEBUG: HIT KEY TO CONTINUE NARROWING\n";
		cin >> c;
	}
	narrowing_fixpoint_reached = counter;
	cerr << "Fixpoint iteration is finished. We needed " << fixpoint_reached << " rounds for reaching the fixpoint and afterwards " << narrowing_fixpoint_reached<< " rounds of narrowing.\n";
	print_results();

	// Set all phi and AI values back to zero
	for(unsigned i=0;i<equations.size();i++) {
		if(equations[i]->AI) free(equations[i]->AI);
		equations[i]->AI=NULL;
	}
	delete_prg_analysis(ast.get_root());
}


void
interval_analysis::phi(void *argv) 
{
	interval_analysis *ai = (interval_analysis*) argv;

	// Parameter ai is meant as temporary data where the astv data component is associated with the ast vertex 
	// that should be processed by phi function.
	// Each ast vertex has a phi and AI component. 
	// The associated AI information of the ast vertex should NOT be changed here in phi.
	assert(ai->astv);
	switch( ai->astv->get_type() ) {
		case VT_INT_DECLARATION:
			ai->D[ ai->astv ] = IA_DOMAIN_TYPE_BOTTOM; 
			break;
		case VT_SPL_int_assign:
		{
			ast_vertex *lhs = FIRSTOF(ai->astv);
			ASSUME_TYPE(lhs, VT_SPL_identifier);
			const string lhs_name = lhs->get_str();
			ast_vertex *decl = sym_tbl.get_decl(lhs_name);
			ai->D[ decl ] = eval( ai->astv, SECONDOF(ai->astv) ); 
			break;
		}
		default:
			break;
	}
}

IA_DOMAIN_TYPE 
interval_analysis::eval(ast_vertex *assign, ast_vertex *v) 
{
	IA_DOMAIN_TYPE result;
	ASSUME_TYPE(assign, VT_SPL_int_assign);
	interval_analysis *ai = (interval_analysis*) assign->AI; assert(assign->AI);
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
			istringstream iss(v->get_str().c_str());
			ia_basic_type c;
			iss >> c;
			result = make_pair( c, c );
			break;
		}
		case VT_SPL_identifier:
		{
			// In case of an identifier, we have to look for the AI of this identifier. 
			const string identifier = v->get_str();
			ast_vertex *decl = sym_tbl.get_decl(identifier);
			if( ai->D.count(decl)>0 )  // In case that there is AI about this identifier provide this info.
				result = ai->D[decl];
			else 
				result = IA_DOMAIN_TYPE_BOTTOM;
			break;
		}
		case VT_SPL_int_add:
		{
			IA_DOMAIN_TYPE l = eval(assign, FIRSTOF(v));
			IA_DOMAIN_TYPE r = eval(assign, SECONDOF(v));
			result = operator_add(l, r);
			if(assign->get_id()==60) {
				cerr << "DEBUG: add assignment 60: \n";
				cerr << "l: " << value_to_string(l) << endl;
				cerr << "r: " << value_to_string(r) << endl;
				cerr << "result: " << value_to_string(result) << endl;
			}
			break;
		}
		case VT_SPL_int_sub:
		{
			IA_DOMAIN_TYPE l = eval(assign, FIRSTOF(v));
			IA_DOMAIN_TYPE r = eval(assign, SECONDOF(v));
			result = operator_sub(l, r);
			break;
		}
		case VT_SPL_int_mul:
		{
			IA_DOMAIN_TYPE l = eval(assign, FIRSTOF(v));
			IA_DOMAIN_TYPE r = eval(assign, SECONDOF(v));
			result = operator_mul(l, r);
			break;
		}
		case VT_SPL_int_div:
		{
			IA_DOMAIN_TYPE l = eval(assign, FIRSTOF(v));
			IA_DOMAIN_TYPE r = eval(assign, SECONDOF(v));
			result = operator_div(l, r);
			break;
		}
		case VT_SPL_int_mudolo:
			assert(0); // This is not implmented yet!
			break;
		case VT_SPL_STACKitop:
		case VT_SPL_STACKctop:
			result = IA_DOMAIN_TYPE_TOP; 
			break;
		case VT_SPL_expr_in_brackets:
			result = eval(assign, FIRST);
			break;
		default:
			cerr << v->vertex_type_2_string() << endl;
			assert(0); // This should not happen!
			result = IA_DOMAIN_TYPE_TOP; 
			break;
	}
	return result;
}


inline void 
interval_analysis::print_results() 
{
	ofstream ofs(RESULT_FILE.c_str()); assert( ofs.good() );
	for(unsigned i=0;i<equations.size();i++) {
		ofs << AI_to_string(equations[i]);
	}
}

inline const string
interval_analysis::value_to_string(IA_DOMAIN_TYPE d) 
{
	ostringstream oss;
	ia_basic_type lower_bound = d.first; 
	ia_basic_type upper_bound = d.second;
	IA_DOMAIN_TYPE bottom = IA_DOMAIN_TYPE_BOTTOM;
	if( d==bottom )
		oss << "BOTTOM";
	else
		oss << "[ " << basic_type_value_to_string(lower_bound) << ", " << basic_type_value_to_string(upper_bound) << " ]";
	return oss.str();
}

inline const string
interval_analysis::basic_type_value_to_string(ia_basic_type btv) 
{
	ostringstream oss;
	switch(btv) {
	case TOP:                  oss << "TOP";              break;
	case BOTTOM:               oss << "BOTTOM";           break;
	case THREAD_ID:            oss << "THREAD_ID";        break;
	case NUM_OF_THREADS:       oss << "NUM_OF_THREADS";   break;
	case PLUS_INFINITY:        oss << "PLUS_INFINITY";    break;
	default:                   oss << btv;                break;
	}
	return oss.str();
}

bool 
interval_analysis::operator_equal(interval_analysis& l, interval_analysis& r) 
{
	// DEBUG: PRint both sets for debugging issues
	assert(l.astv == r.astv);
	if( l.D.size()==0  &&  r.D.size()==0 ) {
		//cerr << "DEBUG: size from both sets is zero.\n";
		return true;
	}
	if( l.D.size()!=r.D.size() ) {
		//cerr << "DEBUG: size from both sets is different.\n";
		return false;
	}
	IA_FOR_EACH_it_IN_DOMAIN(l.D) {
		ast_vertex *d=it->first; IA_DOMAIN_TYPE v=it->second;
		if( r.D[d]!=v )  {
			//cerr << "DEBUG: element " << value_to_string(r.D[d]) << " and " << value_to_string(v) << " are different.\n";
			return false;
		}
	}
	//cerr << "DEBUG: ai sets are both the same.\n";
	return true;
}

// Join two elements of domain D, that is [a,b] join [c,d]
inline IA_DOMAIN_TYPE 
interval_analysis::operator_join( const IA_DOMAIN_TYPE &l
		                        , const IA_DOMAIN_TYPE &r) 
{
	// The join of two intervals is NOT bottom when one interval is bottom.
	// undefined  join  defined is defined.
	// The goal of IA is to get more precise intervals, therefore
	// is: undefined join [1,1] = [1,1]
	// wrong: IA_RETURN_BOTTOM_WHEN_l_OR_r_IS_BOTTOM(l,r);
	IA_DOMAIN_TYPE bottom = IA_DOMAIN_TYPE_BOTTOM;
	if( l==bottom ) return r;
	if( r==bottom ) return l;
 
	// Otherwise, we do not have a bottom element in L or R.

	// Distinguish the cases of   l o r = ?
	// 1. l cap r = l or l cap r = r  (contained)
	// 2. l cap r = emptyset
	// 3. l cap r != emptyset   and   l cap r != l  and  l cap r != r
	// In all cases holds [a,b]  o [c,d]  = [ min{a,c}, max{b,d} ]
	ia_basic_type a = l.first;
	ia_basic_type b = l.second;
	ia_basic_type c = r.first;
	ia_basic_type d = r.second;
	ia_basic_type min = (a<c) ? a : c;
	ia_basic_type max = (b>d) ? b : d;
	return make_pair( min, max );
}

// Widening of two elements, that is [a,b] nabla [c,d]
inline IA_DOMAIN_TYPE 
interval_analysis::operator_widening( const IA_DOMAIN_TYPE &l
		                            , const IA_DOMAIN_TYPE &r) 
{
	// The join of two intervals is NOT bottom when one interval is bottom.
	// undefined  join  defined is defined.
	// The goal of IA is to get more precise intervals, therefore
	// is: undefined join [1,1] = [1,1]
	// wrong: IA_RETURN_BOTTOM_WHEN_l_OR_r_IS_BOTTOM(l,r);
	IA_DOMAIN_TYPE bottom = IA_DOMAIN_TYPE_BOTTOM;
	if( l==bottom ) return r;
	if( r==bottom ) return l;
 
	// Otherwise, we do not have a bottom element in L or R.

	// Distinguish the cases of   l o r = ?
	// 1. l cap r = l or l cap r = r  (contained)
	// 2. l cap r = emptyset
	// 3. l cap r != emptyset   and   l cap r != l  and  l cap r != r
	// In all cases holds [a,b]  o [c,d]  = [ min{a,c}, max{b,d} ]
	ia_basic_type a = l.first;
	ia_basic_type b = l.second;
	ia_basic_type c = r.first;
	ia_basic_type d = r.second;
	ia_basic_type min = (a<=c) ? a : 0;
	ia_basic_type max = (b>=d) ? b : PLUS_INFINITY;
	return make_pair( min, max );
}


// Add two elements of domain D, that is [a,b] + [c,d]
inline IA_DOMAIN_TYPE 
interval_analysis::operator_add(  const IA_DOMAIN_TYPE &l
		                        , const IA_DOMAIN_TYPE &r) 
{
	IA_RETURN_BOTTOM_WHEN_l_OR_r_IS_BOTTOM(l,r);
	ia_basic_type a = l.first; ia_basic_type b = l.second;
	ia_basic_type c = r.first; ia_basic_type d = r.second;
	ia_basic_type min;
	ia_basic_type max;
	if( a!=PLUS_INFINITY && c!=PLUS_INFINITY ) min = a+c;
	else                                       min = PLUS_INFINITY;
	if( b!=PLUS_INFINITY && d!=PLUS_INFINITY ) max = b+d;
	else                                       max = PLUS_INFINITY;
	assert(min<=max);
	return make_pair( min, max );
}

// Subtract two elements of domain D, that is [a,b] - [c,d]
inline IA_DOMAIN_TYPE 
interval_analysis::operator_sub(  const IA_DOMAIN_TYPE &l
		                        , const IA_DOMAIN_TYPE &r) 
{
	IA_RETURN_BOTTOM_WHEN_l_OR_r_IS_BOTTOM(l,r);
	ia_basic_type a = l.first; ia_basic_type b = l.second;
	ia_basic_type c = r.first; ia_basic_type d = r.second;
	ia_basic_type min;
	ia_basic_type max;
	min = a-d;
	max = b-c;
	if( a!=PLUS_INFINITY && d!=PLUS_INFINITY ) {
		if(a-d<0) min = 0;
		else      min = a-d;
	}
	else 
		min = PLUS_INFINITY;
	if( b!=PLUS_INFINITY && c!=PLUS_INFINITY ) {
		if(b-c<0) max = 0;
		else      max = b-c;
	}
	else 
		max = PLUS_INFINITY;

	if(min<0) min=0;
	if(min>PLUS_INFINITY) min=PLUS_INFINITY;
	if(max<0) max=0;
	if(max>PLUS_INFINITY) max=PLUS_INFINITY;
	assert(min<=max);
	return make_pair( min, max );
}

// Multiply two elements of domain D, that is [a,b] * [c,d]
inline IA_DOMAIN_TYPE 
interval_analysis::operator_mul(  const IA_DOMAIN_TYPE &l
		                        , const IA_DOMAIN_TYPE &r) 
{
	IA_RETURN_BOTTOM_WHEN_l_OR_r_IS_BOTTOM(l,r);
	ia_basic_type a = l.first; ia_basic_type b = l.second;
	ia_basic_type c = r.first; ia_basic_type d = r.second;
	ia_basic_type min;
	ia_basic_type max;
	ia_basic_type mul1; // a*c
	ia_basic_type mul2; // a*d
	ia_basic_type mul3; // b*c
	ia_basic_type mul4; // b*d
	// a*c
	if( a==PLUS_INFINITY || c==PLUS_INFINITY ) mul1=PLUS_INFINITY;
	else                                       mul1=a*c;
	// a*d
	if( a==PLUS_INFINITY || d==PLUS_INFINITY ) mul2=PLUS_INFINITY;
	else                                       mul2=a*d;
	// b*c
	if( b==PLUS_INFINITY || c==PLUS_INFINITY ) mul3=PLUS_INFINITY;
	else                                       mul3=b*c;
	// b*d
	if( b==PLUS_INFINITY || d==PLUS_INFINITY ) mul4=PLUS_INFINITY;
	else                                       mul4=b*d;

	min = (mul1 < mul2) ? mul1 : mul2;
	min = (min < mul3) ?  min  : mul3;
	min = (min < mul4) ?  min  : mul4;

	max = (mul1 > mul2) ? mul1 : mul2;
	max = (max > mul3) ?  max  : mul3;
	max = (max > mul4) ?  max  : mul4;

	if(min<0) min=0;
	if(min>PLUS_INFINITY) min=PLUS_INFINITY;
	if(max<0) max=0;
	if(max>PLUS_INFINITY) max=PLUS_INFINITY;
	assert(min<=max);
	return make_pair( min, max );
}

// Divide two elements of domain D, that is [a,b] / [c,d]
inline IA_DOMAIN_TYPE 
interval_analysis::operator_div(  const IA_DOMAIN_TYPE &l
		                        , const IA_DOMAIN_TYPE &r) 
{
	IA_RETURN_BOTTOM_WHEN_l_OR_r_IS_BOTTOM(l,r);
	ia_basic_type a = l.first; ia_basic_type b = l.second;
	ia_basic_type c = r.first; ia_basic_type d = r.second;
	ia_basic_type min;
	ia_basic_type max;
	if(c==0) { // In case that zero can occur as divisor we return TOP as interval.
		return IA_DOMAIN_TYPE_TOP; // [0,inf]
	}
   	min = (a/c < a/d) ? a/c : a/d;
   	min = (min < b/c) ? min : b/c;
   	min = (min < b/d) ? min : b/d;
	if(min<0) min=0;
	if(min>PLUS_INFINITY) min=PLUS_INFINITY;

   	max = (a/c > a/d) ? a/c : a/d;
   	max = (max > b/c) ? max : b/c;
   	max = (max > b/d) ? max : b/d;

	if(min<0) min=0;
	if(min>PLUS_INFINITY) min=PLUS_INFINITY;
	if(max<0) max=0;
	if(max>PLUS_INFINITY) max=PLUS_INFINITY;
	assert(min<=max);
	return make_pair( min, max );
}

// Join two abstract informations AI1 and AI2 which are vectors with n components from domain D.
inline void
interval_analysis::operator_join(interval_analysis& l
		                       , interval_analysis& r 
							   , interval_analysis& result
							   , bool widening) 
{ 
  // Distinguish the cases
  // 1. element is in both sets
  // 2. element is only contained in L
  // 3. element is only contained in R

  // The first loop is for cases one and two.
  IA_FOR_EACH_it_IN_DOMAIN(l.D) { 
  	ast_vertex *d=it->first; IA_DOMAIN_TYPE v=it->second;
	if( r.D.count(d)>0 ) { // If true then d is contained in both sets (case one)
		// The first version used the common join operation with risk
		// of no termination. Therefore, use widening operator as 
		// explained in course, see lecture 8 slide 12.
		if(widening)
			result.D[d] = operator_widening( v, r.D[d] );
		else
			result.D[d] = operator_join( v, r.D[d] );
	}
	else  {  // Otherwise both sets do not have the same content (case two)
		result.D[d] = v;
	}
  }
  IA_FOR_EACH_it_IN_DOMAIN(r.D) {  // The remaining is only to put elements that are only contained in R into account (case three).
  	ast_vertex *d=it->first; IA_DOMAIN_TYPE v=it->second;
	if( l.D.count(d)==0 ) {  // Otherwise the element is already handled by the previous loop.
		result.D[d] = v;
	}
  }
}


inline void 
interval_analysis::init_prg_analysis(ast_vertex *astv) 
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
				from->AI = new interval_analysis;
				((interval_analysis*) from->AI)->astv = from;
			}
			for(i=0;i<equations.size() && equations[i]!=to;i++) 
				;
			if(i==equations.size()) {
				equations.push_back(to);
				assert( !to->AI );
				to->AI = new interval_analysis;
				((interval_analysis*) to->AI)->astv = to;
			}
			if(to->AI)
				((interval_analysis*) to->AI)->data_flow_predecessors.insert(from);
		}
	}
	FOR_EACH_it_IN(astv->get_children()) {
		init_prg_analysis(*it);
	}
}

inline void 
interval_analysis::delete_prg_analysis(ast_vertex *astv) 
{
	FOR_EACH_it_IN(astv->get_children()) {
		delete_prg_analysis(*it);
	}
	astv->AI=NULL;
	astv->phi = NULL;	
}

inline const string
interval_analysis::AI_to_string(ast_vertex *astv) 
{
	ostringstream oss;
	PRINT_LINE;
	oss << "Node " << astv->get_id() << " ("<< astv->vertex_type_2_string() << "):\n";	
	IA_FOR_EACH_it_IN_DOMAIN(((interval_analysis*)astv->AI)->D) {
		ast_vertex *d=it->first; IA_DOMAIN_TYPE v=it->second;
		oss << "Variable '" << d->get_lhs_name() << "' declared in " << d->get_id() << " has value " << value_to_string(v) << ".\n";
	}
	PRINT_LINE;
	oss << endl << endl;
	return oss.str();
}

