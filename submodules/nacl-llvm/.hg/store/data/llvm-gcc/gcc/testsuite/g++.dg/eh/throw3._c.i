         B   A        ��������!�2n����j���q�%�,+�            u// PR c++/19312

struct A {};

void foo(A a)
{
    throw (A)a;
}
