         i   h        ����������S>�u���M3�����            u// PR c++/16904

template<typename T> struct X
{
  X() { this->T::i; } // { dg-error "" }
};

X<int> x;
