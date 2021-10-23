
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "socket.h"

/* This code demonstrates how to use this library */

void get_and_print(char const*const host, char const*const file, char const*const additional_info) {
	struct HttpData http_response = http_get(host, file, additional_info);
	size_t response_length = strlen(http_response.data);
	printf("HTTP Response Code: %d\tHTTP Response length: %zu\t HTTP Received data bytes: %zu\t Received bytes: %zu\n", http_response.http_code, http_response.content_length, response_length, http_response.received_bytes);
	if(http_response.data) free(http_response.data);
}

int main(void) {
	puts("Start of SimpleHTTPGet Test: \n");
	
	get_and_print("www.columbia.edu", "/~fdc/sample.html", 0);
	get_and_print("www.gogle.com", "/", 0);

	return 0;
}
