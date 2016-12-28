#include <iostream>
using namespace std;

class A {
protected:
  int v;
public:
  A(int i) : v(i) {}
  virtual void set(int _v) {v = _v;}
  virtual int get() {return v;}
  void Print (ostream& os = cout) {
    os << "A [ " << v << " ]" << endl;
  }
};

class B : public A {
protected:
  int x;
public:
  B(int i) : A(i), x(i * 2) {}
  virtual void set(int _x) {x = _x;}
  virtual int get() {return x;}
  void Print (ostream& os = cout) {
    os << "B [ " << v << ", " << x << " ]" << endl;
  }
};

int main() {
  B x(123);
  x.Print();
  A xx = dynamic_cast<A&>(x);
  xx.Print();
  x.Print();

  return 0;
}
