         _   ^        ����������y'������Yt	g��i�             u// PR c++/31273

enum E { e };
struct S {
  E v:5;
};
S s;
int main() { if (!s.v) return 0; }
