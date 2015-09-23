#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "api.h"

#define GLACK_API_URL "https://slack.com/api/"

#define CURL_ERROR fprintf(stderr, "libcurl %s\n", curl_easy_strerror(status))

glackAPIHandle glack_API_init(void)
{
	CURLcode status = curl_global_init(CURL_GLOBAL_SSL);

	if (status != CURLE_OK)
	{
		CURL_ERROR;
		return NULL;
	}

	return curl_easy_init();
}

void glack_API_cleanup(glackAPIHandle handle)
{
	curl_easy_cleanup(handle);
	curl_global_cleanup();
}

struct Response
{
	char *mem;

	size_t size;
};

static size_t write_callback(char *ptr, size_t size, size_t nmemb, void *data)
{
	size_t bytes_in = size * nmemb;

	if (bytes_in == 0)
		return 0;

	struct Response *resp = (struct Response *)data;

	char *tmp = realloc(resp->mem, resp->size + bytes_in + 1);

	if(tmp == NULL)
		return 0;

	resp->mem = tmp;

	memcpy(resp->mem + resp->size, ptr, bytes_in);
	resp->size += bytes_in;
	resp->mem[resp->size] = '\0';

	return bytes_in;
}

char *glack_API_call(glackAPIHandle handle, const char *method, ...)
{
	va_list args;
	va_start(args, method);

	// Construct POST data

	char *post_data = malloc(1);

	if (post_data == NULL)
	{
		va_end(args);
		return NULL;
	}

	size_t total_size = 0;

	char *name = va_arg(args, char *);

	while (name != NULL)
	{
		char *value = va_arg(args, char *);

		size_t expand_by = strlen(name) + strlen(value) + 2;

		char *tmp = realloc(post_data, total_size + expand_by + 1);

		if (tmp == NULL)
		{
			free(post_data);
			va_end(args);
			perror("realloc()");
			return NULL;
		}

		post_data = tmp;

		sprintf(post_data + total_size, "%s=%s&", name, value);

		total_size += expand_by;

		name = va_arg(args, char *);
	}

	post_data[total_size - 1] = '\0';

	// Prepare request

	char *url = malloc(sizeof(GLACK_API_URL) + strlen(method));

	if (url == NULL)
	{
		free(post_data);
		va_end(args);
		perror("malloc()");
		return NULL;
	}

	sprintf(url, "%s%s", GLACK_API_URL, method);

	struct Response resp = { .size = 0 };

	resp.mem = malloc(1);

	if (resp.mem == NULL)
	{
		free(post_data);
		free(url);
		va_end(args);
		perror("malloc()");
		return NULL;
	}

	curl_easy_setopt(handle, CURLOPT_URL, url);
	curl_easy_setopt(handle, CURLOPT_POSTFIELDS, post_data);
	curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, write_callback);
	curl_easy_setopt(handle, CURLOPT_WRITEDATA, &resp);

	// Perform request

	CURLcode status = curl_easy_perform(handle);

	char *retval;

	if (status == CURLE_OK)
		retval = resp.mem;
	else
	{
		retval = NULL;
		CURL_ERROR;
	}

	// Cleanup

	free(post_data);
	free(url);
	va_end(args);

	// Return

	return retval;
}
