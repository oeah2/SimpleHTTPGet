
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
	
	char* http_response = http_get(host, file, add_info);
	if(http_response) {
		puts("HTTP get success!");
		size_t response_length = strlen(http_response);
		printf("HTTP Response length: %zu\n", response_length);
		free(http_response);
	} else
		puts("Error during http_get.");
	return 0;
}