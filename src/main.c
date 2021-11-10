/*
   Simple HTTP Get Library
   Copyright (C) 2021 Ahmet Öztürk
   Version 0.1

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

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
	
	// HTTP Test
	get_and_print("www.columbia.edu", "/~fdc/sample.html", 0, http_get);
	get_and_print("www.gogle.com", "/", 0, http_get);

	// HTTPS Test
	get_and_print("www.google.com", "/", 0, https_get);
	get_and_print("www.gogle.com", "/", 0, https_get);

	return 0;
}
