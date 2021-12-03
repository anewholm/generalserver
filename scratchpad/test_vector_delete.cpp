#include <iostream>
#include <vector>
using namespace std;

class MyObject {
  int i;
public:
  ~MyObject() {cout << "dying\n"; }
};

int main() {
  vector<MyObject*> vec;
  vector<MyObject*>::iterator iVec;
  vec.push_back(new MyObject());
  for (iVec = vec.begin(); iVec != vec.end(); iVec++) delete (*iVec);
  vec.clear();
}
