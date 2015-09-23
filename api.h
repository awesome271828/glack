#ifndef API_H
#define API_H

#include <curl/curl.h>

typedef CURL * glackAPIHandle;

glackAPIHandle glack_API_init(void);

void glack_API_cleanup(glackAPIHandle handle);

char *glack_API_call(glackAPIHandle handle, const char *method, ...);

#endif
