let SessionLoad = 1
if &cp | set nocp | endif
let s:cpo_save=&cpo
set cpo&vim
imap <silent> <Plug>IMAP_JumpBack =IMAP_Jumpfunc('b', 0)
imap <silent> <Plug>IMAP_JumpForward =IMAP_Jumpfunc('', 0)
map  :!/bin/bash
map  :tabprevious
vmap <NL> <Plug>IMAP_JumpForward
nmap <NL> <Plug>IMAP_JumpForward
map  :tabnext
map  :tabnew
vnoremap <silent> C :call RangeCommentLine()
nnoremap <silent> C :call CommentLine()
onoremap <silent> C :call CommentLine()
vnoremap <silent> X :call RangeUnCommentLine()
nnoremap <silent> X :call UnCommentLine()
onoremap <silent> X :call UnCommentLine()
nmap <silent> \cv <Plug>VCSVimDiff
nmap <silent> \cu <Plug>VCSUpdate
nmap <silent> \cU <Plug>VCSUnlock
nmap <silent> \cs <Plug>VCSStatus
nmap <silent> \cr <Plug>VCSReview
nmap <silent> \cq <Plug>VCSRevert
nmap <silent> \cn <Plug>VCSAnnotate
nmap <silent> \cN <Plug>VCSSplitAnnotate
nmap <silent> \cl <Plug>VCSLog
nmap <silent> \cL <Plug>VCSLock
nmap <silent> \ci <Plug>VCSInfo
nmap <silent> \cg <Plug>VCSGotoOriginal
nmap <silent> \cG <Plug>VCSClearAndGotoOriginal
nmap <silent> \cd <Plug>VCSDiff
nmap <silent> \cD <Plug>VCSDelete
nmap <silent> \cc <Plug>VCSCommit
nmap <silent> \ca <Plug>VCSAdd
nmap gx <Plug>NetrwBrowseX
nnoremap <silent> <Plug>VCSVimDiff :VCSVimDiff
nnoremap <silent> <Plug>VCSUpdate :VCSUpdate
nnoremap <silent> <Plug>VCSUnlock :VCSUnlock
nnoremap <silent> <Plug>VCSStatus :VCSStatus
nnoremap <silent> <Plug>VCSSplitAnnotate :VCSAnnotate!
nnoremap <silent> <Plug>VCSReview :VCSReview
nnoremap <silent> <Plug>VCSRevert :VCSRevert
nnoremap <silent> <Plug>VCSLog :VCSLog
nnoremap <silent> <Plug>VCSLock :VCSLock
nnoremap <silent> <Plug>VCSInfo :VCSInfo
nnoremap <silent> <Plug>VCSClearAndGotoOriginal :VCSGotoOriginal!
nnoremap <silent> <Plug>VCSGotoOriginal :VCSGotoOriginal
nnoremap <silent> <Plug>VCSDiff :VCSDiff
nnoremap <silent> <Plug>VCSDelete :VCSDelete
nnoremap <silent> <Plug>VCSCommit :VCSCommit
nnoremap <silent> <Plug>VCSAnnotate :VCSAnnotate
nnoremap <silent> <Plug>VCSAdd :VCSAdd
nnoremap <silent> <Plug>NetrwBrowseX :call netrw#NetrwBrowseX(expand("<cWORD>"),0)
vmap <silent> <Plug>IMAP_JumpBack `<i=IMAP_Jumpfunc('b', 0)
vmap <silent> <Plug>IMAP_JumpForward i=IMAP_Jumpfunc('', 0)
vmap <silent> <Plug>IMAP_DeleteAndJumpBack "_<Del>i=IMAP_Jumpfunc('b', 0)
vmap <silent> <Plug>IMAP_DeleteAndJumpForward "_<Del>i=IMAP_Jumpfunc('', 0)
nmap <silent> <Plug>IMAP_JumpBack i=IMAP_Jumpfunc('b', 0)
nmap <silent> <Plug>IMAP_JumpForward i=IMAP_Jumpfunc('', 0)
nnoremap <silent> <F11> :call conque_term#exec_file()
map <C-F12> :tabnew ~/.vimrc
map <C-F11> :vsplit test.in.spl:vsplit t1_test.in.spl:vsplit a1_test.in.spl
map <C-F10> :bufdo e
map <S-F8> ?\(^\\begin\|^\\bib\)
map <F8> /\(^\\begin\|^\\bib\)
map <S-F7> ?\(^\\section\|^\\subsection\|^\\subsubsection\|^\\paragraph\)
map <F7> /\(^\\section\|^\\subsection\|^\\subsubsection\|^\\paragraph\)
map <S-F6>  ?\(^\\part{\|^\\chapter{\)
map <F6> /\(^\\part{\|^\\chapter{\)
map <S-F5> :VCSDiff
map <F5> :TlistToggle
map <S-F4> :set foldenable:set foldmethod=marker:set foldmarker=/*,*/
map <F4> :set foldenable:set foldmethod=marker:set foldmarker={,}
map <S-F3> :!make clean;make
map <F3> :make
map <F2> :ConqueTermVSplit /bin/bash
map <M-Down> :tablast
map <M-Right> :tabnext
map <M-Left> :tabprevious
map <M-Up> :tabfirst
imap <NL> <Plug>IMAP_JumpForward
iabbr AC_MSG_CHECKING() AC_MSG_CHECKING([whether WHAT is enabled ])AC_MSG_RESULT([$ac_cv_enable_WHAT])
iabbr AM_CONDITIONAL() AM_CONDITIONAL([CONDITIONAL_VARIABLE], [test $myvar = "yes"])
iabbr AC_DEFINE() AC_DEFINE([DEBUG], [0], [Debug switch, on=1 off=0])
iabbr AC_CHECK_PROGS() AC_CHECK_PROGS(LATEX,[pdflatex latex],no)if test $LATEX = "no" ;thenAC_MSG_ERROR([Unable to find a Latex application.]);fi
iabbr AC_ARG_WITH() AC_ARG_WITH([WITHWHAT], [AC_HELP_STRING([--with-WITHWHAT], [This compiles with WITHWHAT])],[ac_cv_with_WITHWHAT=$withval],[ac_cv_with_WITHWHAT=no])if test "$ac_cv_with_WITHWHAT" != "no"; thenAC_DEFINE([DEFNAME], [1], [Comment which lands in the config.h])fiAC_SUBST([WITHWHAT], [$ac_cv_with_WITHWHAT])
iabbr AC_ARG_ENABLE() AC_ARG_ENABLE([ENABLEWHAT], [AC_HELP_STRING([--enable-ENABLEWHAT], [This enables ENABLEWHAT.])],[ac_cv_enable_ENABLEWHAT=$enableval],[ac_cv_enable_ENABLEWHAT=no])if test "$ac_cv_enable_ENABLEWHAT" != "no"; thenAC_DEFINE([DEFNAME], [1], [Comment which lands in the config.h])fiAC_SUBST([ENABLEWHAT], [$ac_cv_enable_ENABLEWHAT])
iabbr lstit /*@\it @*/
iabbr lstmm /*@$  $@*/
iabbr SSEQ \SubSeqStmts
iabbr SEQ \SeqStmts
iabbr texspl \begin{align}\begin{array}{l}\end{array}\end{align}
iabbr texsrctrans \lstset{caption={},numbers=none,label={}}\begin{center}\begin{tabular}{ccc}\begin{lstlisting}\end{lstlisting}&\rule[0mm]{10mm}{0mm}$\longmapsto$\rule[0mm]{10mm}{0mm}&\begin{lstlisting}\end{lstlisting}\\\end{tabular}\end{center}
iabbr textbllst \begin{table*}[hptb]\begin{tabular}{p{-1ex}l}&\lstinputlisting[caption={caption},label={lst-label1}]{code/f.c}\end{tabular}\end{table*}
iabbr texfig \begin{figure}[htb]	\centering	\includegraphics[0.95\textwidth]{images/plot.eps}	\caption{caption}	\label{label}\end{figure}
iabbr texlst \begin{lstlisting}[caption={},numbers=none,label={}]\end{lstlisting}2kA
iabbr texrule \rule[0mm]{0mm}{0mm}
iabbr texlstcom /*@$$@*/  
iabbr texlstlabel /*@ \label{} @*/  
iabbr texenum \begin{enumerate}\item {}\item {}\item {}\item {}\end{enumerate}
iabbr texitem \begin{itemize}\item {}\item {}\item {}\item {}\end{itemize}
iabbr texvframe \frame[containsverbatim]{\frametitle{title}\begin{lstlisting}[caption={},numbers=none,label={}]\end{lstlisting}}%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%?title
iabbr texframe \frame{\frametitle{title}\framesubtitle{subtitle}\begin{center}\begin{itemize}\item {}\item {}\item {}\item {}\item {}\end{itemize}\end{center}}%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%?title
iabbr texcols \begin{center}\begin{columns}[c]\begin{column}{0.45\textwidth}\includegraphics[width=32ex]{figures/juelich-logo}   \end{column}\begin{column}{0.45\textwidth}IBG-1: Biotechnologie \end{column}\end{columns}\end{center}
iabbr textable \begin{table}[!ht]\begin{center}\begin{tabular}{|l|c|c|c|}\hline\multicolumn{4}{|c|}{\rule[-3mm]{0mm}{8mm}\textbf{title}}\\\hline\hlinecol1     &    col2   &  \hspace{3cm}  &  col3 \\blabla   &  & & \\ \hline\end{tabular}\caption{The Caption.}\label{tbl1}\end{center}\end{table}2kI	
iabbr texletter \documentclass[DIN, pagenumber=false, parskip=half,%fromalign=left, fromphone=true,%fromemail=true, fromurl=false, %fromlogo=false, fromrule=false] {scrlttr2}\usepackage[utf8]{inputenc} % umlaute und ß direkt eingeben\usepackage{eurosym}\usepackage{ngerman} \setkomavar{fromname}{Dipl. Inform. Michael Förster}\setkomavar{fromaddress}{Rott 11\\52152 Simmerath}\setkomavar{fromphone}{0160/4416609}\setkomavar{fromemail}{Michael.Foerster@com-science.de}\setkomavar{subject}{betreff}\setkomavar{signature}{}%\setkomavar{fromurl}{www.michael-förster.de}\setkomavar{signature}{}\begin{document}%\subject{asdf}\begin{letter}{\noindent%Jungbluth \& Hiltner Grundstücks GbR\\Oberstr. 86\\52349 Düren}\opening{Sehr geehrte Damen und Herren,}ich bin Mieter des Garagenstellplatz mit der Nummer 1215 in der Achterstraße 24, 52072 Aachen.Durch einen Wohnungswechsel bin ich leider gezwungen den Mietvertrag fristgerecht zum 31.07.2010 zu kündigen.Ich wünsche Ihnen weiterhin alles Gute und verbleibe \closing{mit freundlichen Grüßen}\end{letter}\end{document}
iabbr texpapermakefile SRC_CODES=code/f.c code/a1_f.c code/f-wt-omp1.cGENERATED_SRC=memext.c seq.c critical.cGENERATED_TEX=*.aux *.log *.pdf *.ps *.dvi *.bbl *.blgall: paper.ps  paper.pdf%.ps:%.dvidvips $< -o%.pdf: %.pspaper.dvi: paper.tex Makefile $(GENERATED_SRC) $(SRC_CODES)latex $< bibtex $(<:.tex=.aux)latex $< latex $< seq.c: code/a1_adomp_f.c Makefile cat $< | sed "s/\t/\ \ \ \ /g" > /tmp/a.cg++ -E -C -P -DSOLUTION_SEQ /tmp/a.c > $@critical.c: code/a1_adomp_f.c Makefile cat $< | sed "s/\t/\ \ \ \ /g" > /tmp/a.cg++ -E -C -P -DSOLUTION_CRITICAL /tmp/a.c > $@ perl -n -p -e 's#firstprivate\(([^\)]*)\)#\\\n\ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ firstprivate($$1) /*@ \\label{lst-adomp-critical2} @*/#;' $@ > /tmp/a.cmv /tmp/a.c $@memext.c: code/a1_adomp_f.c Makefile cat $< | sed "s/\t/\ \ \ \ /g" > /tmp/a.cg++ -E -C -P -DSOLUTION_MEMEXT /tmp/a.c > $@ perl -n -p -e 's#firstprivate\(([^\)]*)\)#\\\n\ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ firstprivate($$1)#;' $@ > /tmp/a.cmv /tmp/a.c $@clean:rm -f $(GENERATED_SRC) $(GENERATED_TEX).PHONY: all clean
iabbr bibtechreport @book { id,author = {},title = {{}},year = {},address = {},}
iabbr bibbook @book { id,author = {},title = {{}},year = {},publisher = {},address = {},}
iabbr bibarticle @article { id,author = {},title = {{}},journal ={},year = {},pages = {},publisher = {},address = {},}
iabbr texbeamer \documentclass{beamer}\usepackage{beamerthemeshadow}\begin{document}\title{Beamer Class well nice}  \author{Sascha Frank}\date{\today} \frame{\titlepage} \frame{\frametitle{Table of contents}\tableofcontents} \frame{\frametitle{pictures and lists in beamer class}\begin{columns}\begin{column}{5cm}\begin{itemize}\item<1-> subject 1\item<3-> subject 2\item<5-> subject 3\end{itemize}\vspace{3cm} \end{column}\begin{column}{5cm}\begin{overprint}\includegraphics<2>{PIC1}\includegraphics<4>{PIC2}\includegraphics<6>{PIC3}\end{overprint}\end{column}\end{columns}}\end{document}
iabbr texpaper \documentclass[a4paper]{article}\usepackage{amsthm}\usepackage{amsmath}\usepackage{amssymb}\usepackage[utf8]{inputenc}\usepackage[pdftex,bookmarks=true,bookmarksopen=false,bookmarksnumbered=true,linktocpage,colorlinks=true,linkcolor=blue,  citecolor=blue, urlcolor=blue]{hyperref}\usepackage{listings}\theoremstyle{plain}\newtheorem{theorem}{Theorem}\newtheorem{lemma}[theorem]{Lemma}\newtheorem{corollary}[theorem]{Corollary}\newtheorem{proposition}[theorem]{Proposition}\theoremstyle{definition}\newtheorem{definition}[theorem]{Definition}\theoremstyle{remark} \newtheorem*{remark}{Remark}\newcommand{\N}{{\mathbb N}}\newcommand{\R}{{\mathbb R}}\newcommand{\A}{{\mathbf A}}\newcommand{\X}{{\mathbf x}}\newcommand{\Y}{{\mathbf y}}\newcommand{\tl}{tangent-linear}\newcommand{\aj}{adjoint}\newcommand{\oc}{original code}\newcommand{\lstref}[1]{listing \ref{#1}}\newcommand{\Lstref}[1]{Listing \ref{#1}}\newcommand{\lstline}[2]{line \ref{#2} of listing \ref{#1}}\newcommand{\prg}[1]{{\lstinline[basicstyle=\sffamily,keywordstyle=\mdseries]!#1!}}\newcommand{\AD}{Algorithmic Differentiation}\newcommand{\numthreads}{OMP\_NUM\_THREADS}\lstloadlanguages{[ISO]C++}\lstset{captionpos=b,language=[ISO]C++,basicstyle=\small\sffamily, numbers=left, numberstyle=\footnotesize, stepnumber=1, numbersep=5pt, breaklines=true, escapeinside={/*@}{@*/}}\lstset{morekeywords={omp, parallel, pragma, num_threads, firstprivate, reduction}}\title{Paper Title}\author{Michael F\"orster$^1$, Uwe Naumann$^1$, Jean Utke$^2$\\\footnotesize $^1$ LuFG Informatik 12: Software and Tools for Computational Engineering,\\\footnotesize RWTH Aachen University, Germany\\\footnotesize $^2$ Mathematics and Computer Science Division, Argonne National Laboratory,\\\footnotesize Argonne, IL, USA\\}\date{}\hypersetup{%pdfauthor={Michael Förster and Uwe Naumann},pdftitle={\TITLE},pdfsubject={In Proceedings of the 9th Modelica Conference (Modelica 2012), Munich, Germany},linkcolor=blue,anchorcolor=blue,citecolor=blue,filecolor=blue,menucolor=blue,urlcolor=blue}\begin{document}\maketitle\begin{abstract}abstract text\end{abstract}\section{Introduction}\section{What have been done}\section{Experimental Results}\section{Outlook}\bibliographystyle{unsrt}\bibliography{../../paper}\end{document}
iabbr textemplate \documentclass[a4paper]{article}\usepackage[utf8]{inputenc}\usepackage{listings}\begin{document}\begin{lstlisting}asdf\end{lstlisting}\end{document}
iabbr textutorial \documentclass{article}\usepackage{amsmath}\usepackage{palatino}\usepackage{amssymb}\usepackage{amsxtra}\usepackage[german]{babel}\usepackage{t1enc}\usepackage[utf8]{inputenc}\usepackage{amsthm}\usepackage{epsfig}\usepackage{listings}\usepackage{url}\usepackage{graphicx}\begin{document}\lstloadlanguages{[ISO]C++}\lstset{basicstyle=\small, numbers=none, numberstyle=\footnotesize}\pagestyle{plain}\begin{center}{\bf \large Einf\"uhrung in die Informatik (Programmierung, WS10/11)} \vspace{.1cm} \\Prof. Dr. U. Naumann, Dipl. Inf. M. F\"orster, Dipl. Inf. E. Varnik \vspace{.5cm} \\LuFG Informatik 12: Software and Tools for Computational Engineering \\RWTH Aachen University\vspace{.5cm} \\\includegraphics[width=3cm]{figures/logo.pdf} \\{\bf \huge \"Ubungsblatt A (18 Punkte)} \vspace{.1cm} \\(Erste Schritte mit C++, g++ und subversion)\vspace{.3cm} \\{\bf Abgabe: 2.11.2010 bis 16:30 Uhr per svn}\\\end{center}\section*{Aufgabe 1 (10 Punkte)}\section*{Aufgabe 2 (10 Punkte)}\end{document}
iabbr texmatrix \begin{align}\begin{bmatrix}a & b \\c & d\end{bmatrix}\end{align}
iabbr cmaketemplate cmake_minimum_required (VERSION 2.8)set (EXESRC filename.c)add_executable (main ${EXESRC})add_executable (main-wt-omp ${EXESRC})set_target_properties(main-wt-omp PROPERTIES COMPILE_FLAGS "-fopenmp")
iabbr bisontemplate %{#include <stdio.h>#include <stdarg.h>extern int yylineno;void yyerror(const char *s, ...);void info(char *s, ...);%}%token NAME%token INTF%token OP%token EQ%token OPEQ%%slc: 		seqstmts		;seqstmts: 	stmt ';' {info("seqstmts->stmt");}		| 	stmt ';' seqstmts {info("seqstmts->stmt seqstmt");}		;stmt:		assign  {info("stmt->assign");}		;assign:		NAME EQ expr	  	|	NAME OPEQ expr		;expr:		NAME OP NAME		|	INTF '(' NAME ')'		;%%voidyyerror(const char *s, ...){	va_list params;	va_start(params, s);	fprintf(stderr, "%d: error: ", yylineno);	vfprintf(stderr, s, params);	fprintf(stderr, "\n");}intmain(int argc, char **argv){	int i, rc=0;	if( argc < 2 ) { // stdin		yylineno = 1;		rc=yyparse();	}	else {  // from input files		for(i=1;i<argc;i++) {			FILE *f = fopen(argv[i], "r");			if(!f) {				perror(argv[1]);				return 1;			}			yyrestart(f);			rc|=yyparse();			fclose(f);		}	}	if(!rc)		printf("parsing successful\n");	else		printf("parsing unsuccessful\n");	return rc;}
iabbr flextemplate %option noyywrap nounput yylineno%x COMMENT%{#include <stdio.h>#include <stdarg.h>#include "parser.tab.h"voidinfo(char *s, ...){	va_list params;	va_start(params, s);	fprintf(stderr, "%d: info: ", yylineno);	vfprintf(stderr, s, params);	fprintf(stderr, "\n");}%}WS											[\t\ \n]+INTRINSIC_FUNCTION							sin|cos|expOP											[\-+\*/]EQ											=OPEQ										{OP}{EQ}NAME										[_a-zA-Z][_a-zA-Z0-9]*%%"/*"										{ BEGIN(COMMENT); }<COMMENT>"*/"								{ BEGIN(INITIAL); }<COMMENT>([^*]|\n)+|.						{}<COMMENT><<EOF>>							{ yyerror("Unterminated comment"); return 0;}"//".*\n									{}{WS}										{}{INTRINSIC_FUNCTION}						{  info("INTRINSIC_FUNCTION found in line %d", yylineno);return INTF; }{OP}										{  info("OP found"); return OP; }{EQ}										{  info("EQ found"); return EQ; }	{OPEQ}										{  info("OPEQ found"); return OPEQ; }	{NAME}										{  info("NAME found"); return NAME; }.											{  info("DEFAULT rule for char: '%c'", yytext[0]); return yytext[0]; }%%
iabbr dccf void f(const int n, double *x, double& y)#pragma ad indep x#pragma ad dep y{  int i=0;  y=1;  for(i=0;i<n;i++) {	y=y*sin(x[i]);  }}
iabbr ctemplate #include <stdio.h>#include <stdlib.h>int main(int argc, char* argv[]){return 0;}2kI	
iabbr qtuitemplate // form.h#ifndef FORM_H#define FORM_H#include <QtGui>#include <QDialog>#include "ui_mydialog.h"class myDialog : public QDialog {Q_OBJECTpublic:	myDialog();	~myDialog(){}private:	Ui::Dialog ui;private slots:	void handle_dialogresults();};#endif // form.cpp#include "form.h"myDialog::myDialog() {	ui.setupUi(this);	connect(ui.pushButton, SIGNAL( clicked() ), this, SLOT( handle_dialogresults() ));}void myDialog::handle_dialogresults() {}	// main.cpp#include <QApplication>#include "form.h"int main(int argc, char *argv[]) {	QApplication app(argc, argv);	myDialog *dlg = new myDialog;	dlg->show();	return app.exec();}
iabbr qttemplate #include <QApplication>#include <QPushButton>int main(int argc, char *argv[]) {	QApplication app(argc, argv);	QPushButton button("Leave me alone");	button.resize(123,45);	button.show();	return app.exec(); }2kI	
iabbr gtimertemplate GTimer *gtimer=NULL;gtimer=g_timer_new();g_timer_start(gtimer);g_timer_stop(gtimer);g_print("Elapsed time: %6.2f sec\n", g_timer_elapsed(gtimer,NULL));g_timer_destroy(gtimer);
iabbr glibtemplate #include <glib.h>int main(int argc, char* argv[]){return 0;}2kI
iabbr cxxtemplate #include <iostream>#include <iomanip>#include <cassert>#include <cstdlib>using namespace std;int main(int argc, char* argv[]){return 0;}2kI	
iabbr getopttemplate #include <ctype.h>#include <stdio.h>#include <stdlib.h>#include <unistd.h>intmain (int argc, char **argv){int aflag = 0;int bflag = 0;char *cvalue = NULL;int index;int c;opterr = 0;while ((c = getopt (argc, argv, "abc:")) != -1)switch (c){case 'a':aflag = 1;break;case 'b':bflag = 1;break;case 'c':cvalue = optarg;break;case '?':if (optopt == 'c')fprintf (stderr, "Option -%c requires an argument.\n", optopt);else if (isprint (optopt))fprintf (stderr, "Unknown option `-%c'.\n", optopt);elsefprintf (stderr,"Unknown option character `\\x%x'.\n",optopt);return 1;default:abort ();}printf ("aflag = %d, bflag = %d, cvalue = %s\n",aflag, bflag, cvalue);for (index = optind; index < argc; index++)printf ("Non-option argument %s\n", argv[index]);return 0;}
iabbr mpigather MPI_Gather(  a[0]+id*2,       /* void *send_buffer      */2,                /* int  send_cnt          */MPI_DOUBLE,       /* MPI_Datatype send_type */&a,               /* void *recv_buffer      */2,                /* int  *recv_cnt         */MPI_DOUBLE,       /* MPI_Datatype recv_type */0,                /* int root               */MPI_COMM_WORLD    /* MPI_Comm coomunicator  */);
iabbr mpirecv MPI_Recv(&a[2],                    /* void *message         */2,                        /* int count             */MPI_DOUBLE,               /* MPI_Datatype datatype */1,                        /* int source            */2,                        /* int tag               */MPI_COMM_WORLD,           /* MPI_Comm comm         */&status                   /* MPI_Status *status    */);
iabbr mpisend MPI_Send(&a[0],                   /* void* message */2,                       /* int   count   */MPI_DOUBLE,              /* MPI_Datatype datatype */1,                       /* int dest */1,                       /* int tag  */MPI_COMM_WORLD           /* MPI_Comm comm */);
iabbr mpitemplate #include <mpi.h>#include <stdio.h>int main(int argc, char* argv[]) {int i;int id;  /* Process rank */int p;   /* Number of processes */MPI_Init(&argc, &argv);MPI_Comm_rank(MPI_COMM_WORLD, &id);MPI_Comm_size(MPI_COMM_WORLD, &p);printf("Process %d is done\n", id);fflush(stdout);MPI_Finalize();return 0;}
iabbr ompdatadecomp 	int tid=0;	int p=0;	int c=0;	int lb=0;	int ub=0;	int i=0;	tid=omp_get_thread_num();	p=omp_get_num_threads();	c=n/p;	i=c*p;	if(i!=n) {		 c=c+1; 	}	lb=tid*c;	ub=(tid+1)*c-1;	if(ub>=n) {		ub=n-1; 	}	i=lb;	y=0.;	while(i<=ub) {		y=y+sin(x[i]);		i=i+1;	}
iabbr omptemplate #include <stdio.h>#include <stdlib.h>#include <omp.h>int main(int argc, char* argv[]){int i;#pragma omp parallel {fprintf(stderr, "This is thread %d\n.", omp_get_thread_num());}#pragma omp sections{#pragma omp sectionfprintf(stderr, "This is thread %d\n.", omp_get_thread_num());#pragma omp sectionfprintf(stderr, "This is thread %d\n.", omp_get_thread_num());}#pragma omp parallel forfor(i=0;i<10;i++)fprintf(stderr, "This is thread %d\n.", omp_get_thread_num());return 0;}
iabbr perltemplate #!/usr/bin/perluse warnings;use strict;while(<>) {}kA
iabbr pythontemplate #!/usr/bin/env pythonimport reimport sysimport osfor arg in sys.argv:if arg != sys.argv[0]:file = open(arg, 'r');basename, extension = os.path.splitext(arg);newfilename = basename + "-scaling" + extension;of = open(newfilename, 'w');for line in file:linesplit = re.split('\t', line);of.write(linesplit[0]);of.write(linesplit[1]);of.write(linesplit[3]);
iabbr makecxx BINARIES=MxM MxM.debugCXX=g++CXXFLAGS=-o $@LDFLAGS=-lgsl -lgslcblas `pkg-config glib-2.0 --cflags --libs`MxM: MxM.cpp$(CXX) $(CXXFLAGS) $(LDFLAGS) $<MxM.debug: MxM.cpp$(CXX) -g $(CXXFLAGS) $(LDFLAGS) $<clean:rm -f $(BINARIES).PHONY: clean
iabbr maketex TARGET=targetfilenameSRC=$(TARGET).texPDF=$(TARGET).pdf%.pdf: %.texpdflatex $< -o $@$(PDF): $(SRC)clean:rm -f *.o *.exe *.aux *.log *.pdf.PHONY: clean 
iabbr greentemplate "\e[1;32mEVERYTHING'S FINE\e[0;0m"
iabbr redtemplate "\e[1;31mRED ALERT\e[0;0m"
iabbr gccplugintemplate GCC=gcc-svnGCC_SRC_DIR=$$HOME/workspace/external-projects/gcc-svnGCC_BLD_DIR=$$HOME/workspace/external-projects/gcc-svn/bldGCC_PLUGIN_DIR=$(GCC_BLD_DIR)/lib/gcc/x86_64-unknown-linux-gnu/4.7.0/pluginCFLAGS+=-fPIC -O2 -I$(GCC_SRC_DIR)/gcc -I$(GCC_BLD_DIR)/gcc -I$(GCC_BLD_DIR)/gmp -I$(GCC_PLUGIN_DIR)/include$(GCC_PLUGIN_DIR)/helloworld.so: helloworld.o$(GCC) $(CFLAGS) -shared $^ -o $@helloworld.o: helloworld.c$(GCC) -c $(CFLAGS) helloworld.ctest: test.c$(GCC) -fplugin=`$(GCC) -print-file-name=plugin`/helloworld.so  $<clean:rm -f *.ocd $(GCC_PLUGIN_DIR);rm -f helloworld.so.PHONY: clean test-------------------------------------------------------------------------------------/* This plugin is only a template to start with programming a plugin for gcc.  */#include "gcc-plugin.h"#include "config.h"#include "system.h"#include "coretypes.h"#include "tm.h"#include "toplev.h"#include "basic-block.h"#include "gimple.h"#include "tree.h"#include "tree-pass.h"#include "intl.h"#include "plugin-version.h"#include "diagnostic.h"int plugin_is_GPL_compatible; /* Plugin has to have under GPL license what one defines with this definition. Without this definition gcc exits with a fatal error. */static treeget_real_ref_rhs (tree expr){switch (TREE_CODE (expr)){case SSA_NAME:case VAR_DECL:case PARM_DECL:case FIELD_DECL:case COMPONENT_REF:case MEM_REF:case ARRAY_REF:return expr;default:return NULL_TREE;}}static unsigned intexecute_my_plugin_title (void){gimple_stmt_iterator gsi;basic_block bb;fprintf(stderr, "Start of my plugin:\n");FOR_EACH_BB (bb){for (gsi = gsi_start_bb (bb); !gsi_end_p (gsi); gsi_next (&gsi)){fprintf(stderr, "%p\n", gsi);}}return 0;}/* Pass gate function. Currently always returns true.  */static boolgate_my_plugin_title (void){return true;}static struct gimple_opt_pass pass_my_plugin_title ={{GIMPLE_PASS,"my_plugin_title",                    /* name */gate_my_plugin_title,                 /* gate */execute_my_plugin_title,              /* execute */NULL,                                 /* sub */NULL,                                 /* next */0,                                    /* static_pass_number */TV_NONE,                              /* tv_id */PROP_ssa,                             /* properties_required */0,                                    /* properties_provided */0,                                    /* properties_destroyed */0,                                    /* todo_flags_start */TODO_dump_func                        /* todo_flags_finish */}};char hello_text[4096];/* The initialization routine exposed to and called by GCC. The spec of thisfunction is defined in gcc/gcc-plugin.h. LUGIN_NAME - name of the plugin (useful for error reporting)ARGC        - the size of the ARGV arrayARGV        - an array of key-value argument pairReturns 0 if initialization finishes successfully. ote that this function needs to be named exactly "plugin_init".  */intplugin_init (struct plugin_name_args *plugin_info,struct plugin_gcc_version *version){struct register_pass_info pass_info;const char *plugin_name = plugin_info->base_name;int argc = plugin_info->argc;struct plugin_argument *argv = plugin_info->argv;int i;if (!plugin_default_version_check (version, &gcc_version))return 1;/* Self-assign detection should happen after SSA is constructed.  */pass_info.pass = &pass_my_plugin_title.pass;pass_info.reference_pass_name = "ssa";pass_info.ref_pass_instance_number = 1;pass_info.pos_op = PASS_POS_INSERT_AFTER;for (i = 0; i < argc; ++i){if (!strcmp (argv[i].key, "say-hello")){if (argv[i].value)warning (0, G_("option '-fplugin-arg-%s-say-hello=%s'"),plugin_name, argv[i].value);elsestrncpy(hello_text, argv[i].value, 4096);}elsewarning (0, G_("plugin %qs: unrecognized argument %qs ignored"),plugin_name, argv[i].key);}register_callback (plugin_name, LUGIN_PASS_MANAGER_SETUP, ULL,&pass_info);return 0;}
iabbr OEE Ö
iabbr UEE Ü
iabbr AEE Ä 
iabbr sshtml &szlig;
iabbr Uehtml &Uuml;
iabbr Aehtml &Auml;
iabbr Oehtml &Ouml;
iabbr uehtml &uuml;
iabbr aehtml &auml;
iabbr oehtml &ouml;
iabbr EURO €
iabbr lstforallthreads /*@$\forall$@*/ t /*@$\in [0:\numthreads-1]:$@*/
iabbr lstidx /*@$_{idx}$@*/
iabbr lstgets /*@$\gets$@*/
let &cpo=s:cpo_save
unlet s:cpo_save
set autoread
set background=dark
set cinkeys=0{,0},0),:,!^F,o,O,e
set fileencodings=ucs-bom,utf-8,default,latin1
set guioptions=aegirLt
set helplang=en
set printdevice=NRG_DSm645
set printoptions=number:y,syntax:y
set shiftwidth=4
set showcmd
set spelllang=en_us,de_de,fr
set tabpagemax=20
set tabstop=4
set viewdir=~/.vim/views
set visualbell
set window=54
set nowrapscan
let s:so_save = &so | let s:siso_save = &siso | set so=0 siso=0
let v:this_session=expand("<sfile>:p")
silent only
cd ~/comgit/phd/code/compiler/spl-transformer/test-suite/straight-line-code
if expand('%') == '' && !&modified && line('$') <= 1 && getline(1) == ''
  let s:wipebuf = bufnr('%')
endif
set shortmess=aoO
badd +1 data-decomposition.in
badd +0 data-decomposition-p-divides-n.in
badd +0 exclusive-read-results-001.txt
badd +3 exclusive-read-results-002.txt
badd +0 exclusive-read-results-endresult.txt
badd +0 first-example.in
args data-decomposition.in data-decomposition-p-divides-n.in
edit data-decomposition.in
set splitbelow splitright
set nosplitbelow
set nosplitright
wincmd t
set winheight=1 winwidth=1
argglobal
setlocal keymap=
setlocal noarabic
setlocal noautoindent
setlocal balloonexpr=
setlocal nobinary
setlocal bufhidden=
setlocal buflisted
setlocal buftype=
setlocal nocindent
setlocal cinkeys=0{,0},0),:,!^F,o,O,e
setlocal cinoptions=
setlocal cinwords=if,else,while,do,for,switch
setlocal colorcolumn=
setlocal comments=s1:/*,mb:*,ex:*/,://,b:#,:%,:XCOMM,n:>,fb:-
setlocal commentstring=/*%s*/
setlocal complete=.,w,b,u,t,i
setlocal concealcursor=
setlocal conceallevel=0
setlocal completefunc=
setlocal nocopyindent
setlocal cryptmethod=
setlocal nocursorbind
setlocal nocursorcolumn
setlocal nocursorline
setlocal define=
setlocal dictionary=
setlocal nodiff
setlocal equalprg=
setlocal errorformat=
setlocal noexpandtab
if &filetype != 'conf'
setlocal filetype=conf
endif
setlocal foldcolumn=0
setlocal foldenable
setlocal foldexpr=0
setlocal foldignore=#
setlocal foldlevel=0
setlocal foldmarker={{{,}}}
setlocal foldmethod=manual
setlocal foldminlines=1
setlocal foldnestmax=20
setlocal foldtext=foldtext()
setlocal formatexpr=
setlocal formatoptions=tcq
setlocal formatlistpat=^\\s*\\d\\+[\\]:.)}\\t\ ]\\s*
setlocal grepprg=
setlocal iminsert=2
setlocal imsearch=2
setlocal include=
setlocal includeexpr=
setlocal indentexpr=
setlocal indentkeys=0{,0},:,0#,!^F,o,O,e
setlocal noinfercase
setlocal iskeyword=@,48-57,_,192-255
setlocal keywordprg=
setlocal nolinebreak
setlocal nolisp
setlocal nolist
setlocal makeprg=
setlocal matchpairs=(:),{:},[:]
setlocal modeline
setlocal modifiable
setlocal nrformats=octal,hex
set number
setlocal number
setlocal numberwidth=4
setlocal omnifunc=
setlocal path=
setlocal nopreserveindent
setlocal nopreviewwindow
setlocal quoteescape=\\
setlocal noreadonly
setlocal norelativenumber
setlocal norightleft
setlocal rightleftcmd=search
setlocal noscrollbind
setlocal shiftwidth=4
setlocal noshortname
setlocal nosmartindent
setlocal softtabstop=0
setlocal nospell
setlocal spellcapcheck=[.?!]\\_[\\])'\"\	\ ]\\+
setlocal spellfile=
setlocal spelllang=en_us,de_de,fr
setlocal statusline=
setlocal suffixesadd=
setlocal swapfile
setlocal synmaxcol=3000
if &syntax != 'conf'
setlocal syntax=conf
endif
setlocal tabstop=4
setlocal tags=
setlocal textwidth=0
setlocal thesaurus=
setlocal noundofile
setlocal nowinfixheight
setlocal nowinfixwidth
set nowrap
setlocal nowrap
setlocal wrapmargin=0
silent! normal! zE
let s:l = 15 - ((13 * winheight(0) + 26) / 53)
if s:l < 1 | let s:l = 1 | endif
exe s:l
normal! zt
15
normal! 0
tabedit data-decomposition-p-divides-n.in
set splitbelow splitright
set nosplitbelow
set nosplitright
wincmd t
set winheight=1 winwidth=1
argglobal
2argu
setlocal keymap=
setlocal noarabic
setlocal noautoindent
setlocal balloonexpr=
setlocal nobinary
setlocal bufhidden=
setlocal buflisted
setlocal buftype=
setlocal nocindent
setlocal cinkeys=0{,0},0),:,!^F,o,O,e
setlocal cinoptions=
setlocal cinwords=if,else,while,do,for,switch
setlocal colorcolumn=
setlocal comments=s1:/*,mb:*,ex:*/,://,b:#,:%,:XCOMM,n:>,fb:-
setlocal commentstring=/*%s*/
setlocal complete=.,w,b,u,t,i
setlocal concealcursor=
setlocal conceallevel=0
setlocal completefunc=
setlocal nocopyindent
setlocal cryptmethod=
setlocal nocursorbind
setlocal nocursorcolumn
setlocal nocursorline
setlocal define=
setlocal dictionary=
setlocal nodiff
setlocal equalprg=
setlocal errorformat=
setlocal noexpandtab
if &filetype != 'conf'
setlocal filetype=conf
endif
setlocal foldcolumn=0
setlocal foldenable
setlocal foldexpr=0
setlocal foldignore=#
setlocal foldlevel=0
setlocal foldmarker={{{,}}}
setlocal foldmethod=manual
setlocal foldminlines=1
setlocal foldnestmax=20
setlocal foldtext=foldtext()
setlocal formatexpr=
setlocal formatoptions=tcq
setlocal formatlistpat=^\\s*\\d\\+[\\]:.)}\\t\ ]\\s*
setlocal grepprg=
setlocal iminsert=2
setlocal imsearch=2
setlocal include=
setlocal includeexpr=
setlocal indentexpr=
setlocal indentkeys=0{,0},:,0#,!^F,o,O,e
setlocal noinfercase
setlocal iskeyword=@,48-57,_,192-255
setlocal keywordprg=
setlocal nolinebreak
setlocal nolisp
setlocal nolist
setlocal makeprg=
setlocal matchpairs=(:),{:},[:]
setlocal modeline
setlocal modifiable
setlocal nrformats=octal,hex
set number
setlocal number
setlocal numberwidth=4
setlocal omnifunc=
setlocal path=
setlocal nopreserveindent
setlocal nopreviewwindow
setlocal quoteescape=\\
setlocal noreadonly
setlocal norelativenumber
setlocal norightleft
setlocal rightleftcmd=search
setlocal noscrollbind
setlocal shiftwidth=4
setlocal noshortname
setlocal nosmartindent
setlocal softtabstop=0
setlocal nospell
setlocal spellcapcheck=[.?!]\\_[\\])'\"\	\ ]\\+
setlocal spellfile=
setlocal spelllang=en_us,de_de,fr
setlocal statusline=
setlocal suffixesadd=
setlocal swapfile
setlocal synmaxcol=3000
if &syntax != 'conf'
setlocal syntax=conf
endif
setlocal tabstop=4
setlocal tags=
setlocal textwidth=0
setlocal thesaurus=
setlocal noundofile
setlocal nowinfixheight
setlocal nowinfixwidth
set nowrap
setlocal nowrap
setlocal wrapmargin=0
silent! normal! zE
let s:l = 1 - ((0 * winheight(0) + 26) / 53)
if s:l < 1 | let s:l = 1 | endif
exe s:l
normal! zt
1
normal! 0
tabedit first-example.in
set splitbelow splitright
set nosplitbelow
set nosplitright
wincmd t
set winheight=1 winwidth=1
argglobal
edit first-example.in
setlocal keymap=
setlocal noarabic
setlocal noautoindent
setlocal balloonexpr=
setlocal nobinary
setlocal bufhidden=
setlocal buflisted
setlocal buftype=
setlocal nocindent
setlocal cinkeys=0{,0},0),:,!^F,o,O,e
setlocal cinoptions=
setlocal cinwords=if,else,while,do,for,switch
setlocal colorcolumn=
setlocal comments=s1:/*,mb:*,ex:*/,://,b:#,:%,:XCOMM,n:>,fb:-
setlocal commentstring=/*%s*/
setlocal complete=.,w,b,u,t,i
setlocal concealcursor=
setlocal conceallevel=0
setlocal completefunc=
setlocal nocopyindent
setlocal cryptmethod=
setlocal nocursorbind
setlocal nocursorcolumn
setlocal nocursorline
setlocal define=
setlocal dictionary=
setlocal nodiff
setlocal equalprg=
setlocal errorformat=
setlocal noexpandtab
if &filetype != ''
setlocal filetype=
endif
setlocal foldcolumn=0
setlocal foldenable
setlocal foldexpr=0
setlocal foldignore=#
setlocal foldlevel=0
setlocal foldmarker={{{,}}}
setlocal foldmethod=manual
setlocal foldminlines=1
setlocal foldnestmax=20
setlocal foldtext=foldtext()
setlocal formatexpr=
setlocal formatoptions=tcq
setlocal formatlistpat=^\\s*\\d\\+[\\]:.)}\\t\ ]\\s*
setlocal grepprg=
setlocal iminsert=2
setlocal imsearch=2
setlocal include=
setlocal includeexpr=
setlocal indentexpr=
setlocal indentkeys=0{,0},:,0#,!^F,o,O,e
setlocal noinfercase
setlocal iskeyword=@,48-57,_,192-255
setlocal keywordprg=
setlocal nolinebreak
setlocal nolisp
setlocal nolist
setlocal makeprg=
setlocal matchpairs=(:),{:},[:]
setlocal modeline
setlocal modifiable
setlocal nrformats=octal,hex
set number
setlocal number
setlocal numberwidth=4
setlocal omnifunc=
setlocal path=
setlocal nopreserveindent
setlocal nopreviewwindow
setlocal quoteescape=\\
setlocal noreadonly
setlocal norelativenumber
setlocal norightleft
setlocal rightleftcmd=search
setlocal noscrollbind
setlocal shiftwidth=4
setlocal noshortname
setlocal nosmartindent
setlocal softtabstop=0
setlocal nospell
setlocal spellcapcheck=[.?!]\\_[\\])'\"\	\ ]\\+
setlocal spellfile=
setlocal spelllang=en_us,de_de,fr
setlocal statusline=
setlocal suffixesadd=
setlocal swapfile
setlocal synmaxcol=3000
if &syntax != ''
setlocal syntax=
endif
setlocal tabstop=4
setlocal tags=
setlocal textwidth=0
setlocal thesaurus=
setlocal noundofile
setlocal nowinfixheight
setlocal nowinfixwidth
set nowrap
setlocal nowrap
setlocal wrapmargin=0
silent! normal! zE
let s:l = 21 - ((20 * winheight(0) + 26) / 53)
if s:l < 1 | let s:l = 1 | endif
exe s:l
normal! zt
21
normal! 0
tabedit exclusive-read-results-endresult.txt
set splitbelow splitright
set nosplitbelow
set nosplitright
wincmd t
set winheight=1 winwidth=1
argglobal
edit exclusive-read-results-endresult.txt
setlocal keymap=
setlocal noarabic
setlocal noautoindent
setlocal balloonexpr=
setlocal nobinary
setlocal bufhidden=
setlocal buflisted
setlocal buftype=
setlocal nocindent
setlocal cinkeys=0{,0},0),:,!^F,o,O,e
setlocal cinoptions=
setlocal cinwords=if,else,while,do,for,switch
setlocal colorcolumn=
setlocal comments=s1:/*,mb:*,ex:*/,://,b:#,:%,:XCOMM,n:>,fb:-
setlocal commentstring=/*%s*/
setlocal complete=.,w,b,u,t,i
setlocal concealcursor=
setlocal conceallevel=0
setlocal completefunc=
setlocal nocopyindent
setlocal cryptmethod=
setlocal nocursorbind
setlocal nocursorcolumn
setlocal nocursorline
setlocal define=
setlocal dictionary=
setlocal nodiff
setlocal equalprg=
setlocal errorformat=
setlocal noexpandtab
if &filetype != 'text'
setlocal filetype=text
endif
setlocal foldcolumn=0
setlocal foldenable
setlocal foldexpr=0
setlocal foldignore=#
setlocal foldlevel=0
setlocal foldmarker={{{,}}}
setlocal foldmethod=manual
setlocal foldminlines=1
setlocal foldnestmax=20
setlocal foldtext=foldtext()
setlocal formatexpr=
setlocal formatoptions=tcq
setlocal formatlistpat=^\\s*\\d\\+[\\]:.)}\\t\ ]\\s*
setlocal grepprg=
setlocal iminsert=2
setlocal imsearch=2
setlocal include=
setlocal includeexpr=
setlocal indentexpr=
setlocal indentkeys=0{,0},:,0#,!^F,o,O,e
setlocal noinfercase
setlocal iskeyword=@,48-57,_,192-255
setlocal keywordprg=
setlocal nolinebreak
setlocal nolisp
setlocal nolist
setlocal makeprg=
setlocal matchpairs=(:),{:},[:]
setlocal modeline
setlocal modifiable
setlocal nrformats=octal,hex
set number
setlocal number
setlocal numberwidth=4
setlocal omnifunc=
setlocal path=
setlocal nopreserveindent
setlocal nopreviewwindow
setlocal quoteescape=\\
setlocal noreadonly
setlocal norelativenumber
setlocal norightleft
setlocal rightleftcmd=search
setlocal noscrollbind
setlocal shiftwidth=4
setlocal noshortname
setlocal nosmartindent
setlocal softtabstop=0
setlocal nospell
setlocal spellcapcheck=[.?!]\\_[\\])'\"\	\ ]\\+
setlocal spellfile=
setlocal spelllang=en_us,de_de,fr
setlocal statusline=
setlocal suffixesadd=
setlocal swapfile
setlocal synmaxcol=3000
if &syntax != 'text'
setlocal syntax=text
endif
setlocal tabstop=4
setlocal tags=
setlocal textwidth=0
setlocal thesaurus=
setlocal noundofile
setlocal nowinfixheight
setlocal nowinfixwidth
set nowrap
setlocal nowrap
setlocal wrapmargin=0
silent! normal! zE
let s:l = 207 - ((30 * winheight(0) + 26) / 53)
if s:l < 1 | let s:l = 1 | endif
exe s:l
normal! zt
207
normal! 037|
tabnext 3
if exists('s:wipebuf')
  silent exe 'bwipe ' . s:wipebuf
endif
unlet! s:wipebuf
set winheight=1 winwidth=20 shortmess=filnxtToO
let s:sx = expand("<sfile>:p:r")."x.vim"
if file_readable(s:sx)
  exe "source " . fnameescape(s:sx)
endif
let &so = s:so_save | let &siso = s:siso_save
doautoall SessionLoadPost
unlet SessionLoad
" vim: set ft=vim :
