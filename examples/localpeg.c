#include <stdio.h>

#define YY_CTX_LOCAL

#include "simple.peg.c"

int main()
{
  yycontext ctx;
  memset(&ctx, 0, sizeof(yycontext));
  while (yyparse(&ctx));
  return 0;
}
