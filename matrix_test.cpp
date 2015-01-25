#include <iostream>
#include <cassert>
#include <cmath>

using namespace std;

#include "matrix.h"
#include "test.h"

template <typename T, int N_>
void test_solve(int N) {
  matrix<double, N_, N_> A(N, N);
  matrix<double, N_, N_ == 0 ? 0 : 1> b(N, 1);

  for (int i = 0; i < N; i++) {
    b(i) = randf();
    for (int j = 0; j < N; j++) {
      A(i, j) = randf();
    }
  }

  matrix<double, N_, N_> A_(A);
  matrix<double, N_, N_ == 0 ? 0 : 1> b_(b);

  matrix_ref<double, N_, N_ == 0 ? 0 : 1> x = solve(A_, b_);
  
  matrix<double, N_, N_ == 0 ? 0 : 1> Ax = A*x;
  
  for (int i = 0; i < N; i++) {
    ASSERT_LT(abs(Ax(i) - b(i)), 1e-6);
  }
}

int main(int argc, const char **argv) {
  for (int i = 0; i < 100; i++) {
    for (int N = 0; N < 10; N++)
      test_solve<double, 0>(N);

    test_solve<double, 2>(2);
    test_solve<double, 5>(5);
    test_solve<double, 7>(7);
    test_solve<double, 10>(10);
    test_solve<double, 16>(16);
  }  
  return 0;
}