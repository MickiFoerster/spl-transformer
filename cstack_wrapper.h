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
#ifdef _cstack_h_
	#error "error: You have to include cstack_wrapper.h before cstack.h."
#endif

#define  a1_STACKc_push(v)             ad_push(a1_STACKc, a1_STACKc_c, v)
#define  a1_STACKi_push(v)             ad_push(a1_STACKi, a1_STACKi_c, v)
#define  a1_STACKf_push(v)             ad_push(a1_STACKf, a1_STACKf_c, v)
#define  a1_STACKo_push(v)             ad_push(a1_STACKo, a1_STACKo_c, v)

#define  a1_STACKc_top()               ad_top(a1_STACKc, a1_STACKc_c)
#define  a1_STACKi_top()               ad_top(a1_STACKi, a1_STACKi_c)
#define  a1_STACKf_top()               ad_top(a1_STACKf, a1_STACKf_c)
#define  a1_STACKo_top()               ad_top(a1_STACKo, a1_STACKo_c)

#define  a1_STACKc_pop()               ad_pop(a1_STACKc, a1_STACKc_c)
#define  a1_STACKi_pop()               ad_pop(a1_STACKi, a1_STACKi_c)
#define  a1_STACKf_pop()               ad_pop(a1_STACKf, a1_STACKf_c)
#define  a1_STACKo_pop()               ad_pop(a1_STACKo, a1_STACKo_c)

#define  a1_STACKc_empty()             ad_empty(a1_STACKc, a1_STACKc_c)
#define  a1_STACKo_empty()             ad_empty(a1_STACKo, a1_STACKo_c)

#define  a1_STACKc_init()              ad_init(a1_STACKc, a1_STACKc_c, uint)
#define  a1_STACKi_init()              ad_init(a1_STACKi, a1_STACKi_c, int)
#define  a1_STACKf_init()              ad_init(a1_STACKf, a1_STACKf_c, double)

#define  a1_STACKc_deallocate()        ad_dealloc(a1_STACKc, a1_STACKc_c)
#define  a1_STACKi_deallocate()        ad_dealloc(a1_STACKi, a1_STACKi_c)
#define  a1_STACKf_deallocate()        ad_dealloc(a1_STACKf, a1_STACKf_c)

#define  a1_STACKc_clear()        	   ad_clear(a1_STACKc_c)
#define  a1_STACKi_clear()             ad_clear(a1_STACKi_c)
#define  a1_STACKf_clear()             ad_clear(a1_STACKf_c)

#define  a1_STACKc_nest_init()         ad_init_nested(a1_STACKc, a1_STACKc_c, uint)
#define  a1_STACKi_nest_init()         ad_init_nested(a1_STACKi, a1_STACKi_c, int)
#define  a1_STACKf_nest_init()         ad_init_nested(a1_STACKf, a1_STACKf_c, double)





#define  t2_STACKc_push(v)             ad_push(t2_STACKc, t2_STACKc_c, v)
#define  t2_STACKi_push(v)             ad_push(t2_STACKi, t2_STACKi_c, v)
#define  t2_STACKf_push(v)             ad_push(t2_STACKf, t2_STACKf_c, v)
#define  t2_STACKo_push(v)             ad_push(t2_STACKo, t2_STACKo_c, v)

#define  t2_STACKc_top()               ad_top(t2_STACKc, t2_STACKc_c)
#define  t2_STACKi_top()               ad_top(t2_STACKi, t2_STACKi_c)
#define  t2_STACKf_top()               ad_top(t2_STACKf, t2_STACKf_c)
#define  t2_STACKo_top()               ad_top(t2_STACKo, t2_STACKo_c)

#define  t2_STACKc_pop()               ad_pop(t2_STACKc, t2_STACKc_c)
#define  t2_STACKi_pop()               ad_pop(t2_STACKi, t2_STACKi_c)
#define  t2_STACKf_pop()               ad_pop(t2_STACKf, t2_STACKf_c)
#define  t2_STACKo_pop()               ad_pop(t2_STACKo, t2_STACKo_c)

#define  t2_STACKc_empty()             ad_empty(t2_STACKc, t2_STACKc_c)
#define  t2_STACKo_empty()             ad_empty(t2_STACKo, t2_STACKo_c)

#define  t2_STACKc_init()              ad_init(t2_STACKc, t2_STACKc_c, uint)
#define  t2_STACKi_init()              ad_init(t2_STACKi, t2_STACKi_c, int)
#define  t2_STACKf_init()              ad_init(t2_STACKf, t2_STACKf_c, double)

#define  t2_STACKc_deallocate()        ad_dealloc(t2_STACKc, t2_STACKc_c)
#define  t2_STACKi_deallocate()        ad_dealloc(t2_STACKi, t2_STACKi_c)
#define  t2_STACKf_deallocate()        ad_dealloc(t2_STACKf, t2_STACKf_c)

#define  t2_STACKc_clear()        	   ad_clear(t2_STACKc_c)
#define  t2_STACKi_clear()             ad_clear(t2_STACKi_c)
#define  t2_STACKf_clear()             ad_clear(t2_STACKf_c)




#define  a2_STACKc_push(v)             ad_push(a2_STACKc, a2_STACKc_c, v)
#define  a2_STACKi_push(v)             ad_push(a2_STACKi, a2_STACKi_c, v)
#define  a2_STACKf_push(v)             ad_push(a2_STACKf, a2_STACKf_c, v)
#define  a2_STACKo_push(v)             ad_push(a2_STACKo, a2_STACKo_c, v)

#define  a2_STACKc_top()               ad_top(a2_STACKc, a2_STACKc_c)
#define  a2_STACKi_top()               ad_top(a2_STACKi, a2_STACKi_c)
#define  a2_STACKf_top()               ad_top(a2_STACKf, a2_STACKf_c)
#define  a2_STACKo_top()               ad_top(a2_STACKo, a2_STACKo_c)

#define  a2_STACKc_pop()               ad_pop(a2_STACKc, a2_STACKc_c)
#define  a2_STACKi_pop()               ad_pop(a2_STACKi, a2_STACKi_c)
#define  a2_STACKf_pop()               ad_pop(a2_STACKf, a2_STACKf_c)
#define  a2_STACKo_pop()               ad_pop(a2_STACKo, a2_STACKo_c)

#define  a2_STACKc_empty()             ad_empty(a2_STACKc, a2_STACKc_c)
#define  a2_STACKo_empty()             ad_empty(a2_STACKo, a2_STACKo_c)

#define  a2_STACKc_init()              ad_init(a2_STACKc, a2_STACKc_c, uint)
#define  a2_STACKi_init()              ad_init(a2_STACKi, a2_STACKi_c, int)
#define  a2_STACKf_init()              ad_init(a2_STACKf, a2_STACKf_c, double)

#define  a2_STACKc_deallocate()        ad_dealloc(a2_STACKc, a2_STACKc_c)
#define  a2_STACKi_deallocate()        ad_dealloc(a2_STACKi, a2_STACKi_c)
#define  a2_STACKf_deallocate()        ad_dealloc(a2_STACKf, a2_STACKf_c)

#define  a2_STACKc_clear()        	   ad_clear(a2_STACKc_c)
#define  a2_STACKi_clear()             ad_clear(a2_STACKi_c)
#define  a2_STACKf_clear()             ad_clear(a2_STACKf_c)
