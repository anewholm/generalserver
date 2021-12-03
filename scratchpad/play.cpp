#include <iostream>
using namespace std;

class C;

class I {
public:
  virtual C *queryInterface(C *p) = 0;
};

class C: virtual public I {
public:
  C *queryInterface(C *p) {cout << "C::queryInterface\n"; return this;}
};

class A: public C, virtual public I {
public:
};

int main() {
  cout << "play.cpp 2:\n";
  A a;
  C *c = a.queryInterface((C*) 0);
  cout << "complete\n";
}
