
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "socket.h"

/* This code demonstrates how to use this library */

int main(void) {
	puts("Start of SimpleHTTPGet Test: \n");
	char const* host = "www.google.com"; 	/* Host-Server */
	char const* file = "/"; 				/* Requested file */
	char const* add_info = 0; 				/* Additional information to be passed in the HTTP request */
	
	struct HttpData http_response = http_get(host, file, add_info);
	if(http_response.data) {
		puts("HTTP get success!");
		size_t response_length = strlen(http_response.data);
		printf("HTTP Response length: %zu\t HTTP Received data bytes: %zu\t Received bytes: %zu\n", http_response.content_length, response_length, http_response.received_bytes);
		free(http_response.data);
	} else
		puts("Error during http_get.");
	return 0;
}