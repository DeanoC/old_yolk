         V   U        ����������g8�B&&Z%Ѯ��'i��g�            u// PR c++/10926

struct Foo
{
    template <int i>
    ~Foo(); // { dg-error "" }
};
