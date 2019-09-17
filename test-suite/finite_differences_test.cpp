#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <omp.h>
#include <assert.h>
#include <set>
#include <vector>
#include <unistd.h>
#include "dummy.h"

//#include "Stack.h"
#include "cstack_wrapper.h"
#include "cstack.h"

using namespace std;

#include "utilities.c"

#define LOG_POINTER_init 	set<void*> *thread_access_set; thread_access_set = new set<void*> [omp_get_num_procs()]
#define LOG_POINTER(p) 		thread_access_set[omp_get_thread_num()].insert((void*)p)

bool verbose_mode=false;

#include "adjoint_stacks.c"

extern long adjoint_stack_size;


double elapsed_time=0.;
double start,end;

int main(int argc, char* argv[])
{
	#include "test_specific_decl.c"
	LOG_POINTER_init;
	long mem_free_kb=0;
	int p = omp_get_max_threads() ;
	int m;
	long n = 1e8;
	long nfourth;
	long i=0;
 	long j=0;

	for(int i=1;i<argc;i++) {
		string arg(argv[i]);
		if(arg=="-v" || arg=="--verbose")
			verbose_mode=true;
	}

	// Get hostname to name output files accordingly.
	char buffer[1024];
	size_t pos=string(argv[0]).find_last_of('/'); assert(pos!=string::npos);
	string filename("/tmp/" + string(argv[0]).substr(pos) + ".tmp");
	string cmd = "hostname > " + filename;
	assert( !system(cmd.c_str()) );
	FILE *fp = fopen(filename.c_str(), "r"); assert(fp);
	assert( fgets(buffer, 1024, fp)==buffer );
	fclose(fp);
	cmd = "rm " + filename;  assert( !system(cmd.c_str()) );
	string hostname(buffer);   
	hostname = hostname.substr(0,hostname.length()-1);
	assert(hostname.length()>0);

	#ifdef PAPI
	const int number_of_events = 2;
	int* EventSets = new int [p];
	long long **PAPI_values = new long long* [p];
	long long fp_ops=0;
	long long max_cyc=0;
	long long PAPI_elapsed_cyc;
	int PAPI_mhz;

	for(i=0;i<p;i++) { EventSets[i] = PAPI_NULL; }
	int global_PAPI_retval = PAPI_library_init( PAPI_VER_CURRENT ); assert(global_PAPI_retval == PAPI_VER_CURRENT );
	assert( omp_get_nested()==0 ); // In the non-nested case we can use omp_get_thread_num to identify threads uniquely.
	global_PAPI_retval = PAPI_thread_init( ( unsigned long (*)( void ) ) ( omp_get_thread_num ) ); assert(global_PAPI_retval==PAPI_OK);
	#endif

	omp_lock_t lock_var;
	omp_init_lock(&lock_var);

	DEFINE_ARR_LD( thread_result, omp_get_num_procs() );
	DEFINE_ARR_LD( t1_thread_result, omp_get_num_procs() );
	DEFINE_ARR_LD( a1_thread_result, omp_get_num_procs() );

	// Init data
	cerr << "Init data" << endl;
	// Obtain size of x vector
	ifstream ifs("/proc/meminfo"); assert(ifs.good());
	while( ifs.good() ) {
		string line;
		const string searchpattern = "MemFree:  ";
		//const string searchpattern = "MemTotal:  ";
		getline(ifs, line) ;
		if( line.find(searchpattern)!=string::npos )  {
			cout << line << endl;
			line.erase( 0, string(searchpattern).length()-1 );
			line.erase( line.length()-2 );
			istringstream iss(line);
			iss >> mem_free_kb;
			cout << "There is " << mem_free_kb/1024. << "mb memory free." << endl;
			n = 2048;
			// As long as the memory needed for the data is bigger than 1% of free memory, reduce n:
			while( (n*n+n)*2*sizeof(double)/1024 > (mem_free_kb*1/100) ) 
				n--;
			// Second restriction to size n is that it is dividable by the number of cores.
			while( (n%omp_get_num_procs())!=0 )
				n--;

			adjoint_stack_size_kb=(mem_free_kb*80/100) - ((n+n*n)*2*sizeof(double)/1024);

			break;
		}
	}
	ifs.close();
	cerr << "info: n has size "  << n << ".\n";
	cerr << "info: x vector has "  << (unsigned)n/1000 << " * 1e3.\n";
	cerr << "info: A matrix has "  << (unsigned)(n*n)/1000 << " * 1e3.\n";
	cerr << "info: adjoint stack size (in mb): "  << (ulong)adjoint_stack_size_kb/1024 << ".\n";
	cerr << "Test (n%omp_get_num_procs())==0 successful.\n";
	DEFINE_ARR_LD(x, n);
	DEFINE_ARR_LD(t1_x, n);
	long double *a1_x = t1_x;

	DEFINE_MTX_LD(A, n);
	DEFINE_MTX_LD(t1_A, n);
	long double *a1_A = t1_A;
	// finite differences test
	cerr << "TEST: finite differences values compared to tangent-linear and adjoint values ";
	//omp_set_num_threads(omp_get_num_procs());
	assert( (n%omp_get_num_procs())==0 );
	const long double epsilon = 1e-2;
	for(int j=0;j<100;j++) {
		i=0;
		//for(i=0;i<omp_get_num_procs();i++) {
			#include "finite_differences_kernel.c"
		//}
		cerr << ".";
	}
	cerr << " done.\n";
	cerr << "Comparison between finite differences and tangent-linear as well as the adjoint code was \e[1;32m   successful    \e[0;0m.\n";


	cerr << "done\n";
	omp_destroy_lock(&lock_var); 
	return 0;
}

