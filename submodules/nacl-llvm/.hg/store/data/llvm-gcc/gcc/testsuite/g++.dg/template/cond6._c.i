         S   R        ��������
"����ݬ kdu�]�F5�            u// PR c++/27801

template<int> int foo(int i)
{
  return !( (1 && i) ? 0 : 1 );
}
