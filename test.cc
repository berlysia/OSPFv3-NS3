#include <iostream>
#include <vector>
using namespace std;

struct A {
    int type;
    int v;
    A(){type = 0;}
    virtual void Dummy() {}
    virtual void Print () const {
       cout << "A: " << v << endl;
    }
};

struct B : public A {
    int x;
    B(){type = 1;}
    virtual void Print () const {
       cout << "B: " << v << ", " << x << endl;
    }
};

// int main() {
//     B b; b.v = 345; b.x = 4567;
//     b.Print();

//     A &a = b;
//     a.Print();

//     return 0;
// }
