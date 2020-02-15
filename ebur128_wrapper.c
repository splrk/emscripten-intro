#include <stdio.h>
#include <emscripten.h>
#include "libebur128/ebur128/ebur128.h"

EMSCRIPTEN_KEEPALIVE
char* get_version() {
    static char version[8];
    int major;
    int minor;
    int patch;

    ebur128_get_version(&major, &minor, &patch);
    sprintf(version, "%d.%d.%d", major, minor, patch);
    
    return version;
}
