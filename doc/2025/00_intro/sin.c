
#include <stdio.h>
#include <math.h>

const double PI = 3.14159265358979323846;

int myfun(double a) {
  double result = sin(a);
  printf("myfun(%.3f) = %.3f\n", a, result);
  return 0;
}

int main() {
  myfun(0.0);
  myfun(PI/2);
  myfun(PI);
  return 0;
}