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
#ifndef _cstack_h_
#define _cstack_h_

long adjoint_stack_size_kb = 0;
long unit_for_size=0; 

#define ad_init(sname,scounter,element_type)     \
{	assert(!sname) ; \
	assert( adjoint_stack_size_kb ) ; \
	long local_stack_size=(adjoint_stack_size_kb*1024)/omp_get_num_threads();  \
	unit_for_size=local_stack_size/16; /* 16 = 8+4+4 (8double, 2*4 for 2 int stacks) */ \
	while(!omp_test_lock(&lock_var)) { usleep(100); }  \
	/*if(omp_get_thread_num()==0) cerr << "Thread " << setw(3) << omp_get_thread_num() << ": " << __FILE__ << ":" << __LINE__ << ":info: Stack allocates " << unit_for_size << " elements of size " << sizeof(element_type) << " ... "; */ \
	sname = (element_type*) malloc( sizeof(element_type)*unit_for_size ); \
	/*if(omp_get_thread_num()==0) cerr << "done.\n"; */ \
	omp_unset_lock(&lock_var); \
	scounter = 0; \
	assert(sname); \
	/* for(int _i=0;_i<unit_for_size;_i++) { sname[_i]=(element_type)0; } */ \
	assert(!scounter); \
}

#define ad_push(sname,scounter,v)      sname[scounter++] = v 
#define ad_top(sname,scounter)         ((scounter>0) ? sname[scounter-1] : 0)
#define ad_pop(sname,scounter)         scounter--
#define ad_empty(sname,scounter)       (scounter==0)
#define ad_size(sname,scounter)     scounter

#define ad_spush(sname,scounter,v)     \
	{ \
		static int secure_message=0;    \
		if( omp_get_thread_num()==0 ) { \
			while(!omp_test_lock(&lock_var)) { usleep(100); }  \
			secure_message++;   \
			if(secure_message==1) cerr << "\e[1;31mThread " << setw(3) << omp_get_thread_num() << ": info: Performance issue: All thread are using secure push operations.\e[0;0m" << endl; \
			omp_unset_lock(&lock_var); \
		}  \
		assert(sname);  \
		assert(scounter<unit_for_size);   \
		sname[scounter++] = v;   \
	}
#define ad_spop(sname,scounter)        assert(scounter>=0); scounter--

#define ad_print_stack_memory_usage(sname,scounter)        \
		{  \
			while(!omp_test_lock(&lock_var)){ usleep(100); }  \
			cerr << "ad_print_stack_memory_usage: thread "<< omp_get_thread_num()  << ": Stack "<< sname << " needs " << scounter << " elements." << endl;  \
			omp_unset_lock(&lock_var);  \
		}

#define ad_clear(scounter)       scounter=0

#define ad_dealloc(sname,scounter)     \
	assert(sname) ; \
	while(!omp_test_lock(&lock_var)) { usleep(100); }  \
	/*cerr << "Thread " << setw(3) << omp_get_thread_num() << ": " << __FILE__ << ":" << __LINE__ << ":info: Stack deallocation ... "; */\
	free(sname) ;   \
	/*cerr << "done\n"; */\
	sname = NULL ;  \
	omp_unset_lock(&lock_var); \
	scounter = 0; \
	assert(!sname);  \
	assert(!scounter);



#define ad_init_nested(sname,scounter,element_type)     \
{	assert(!sname) ; \
	assert( adjoint_stack_size_kb ) ; \
	long local_stack_size=(adjoint_stack_size_kb*1024)/omp_get_num_threads();  \
	unit_for_size=local_stack_size/16; /* 16 = 8+4+4 (8double, 2*4 for 2 int stacks) */ \
	omp_set_nest_lock(&lock_var);   \
	/*if(omp_get_thread_num()==0) cerr << "Thread " << setw(3) << omp_get_thread_num() << ": " << __FILE__ << ":" << __LINE__ << ":info: Stack allocates " << unit_for_size << " elements of size " << sizeof(element_type) << " ... "; */ \
	sname = (element_type*) malloc( sizeof(element_type)*unit_for_size ); \
	/*if(omp_get_thread_num()==0) cerr << "done.\n"; */ \
	omp_unset_nest_lock(&lock_var); \
	scounter = 0; \
	assert(sname); \
	/* for(int _i=0;_i<unit_for_size;_i++) { sname[_i]=(element_type)0; } */ \
	assert(!scounter); \
}


#endif 
