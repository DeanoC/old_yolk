         X   W        ���������]�ˮC7&oD��䐰6���            u// PR c++/24560

struct A { void f(); };
void g() { A().f.a; } // { dg-error "class" }
