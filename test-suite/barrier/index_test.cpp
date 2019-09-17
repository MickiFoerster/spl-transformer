#include <iostream>
#include <iomanip>
#include <cassert>
#include <cmath>
#include <omp.h>
#include <cstdlib>

using namespace std;
#define DEFINE_ARR(an, sz)		double *an = new double [sz]; for(int i=0;i<sz;i++) an[i]=0.

int main(int argc, char* argv[])
{	
	const int n = 10;
	int nfourth=n/4;
	int m;
	DEFINE_ARR(x, n);
	DEFINE_ARR(phi, 2);
	DEFINE_ARR(constrain, 2);
	DEFINE_ARR(thread_result, omp_get_max_threads());
	double L;
	#include "test.in.spl"
	return 0;
}

