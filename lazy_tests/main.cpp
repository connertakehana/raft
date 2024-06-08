#include <chrono>
#include <random>
#include <thread>
#include <iostream>
#include <vector>
int main() {
  std::vector<int> v;
  auto itt = std::lower_bound(v.begin(), v.end(), 1);;
  bool thing1 = itt == v.begin();
  auto thing2 = itt == v.end();
  printf("%d, %d\n", thing1, thing2);
  
}