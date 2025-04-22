#ifndef SCAN_H
#define SCAN_H

#include <stdlib.h>
#include "struct.h"

char** scan_bmp_files(const char *path, size_t *out_count);

#endif