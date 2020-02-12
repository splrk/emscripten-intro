#include <math.h>
#include <emscripten.h>

EMSCRIPTEN_KEEPALIVE
double size(double x, double y) {
  return sqrt(x * x + y * y);
}
