         P   O        ���������|�OB�`�̬+�PۃN            u// PR c++/18177

void foo()
{
  X; // { dg-error "" }
  const_cast<int&>(X);
}
