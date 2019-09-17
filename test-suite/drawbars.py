#!/usr/bin/env python
import numpy as np
import matplotlib.pyplot as plt
import os
import re
import sys
import math
import glob
import random

drawbar_title = 'Speedup and MFLOPS of original, tangent-linear, and\nadjoint code depending on the number of threads';

threads_cycles_original = [];
threads_cycles_tangentlinear = [];
threads_cycles_adjoint = [];

threads_runtime_original = [];
threads_runtime_tangentlinear = [];
threads_runtime_adjoint = [];

threads_list = [];
speedups_list = [];
mflops_list   = [];

t1_threads_list = [];
t1_speedups_list = [];
t1_mflops_list   = [];

a1_threads_list = [];
a1_speedups_list = [];
a1_mflops_list   = [];
a1_STACK_threads = [];
a1_STACKc = [];
a1_STACKi = [];
a1_STACKf = [];
a1_STACK_overall = [];

opt_level = 0;
testname = "UNKNOWN";
cxx = "UNKNOWN";

size_of_data = "";
test_id = "";

def set_props(basename):
	global opt_level;
	global testname;
	global cxx;
	print basename;
	mobj = re.search('\.rz\.RWTH-Aachen\.DE\.([a-zA-Z_-]+)-O([0-9])-papi-([a-zA-Z0-9\.\+\-]+)\.[0-9]+\.original_speedup', basename);
	if mobj != None:
		testname  = mobj.group(1); 
		opt_level = int(mobj.group(2));
		cxx = mobj.group(3) ;
		if(cxx=="icpc"): cxx='Intel C++ compiler';
		if(cxx=="g++"): cxx='GCC (g++)';
		print testname;
		print opt_level;
		print cxx;
		if cxx == "UNKNOWN":
			print "error: Compiler cannot be recognized!";
			sys.exit(1);
		testname = str.replace(testname,"_","\_");
	else:
		print "error: reg expr did not match!";
		sys.exit(1);


def get_size_of_n_and_A(fn):
	global size_of_data;
	basedir = os.path.dirname(fn);
	list_of_stderr_files = glob.glob(basedir+'/job*.stderr');
	if len(list_of_stderr_files)>0:
		f = open(list_of_stderr_files[0], 'r');
		for line in f.readlines():
			mobj = re.match('info: n has size ([0-9]+)\.', line);
			if mobj != None:
				n = int(mobj.group(1));
				size_x = n*8/(1024);      # size of x in kilobytes
				if size_x/1024**2>0:
					size_x=size_x/1024**2;
					unit_x="gigabytes";
				elif size_x/1024>0: 
					size_x=size_x/1024**1;
					unit_x="megabytes";
				else:
					unit_x="kilobytes";

				size_A = (n**2)*8/(1024**2); # size of A in megabytes
				if size_A/1024>0:
					size_A=size_A/1024;
					unit_A="gigabytes";
				else:
					unit_A="megabytes";
				size_of_data = "The test was run with vector \\prg{x} $\\in \\R^{%d}$ with a memory size of %d %s. The matrix \\prg{A} $\\in \R^{%d \\times %d}$ consumes %d %s of memory." % (n, size_x, unit_x, n, n, size_A, unit_A);



def get_data_from_adjoint_stack_mem_file():
	global test_id;
	stack_mem_file = "";
	for fn in sys.argv:
		print fn;
		if re.search('original_speedup.plot', fn) != None: 
			stack_mem_file=re.sub("original_speedup.plot", "adjoint_stack_mem.plot", fn);
			mobj = re.search('\.([0-9]+)\.original_speedup.plot', fn);
			if mobj != None: 
				test_id = mobj.group(1);

	if os.path.isfile(stack_mem_file):
		get_size_of_n_and_A(stack_mem_file);
		f = open(stack_mem_file, 'r');
		for line in f.readlines():
			if line[0]!="#":
				mobj = re.match('^[\ ]+([0-9]+)[\ ]+([0-9\.]+)[\ ]+([0-9\.]+)[\ ]+([0-9\.]+)', line);
				if mobj != None:
					a1_STACK_threads.append( int(mobj.group(1)) );
					overall = float(mobj.group(2)) + float(mobj.group(3)) + float(mobj.group(4)) ;
					a1_STACKc.append( round(float(mobj.group(2))/1024, 1) );
					a1_STACKi.append( round(float(mobj.group(3))/1024, 1) );
					a1_STACKf.append( round(float(mobj.group(4))/1024, 1) );
					a1_STACK_overall.append( round(overall/1024, 1) );
				else:
					print "error: reg expr has no match.";
					sys.exit(1);
		f.close();
	else:
		print "error: File '%s' has not been found." % (stack_mem_file,);
		sys.exit(1);


def get_runtime_data_from_stderr_file():
	global threads_cycles_original;
	global threads_cycles_tangentlinear;
	global threads_cycles_adjoint;
	global threads_runtime_original;
	global threads_runtime_tangentlinear;
	global threads_runtime_adjoint;
	global test_id;
	filelist = glob.glob('*.stderr');
	for filename in filelist :
		f = open(filename, 'r');
		stat=0;
		for line in f.readlines():
			if stat==0:
				mobj = re.match('^Start of scale test with up to ', line);
				if mobj != None: 
					stat=1;
					content=[];
			elif stat==1:
				mobj = re.match('^done$', line);
				if mobj != None: 
					stat=0;
					content=[];
				else: 
					mobj = re.search('%s' % (test_id,), line);
					if mobj != None: 
						stat=2;
					content.append(line);
			elif stat==2:
				mobj = re.match('^done$', line);
				if mobj != None: 
					stat=3;
				content.append(line);
			elif stat==3:
				break;
		f.close();
		# At this point we filtered the part of the log-file where
		# the results of the currently considered plot file are printed.
		if stat==3: 
			stat=-1;
			for line in content:
				if   stat==-1:
					mobj = re.search('^Scaling test with ', line);
					if mobj != None: stat=0;
				elif stat==0:
					mobj = re.search('^Original code', line);
					if mobj != None: stat=1;
				elif stat==1:
					mobj = re.search('^parallel region took :[\ ]+([0-9\.]+)\ sec', line);
					if mobj != None: 
						threads_runtime_original.append(mobj.group(1));
					else:
						mobj = re.search('^overall FP_OPS     :.* max of TOT_CYC :[\ ]+([0-9]+)\ ', line);
						if mobj != None: threads_cycles_original.append(mobj.group(1));
						else:
							mobj = re.search('^tangent-linear test', line);
							if mobj != None: stat=2;
				elif stat==2:
					mobj = re.search('^parallel region took :[\ ]+([0-9\.]+)\ sec', line);
					if mobj != None: 
						threads_runtime_tangentlinear.append(mobj.group(1));
					else:
						mobj = re.search('^overall FP_OPS     :.* max of TOT_CYC :[\ ]+([0-9]+)\ ', line);
						if mobj != None: threads_cycles_tangentlinear.append(mobj.group(1));
						else:
							mobj = re.search('^Adjoint scale test', line);
							if mobj != None: stat=3;
				elif stat==3:
					mobj = re.search('^parallel region took :[\ ]+([0-9\.]+)\ sec', line);
					if mobj != None: 
						threads_runtime_adjoint.append(mobj.group(1));
					else:
						mobj = re.search('^overall FP_OPS     :.* max of TOT_CYC :[\ ]+([0-9]+)\ ', line);
						if mobj != None: 
							threads_cycles_adjoint.append(mobj.group(1));
						else:
							mobj = re.search('^Scaling test with ', line);
							if mobj != None: stat=0;



def save_latex_table(filename_wo_ext):
	global threads_runtime_original;
	global threads_runtime_tangentlinear;
	global threads_runtime_adjoint;
	global test_id;
	global size_of_data;
	get_data_from_adjoint_stack_mem_file();
	get_runtime_data_from_stderr_file();
	fn = ("%s.textbl" % (filename_wo_ext,) );
	f = open(fn, 'w');
	f.write("\\begin{table}[H]\\footnotesize\n") ;
	f.write("\\begin{center}\n") ;
	f.write("\\begin{tabular}{ccccc}\n") ;
	i=0;
	f.write(" \multicolumn{5}{c}{Original code} \\\\\n");
	f.write("%10s  &  %13s  &  %20s  &  %10s  &  %10s  \\\\\n\\hline\n" % ("\#Threads", "Runtime (sec)", "CPU cycles ($\cdot 10^{-9}$)", "Speedup", "MFLOPS"));
	while i<len(threads_list): 
		if i==len(threads_runtime_original): threads_runtime_original.append("0.0");
		if i==len(threads_cycles_original): threads_cycles_original.append("0");
		f.write("%10d  &  %13d  &  %20d  &  %10s  &  %10d \\\\\n" % (int(threads_list[i]), int(float(threads_runtime_original[i])), int(int(threads_cycles_original[i])*10e-9), speedups_list[i], int(mflops_list[i])));
		i=i+1;
	f.write(" \multicolumn{5}{c}{}   \\\\\n");
	f.write(" \multicolumn{5}{c}{Tangent-linear code} \\\\\n");
	f.write("%10s  &  %13s  &  %20s  &  %10s  &  %10s  \\\\\n\\hline\n" % ("\#Threads", "Runtime (sec)", "CPU cycles ($\cdot 10^{-9}$)", "Speedup", "MFLOPS"));
	i=0;
	while i<len(threads_list): 
		if i==len(threads_runtime_tangentlinear): threads_runtime_tangentlinear.append("0.0");
		if i==len(threads_cycles_tangentlinear): threads_cycles_tangentlinear.append("0");
		f.write("%10d  &  %13d  &  %20d  &  %10s  &  %10d  \\\\\n" % (int(threads_list[i]), int(float(threads_runtime_tangentlinear[i])), int(int(threads_cycles_tangentlinear[i])*10e-9), t1_speedups_list[i], int(t1_mflops_list[i])));
		i=i+1;
	f.write(" \multicolumn{5}{c}{}   \\\\\n");
	f.write(" \multicolumn{5}{c}{Adjoint code}  \\\\\n");
	f.write("%10s  &  %13s  &  %20s  &  %10s  &  %10s \\\\\n\\hline\n" % ("\#Threads", "Runtime (sec)", "CPU cycles ($\cdot 10^{-9}$)", "Speedup", "MFLOPS"));
	i=0;
	while i<len(threads_list): 
		if i==len(threads_runtime_adjoint): threads_runtime_adjoint.append("0.0");
		if i==len(threads_cycles_adjoint): threads_cycles_adjoint.append("0");
		f.write("%10d  &  %13d  &  %20d  &  %10s  &  %10d  \\\\\n" % (int(a1_threads_list[i]), int(float(threads_runtime_adjoint[i])), int(int(threads_cycles_adjoint[i])*10e-9), a1_speedups_list[i], int(a1_mflops_list[i])));
		i=i+1;
	f.write(" \multicolumn{5}{c}{}   \\\\\n");
	f.write(" \multicolumn{5}{c}{Sizes of stacks in gigabyte per thread used in the adjoint code} \\\\\n");
	f.write("%10s  &  %10s  &  %10s  &  %10s  &  %10s \\\\\n\\hline\n" % ("\#Threads", "$\\STACK{c}$", "$\\STACK{i}$", "$\\STACK{f}$", "Overall"));
	i=0;
	while i<len(threads_list): 
		f.write("%10d  &  %10.1f  &  %10.1f  &  %10.1f  &  %10.1f \\\\\n" % (int(threads_list[i]), a1_STACKc[i], a1_STACKi[i], a1_STACKf[i], a1_STACK_overall[i]));
		i=i+1;
	f.write("\\end{tabular}\n") 
	f.write("\\caption{Runtime results of the original, the tangent-linear, and the adjoint code of test {\\bf `%s'}, compiled by the {\\bf %s} with optimization level {\\bf %d}. In addition, the size of the adjoint code stacks are shown in gigabytes. The speedup is computed with respect to CPU cycles not runtime. %s}\n" % (testname, cxx, opt_level, size_of_data)) ;
	if test_id!="":
		table_label = test_id;
	else:
		sys.exit(1);
		table_label = "%s" % (random.uniform(1000000,10000000-1),);
	f.write("\\label{tbl:%s}\n" % (table_label,) ) ;
	f.write("\\end{center}\n") ;
	f.write("\\end{table}\n") ;
	f.close();



def line_to_data(line):
	mobj = re.match('^[\ ]+([0-9]+)[\ ]+([0-9\.]+)[\ ]+([0-9\.]+)[\ ]*([0-9\.]*)', line);
	if mobj != None:
		threads=int(mobj.group(1));
		speedup=float(mobj.group(2));
		mflops=float(mobj.group(3));
		if mobj.group(4)!='': speedup_time=float(mobj.group(4));
		else:  speedup_time=speedup;
	else:
		print "error: no match";
		print line;
		sys.exit(1);
	return (threads, speedup, mflops, speedup_time); 

if len(sys.argv)==1:
	print "syntax error: %s <original file.plot> <tangent linear.plot> <adjoint file.plot> " % (sys.argv[0],);
	sys.exit(1);

filename_prefix = "";
sys.argv.pop(0); # remove script name
if len(sys.argv)!=3:
	print "There should be three parameters containing the results from original, tangent-linear and adjoint code.";
	sys.exit(1);

for fn in sys.argv:
	basename, extension = os.path.splitext(fn);
	#print "Reading data from file '%s%s'.\n" % (basename,extension);
	if re.search('original_speedup', basename) != None: 
		set_props(fn);
		f = open(fn, 'r');
		for line in f:
			if line[0]!="#" and len(line)>20:
				threads, speedup, mflops, speedup_time = line_to_data(line) ;
				if (speedup/speedup_time)>2 or (speedup/speedup_time)<0.5:
					print "warning: big difference between papi and time measurement: papi=%f  time=%f" % (speedup, speedup_time);
				threads_list.append( threads );
				speedups_list.append( speedup );
				mflops_list.append( mflops );
	if re.search('tangent_speedup', basename) != None: 
		#drawbar_title = drawbar_title + " of tangent-linear code";
		f = open(fn, 'r');
		for line in f:
			if line[0]!="#" and len(line)>20:
				threads, speedup, mflops, speedup_time = line_to_data(line) ;
				t1_threads_list.append( threads );
				t1_speedups_list.append( speedup );
				t1_mflops_list.append( mflops );
	if re.search('adjoint_speedup', basename) != None: 
		#drawbar_title = drawbar_title + " of adjoint code";
		f = open(fn, 'r');
		for line in f:
			if line[0]!="#" and len(line)>20:
				threads, speedup, mflops, speedup_time = line_to_data(line) ;
				a1_threads_list.append( threads );
				a1_speedups_list.append( speedup );
				a1_mflops_list.append( mflops );


N = 0;

ind = np.arange( len(speedups_list) )  ;
width = 0.14;
fig = plt.figure();
ax1 = fig.add_subplot(111);

colors = ('r', 'b', 'y', 'g');

rects_speedups    = ax1.bar(ind, speedups_list, width, color='0.70') ;
rects_t1_speedups = ax1.bar(ind+2*width, t1_speedups_list, width, color='0.70') ;
rects_a1_speedups = ax1.bar(ind+4*width, a1_speedups_list, width, color='0.70') ;
ax1.set_xlabel('#Threads');
ax1.set_ylabel('Speedup');
#ax1.yaxis.label.set_color('b');

ax2 = ax1.twinx();
rects_mflops    = ax2.bar(ind+width, mflops_list,   width, color='0.25') ;
rects_t1_mflops = ax2.bar(ind+3*width, t1_mflops_list,   width, color='0.25') ;
rects_a1_mflops = ax2.bar(ind+5*width, a1_mflops_list,   width, color='0.25') ;
ax2.set_ylabel('MFLOPS');
ax2.set_title(drawbar_title);
ax2.legend((rects_speedups[0], rects_mflops[0]), ('Speedup', 'MFLOPS'), loc=2 )
#ax2.yaxis.label.set_color('r');

plt.xticks(ind+3*width, threads_list);



#plt.tight_layout();
#plt.tight_layout(pad=6., w_pad=0.5, h_pad=0.5)
plt.subplots_adjust( right=0.88 )

#plt.show();
figure_filename = "%sspeedup_and_mflops" % (filename_prefix,);
plt.savefig("%s.eps" % (figure_filename,) );
plt.savefig("%s.png" % (figure_filename,) );
save_latex_table(figure_filename);
