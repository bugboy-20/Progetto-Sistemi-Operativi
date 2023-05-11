#include <pandos_utils.h>

void *memcpy(void *dest, const void * src, size_t n) {
    char byte;
    for(int i=0; i<n; i++) {
        byte = *((char*) src+i);
        *((char*) dest+i) = byte;

    }
    return dest;
}
