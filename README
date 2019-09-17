
Requirements
------------
- C++ compiler
- flex 
- bison
- cmake
- make



Building the Simple Parallel Language Compiler SPLc
---------------------------------------------------
cmake CMakeLists.txt 
make

Afterwards, the binary file SPLc is present in the current folder.



Using the SPLc
---------------------------------------------------
The interactive mode can be used by calling
./SPLc

There, you type in a parallel region coded in the SPL language
and end up your input with the keyword END.

The common case is that a parallel region is contained in a file,
e.g. test.in, and the SPLc should transform this code.
./SPLc test.in

The default is that no exclusive read analysis is used. 
The semantically equivalent command to the above is:
./SPLc test.in -O0

In contrast to this, the exclusive read analysis can be enabled
by giving the option -O1:
./SPLc test.in -O1

Each file
exclusive-read-results-001.txt
.
.
.
exclusive-read-results-N.txt
represents one iteration result of the fixed point iteration.
The final result can be examined in 
exclusive-read-results-endresult.txt.


Example:
cd SPLc/test-suite/exclusive-read-examples
../../SPLc second-example.in -O1
tail -n 18 exclusive-read-results-endresult.txt

	Read access 106 in float-assignment 211 (line 34) represents       A[((i*n)+j)+4] type(SPL_identifier) : exclusive read: yes
	Read access 118 in float-assignment 211 (line 34) represents                 x[i] type(SPL_identifier) : exclusive read: yes
	Read access 121 in float-assignment 211 (line 34) represents                 x[i] type(SPL_identifier) : exclusive read: yes
	Read access 125 in float-assignment 211 (line 34) represents                 x[i] type(SPL_identifier) : exclusive read: yes
	Read access 129 in float-assignment 211 (line 34) represents                 x[i] type(SPL_identifier) : exclusive read: yes
	Read access 135 in float-assignment 211 (line 34) represents       A[((i*n)+j)+3] type(SPL_identifier) : exclusive read: yes
	Read access 147 in float-assignment 211 (line 34) represents                 x[i] type(SPL_identifier) : exclusive read: yes
	Read access 150 in float-assignment 211 (line 34) represents                 x[i] type(SPL_identifier) : exclusive read: yes
	Read access 154 in float-assignment 211 (line 34) represents                 x[i] type(SPL_identifier) : exclusive read: yes
	Read access 161 in float-assignment 211 (line 34) represents       A[((i*n)+j)+2] type(SPL_identifier) : exclusive read: yes
	Read access 173 in float-assignment 211 (line 34) represents                 x[i] type(SPL_identifier) : exclusive read: yes
	Read access 176 in float-assignment 211 (line 34) represents                 x[i] type(SPL_identifier) : exclusive read: yes
	Read access 183 in float-assignment 211 (line 34) represents       A[((i*n)+j)+1] type(SPL_identifier) : exclusive read: yes
	Read access 195 in float-assignment 211 (line 34) represents                 x[i] type(SPL_identifier) : exclusive read: yes
	Read access 201 in float-assignment 211 (line 34) represents           A[(i*n)+j] type(SPL_identifier) : exclusive read: yes
	Read access 214 in float-assignment 215 (line 35) represents                    y type(SPL_identifier) : exclusive read: yes
	Read access 231 in float-assignment 232 (line 40) represents            local_sum type(SPL_identifier) : exclusive read: yes
