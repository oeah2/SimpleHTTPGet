
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "socket.h"

/* This code demonstrates how to use this library */

typedef struct HttpData HttpFunc(char const*const host, char const*const file, char const*const additional_info);

void get_and_print(char const*const host, char const*const file, char const*const additional_info, HttpFunc func) {
	struct HttpData http_response = func(host, file, additional_info);
	size_t response_length = strlen(http_response.data);
	printf("Host: %s\nHTTP Response Code: %d\nData length according to header: %zu\nReceived data bytes: %zu\nTotal Received bytes: %zu\n",
					host,
					http_response.http_code, 
					http_response.content_length, 
					response_length, 
					http_response.received_bytes);
	if(http_response.http_code != 200 && http_response.data)
		printf("Response Data: %s\n", http_response.data);
	printf("\n");
	
	if(http_response.data) free(http_response.data);
}

int main(void) {
	puts("Start of SimpleHTTPGet Test: \n");
	
	get_and_print("www.columbia.edu", "/~fdc/sample.html", 0, http_get);
	get_and_print("www.gogle.com", "/", 0, http_get);

	return 0;
}
