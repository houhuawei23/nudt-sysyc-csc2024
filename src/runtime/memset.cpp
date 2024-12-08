

extern "C" {
void _memset(int* a, int len) {
  for (int i = 0; i * 4 < len; i++) {
    a[i] = 0;
  }
}
}

