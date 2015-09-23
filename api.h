#ifndef API_H
#define API_H

#include <curl/curl.h>

typedef CURL * glackAPIHandle;

glackAPIHandle glack_API_init(void);

void glack_API_cleanup(glackAPIHandle handle);

// glack_API_call(handle, method, key1, value1, key2, value2, ..., NULL);
char *glack_API_call(glackAPIHandle handle, const char *method, ...);

#endif
