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

#include "socket.c"

int main(void) {
	puts("\n\nStart of SimpleHTTPGet Test: \n\n");
	char const* host = "www.columbia.edu";
	char const* file = "/~fdc/sample.html";
	char const* add_info = 0;
	
	char* http_response = http_get(host, file, add_info).data;
	assert(http_response);
	size_t resp_len = strlen(http_response);
	printf("HTTP Response length: %zu\n",  resp_len);
	fflush(stdout);
	free(http_response);
	
	http_response = http_get("www.gogle.com", "/", 0).data;
	assert(http_response);
	if(*http_response)
		resp_len = strlen(http_response);
	else
		resp_len = 0;
	printf("HTTP Response length: %zu\n", resp_len);
	if(http_response) free(http_response);
	
	http_response = https_get("www.google.com", "/", 0);
	assert(http_response);
	if(*http_response)
		resp_len = strlen(http_response);
	else
		resp_len = 0;
	printf("HTTPS Response length: %zu\n", resp_len);
	free(http_response);

	puts("\n\nEnd of SimpleHTTPGet Test!\n\n");
	return 0;
}
