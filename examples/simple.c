#include <stdio.h>
#include "simple.peg.c"

int main()
{
  while (yyparse());
  return 0;
}
