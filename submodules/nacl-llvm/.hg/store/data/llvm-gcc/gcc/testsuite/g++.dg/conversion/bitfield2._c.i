         S   R        ��������-bD��H�X>��fOz�            u// PR c++/27292

struct A
{
  int i : 8;
};

bool foo(A a)
{
  return int(a.i);
}
