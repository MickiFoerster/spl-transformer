#!/usr/bin/env python
import re
import sys
import os
import shutil

if len(sys.argv) != 4:
	print "syntax error: %s <target filename> <size of stack> <prefix for stack identifier>" % (sys.argv[0],);
	sys.exit(1);

target_file = sys.argv[1];
stack_size  = int(sys.argv[2]);
prefix      = sys.argv[3];
if not os.path.isfile(target_file):
	print "error: %s is not a file." % (target_file,);
	sys.exit(2);


basename, extension = os.path.splitext(target_file);
newfilename = basename + ".bak";
shutil.copy(target_file, newfilename);
f = open(target_file, 'r');
content = f.readlines();
f.close();
# Check whether stack are used or not.
isSTACKi_used = 0;   STACKi_pattern = "%sSTACKi" % (prefix,);
isSTACKf_used = 0;   STACKf_pattern = "%sSTACKf" % (prefix,);
isSTACKc_used = 0;   STACKc_pattern = "%sSTACKc" % (prefix,);
for i in range(0,len(content)-1):
	if re.search(STACKi_pattern, content[i])!=None: isSTACKi_used = 1;
	if re.search(STACKf_pattern, content[i])!=None: isSTACKf_used = 1;
	if re.search(STACKc_pattern, content[i])!=None: isSTACKc_used = 1;


# Inject stack definitions in case that they are necessary.
i=0;
while i<len(content):
	found = 0;
	if re.search('^#pragma omp parallel', content[i])!=None:
		if re.search('^{', content[i+1])!=None:
			found = 1;
			i+=1;
			sys.stdout.write("#pragma omp parallel\n{\n");
			if isSTACKi_used: sys.stdout.write('\tint %sSTACKi_c=0;\n\tint %sSTACKi[%s];\n' % (prefix,prefix,stack_size) );
			if isSTACKf_used: sys.stdout.write('\tint %sSTACKf_c=0;\n\tdouble %sSTACKf[%s];\n' % (prefix,prefix,stack_size) );
			if isSTACKc_used: sys.stdout.write('\tint %sSTACKc_c=0;\n\tint %sSTACKc[%s];\n' % (prefix,prefix,stack_size) );
	if found == 0:
		sys.stdout.write(content[i]);
	i+=1;

sys.exit(0);
