         f   e        ���������֧�(��$�0�g?�r6��            u// PR c++/15076

struct Y { Y(int &); };

int v;
Y y1(reinterpret_cast<int>(v));  // { dg-error "" }
