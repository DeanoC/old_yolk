         \   [        ��������j���>ɓc�U�:�R�            u// PR 27451
// { dg-do compile }

void foo()
{
  asm("" ::: X); // { dg-error "before" }
}
