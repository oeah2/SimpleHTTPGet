
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
	
	bool first_run = true;
	struct HttpData http_response;
GET:
	http_response = http_get(host, file, add_info);
	puts("HTTP get success!");
	size_t response_length = strlen(http_response.data);
	printf("HTTP Response Code: %d\tHTTP Response length: %zu\t HTTP Received data bytes: %zu\t Received bytes: %zu\n", http_response.http_code, http_response.content_length, response_length, http_response.received_bytes);
	if(http_response.data) free(http_response.data);
	
	host = "www.gogle.de";
	if(first_run) {
		first_run = false;
		goto GET;
	}

	puts("Error during http_get.");
	return 0;
}