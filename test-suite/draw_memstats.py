#!/usr/bin/env python
import numpy as np
import matplotlib.pyplot as plt
import os
import re
import sys

drawbar_title = 'Memory Usage of Stacks in Adjoint Mode';
threads_list = [];
STACKc_mb_list = [];
STACKi_mb_list = [];
STACKf_mb_list = [];

if len(sys.argv)==1:
	print "syntax error: %s <mem stat file.plot>" % (sys.argv[0],);
	sys.exit(1);

filename_prefix = "";
sys.argv.pop(0); # remove script name
if len(sys.argv)!=1:
	print "There should be only one parameter containing the memory statistics for adjoint mode.";
	sys.exit(1);

f = open(sys.argv[0], 'r');
for line in f:
	#print line;
	if line[0]!="#" and len(line)>20:
		threads    = int(line[0:11]);
		STACKi_mb  = float(line[13:40]);
		STACKc_mb  = float(line[41:63]);
		STACKf_mb  = float(line[64:]);
		threads_list.append( threads );
		STACKc_mb_list.append( STACKc_mb );
		STACKi_mb_list.append( STACKi_mb );
		STACKf_mb_list.append( STACKf_mb );

N = 0;

ind = np.arange( len(STACKc_mb_list) )  ;
width = 0.12;
fig = plt.figure();
ax1 = fig.add_subplot(111);

colors = ('r', 'b', 'y', 'g');

rects_stackc = ax1.bar(ind,         STACKc_mb_list, width, color='r') ;
rects_stacki = ax1.bar(ind+1*width, STACKi_mb_list, width, color='b') ;
rects_stackf = ax1.bar(ind+2*width, STACKf_mb_list, width, color='y') ;
ax1.set_ylabel('Megabyte');
#ax1.yaxis.label.set_color('b');

ax1.set_title(drawbar_title);
ax1.legend((rects_stackc[0], rects_stacki[0], rects_stackf[0]), ('Control Flow Stack', 'Integer Stack', 'Floating-point Stack'), loc=1 )

plt.xticks(ind+1.5*width, threads_list);

#plt.show();
figure_filename = "%sadjoint_stacks_mem_usage" % (filename_prefix,);
plt.savefig("%s.eps" % (figure_filename,) );
plt.savefig("%s.png" % (figure_filename,) );
