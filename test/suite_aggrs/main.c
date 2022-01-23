/*

 Package: dyncall
 Library: test
 File: test/call_aggrs/main.c
 Description: 
 License:

   Copyright (c) 2022 Tassilo Philipp <tphilipp@potion-studios.com>

   Permission to use, copy, modify, and distribute this software for any
   purpose with or without fee is hereby granted, provided that the above
   copyright notice and this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
   WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
   MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
   ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
   WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
   ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
   OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

*/

#include "dyncall.h"
#include "dyncall_struct.h"
#include "globals.h"
#include <string.h>
#include "../common/platformInit.h"
#include "../common/platformInit.c" /* Impl. for functions only used in this translation unit */


void* G_callvm;


static int find_agg_idx(int* len, const char* sig)
{
  for(int i=0; i<G_naggs; ++i) {
    const char* agg_sig = G_agg_sigs[i];
    *len = strlen(agg_sig);
    if(strncmp(agg_sig, sig, *len) == 0)
      return i;
  }
  return -1;
}

static int invoke(char const* signature, void* t)
{
  DCCallVM   * p = (DCCallVM*) G_callvm;
  char const * sig = signature;
  char const * sig_args;
  char         rtype;
  DCstruct *   rtype_st = NULL;
  funptr       rtype_st_cmp = NULL;
  char         atype;
  int          pos = 0;
  int          s = 0;

  clear_V();

  dcReset(p);

  if(*sig == '{') {
    int len;
    int i = find_agg_idx(&len, sig);
    if(i == -1) {
      printf("unknown rtype sig at '%s' ;", sig);
      return 0;
    }
    rtype = *sig;
    sig += len;

	rtype_st_cmp = G_agg_cmpfuncs[i];
    rtype_st = ((DCstruct*(*)())G_agg_newdcstfuncs[i])(NULL);
    dcBeginCallStruct(p, rtype_st, DC_FALSE);
  }
  else
    rtype = *sig++;

  sig_args = sig;

  while ( (atype = *sig) != '\0') {
    pos++;
    switch(atype) {
      case 'c': dcArgChar    (p,K_c[pos]); break;
      case 's': dcArgShort   (p,K_s[pos]); break;
      case 'i': dcArgInt     (p,K_i[pos]); break;
      case 'j': dcArgLong    (p,K_j[pos]); break;
      case 'l': dcArgLongLong(p,K_l[pos]); break;
      case 'p': dcArgPointer (p,K_p[pos]); break;
      case 'f': dcArgFloat   (p,K_f[pos]); break;
      case 'd': dcArgDouble  (p,K_d[pos]); break;
      case '{': {
        /* find struct sig */
        int len;
        int i = find_agg_idx(&len, sig);
        if(i == -1) {
          printf("unknown sig at '%s' ;", sig);
          return 0;
        }
        DCstruct *st = ((DCstruct*(*)())G_agg_newdcstfuncs[i])(NULL);
        dcArgStruct(p, st, K_a[pos]);
        sig += len-1; /* advance to next arg char */
        break;
      }
      default: printf("unknown atype '%c' (1) ;", atype); return 0;
    }
    ++sig;
  }
  
  switch(rtype) 
  {
    case 'v': dcCallVoid(p,t); s=1; /*TODO:check that no return-arg was touched.*/ break;
    case 'c': s = (dcCallChar    (p,t) == K_c[pos]) ; break;
    case 's': s = (dcCallShort   (p,t) == K_s[pos]) ; break;
    case 'i': s = (dcCallInt     (p,t) == K_i[pos]) ; break;
    case 'j': s = (dcCallLong    (p,t) == K_j[pos]) ; break;
    case 'l': s = (dcCallLongLong(p,t) == K_l[pos]) ; break;
    case 'p': s = (dcCallPointer (p,t) == K_p[pos]) ; break;
    case 'f': s = (dcCallFloat   (p,t) == K_f[pos]) ; break;
    case 'd': s = (dcCallDouble  (p,t) == K_d[pos]) ; break;
    case '{': {
      s = ((int(*)())rtype_st_cmp)(dcCallStruct(p,t,rtype_st, V_a[pos]), K_a[pos]);
	  break;
	}
    default: printf("unknown rtype '%c'", rtype); return 0;
  }

  if (!s) { printf("rval wrong;"); return 0; }

  /* test V_* array against values passed to func: */
  sig = sig_args;
  pos = 1;
  while ( (atype = *sig) != '\0') {
    switch(atype) {
      case 'c': s = ( V_c[pos] == K_c[pos] ); if (!s) printf("'c':%d: %d != %d ; ",     pos, V_c[pos], K_c[pos]); break;
      case 's': s = ( V_s[pos] == K_s[pos] ); if (!s) printf("'s':%d: %d != %d ; ",     pos, V_s[pos], K_s[pos]); break;
      case 'i': s = ( V_i[pos] == K_i[pos] ); if (!s) printf("'i':%d: %d != %d ; ",     pos, V_i[pos], K_i[pos]); break;
      case 'j': s = ( V_j[pos] == K_j[pos] ); if (!s) printf("'j':%d: %ld != %ld ; ",   pos, V_j[pos], K_j[pos]); break;
      case 'l': s = ( V_l[pos] == K_l[pos] ); if (!s) printf("'l':%d: %lld != %lld ; ", pos, V_l[pos], K_l[pos]); break;
      case 'p': s = ( V_p[pos] == K_p[pos] ); if (!s) printf("'p':%d: %p != %p ; ",     pos, V_p[pos], K_p[pos]); break;
      case 'f': s = ( V_f[pos] == K_f[pos] ); if (!s) printf("'f':%d: %f != %f ; ",     pos, V_f[pos], K_f[pos]); break;
      case 'd': s = ( V_d[pos] == K_d[pos] ); if (!s) printf("'d':%d: %f != %f ; ",     pos, V_d[pos], K_d[pos]); break;
      case '{': {
        /* no check: guaranteed to exist, or invoke func would've exited when passing args, above */
        int len;
        int i = find_agg_idx(&len, sig);
        s = ((int(*)())G_agg_cmpfuncs[i])(V_a[pos], K_a[pos]);
        if (!s) printf("'{':%d:  *%p != *%p ; ", pos, V_a[pos], K_a[pos]);
        sig += len-1; /* advance to next arg char */
        break;
      }
      default: printf("unknown atype '%c' ; ", atype); return 0;
    }
    if (!s) {
      printf("arg mismatch at %d ; ", pos);
      return 0;
    }
    ++sig;
    ++pos;
  }
  return 1;
}

int run_test(int i)
{  
  char const * sig;
  void * target;
  int success;
  sig = G_sigtab[i];
  target = (void*) G_funtab[i];
  printf("%d:%s:",i,sig);
  success = invoke(sig,target);
  printf("%d\n",success);
  return success;
}

int run_all()
{
  int i;
  int failure = 0;
  for(i=0;i<G_ncases;++i)
    failure |= !( run_test(i) );

  return !failure;
}

int main(int argc, char* argv[])
{
  int total;

  dcTest_initPlatform();

  init_K(G_maxargs);
  G_callvm = (DCCallVM*) dcNewCallVM(32768);
  dcReset(G_callvm);
  total = run_all();
  printf("result: call_aggrs: %d\n", total);

  dcTest_deInitPlatform();

  return !total;
}
