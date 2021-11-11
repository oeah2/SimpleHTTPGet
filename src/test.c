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

static struct HttpData dataTest;
static _Atomic(bool) finished;

void Callback(pthread_t threadID, struct HttpData data_local) {
	dataTest = data_local;
	finished = true;
}

int main(void) {
	puts("\n\nStart of SimpleHTTPGet Test: \n\n");
	finished = false;
	http_get_with_thread(HttpCommand_GetHttps, "www.google.com", "/", 0, 0, 0, Callback);
	sleep(2);
	if(finished) {
		printf("Http Code: %d\n", dataTest.http_code);
	}	
	fflush(stdout);
	return;
	
	char const* host = "www.columbia.edu";
	char const* file = "/~fdc/sample.html";
	char const* add_info = 0;
	size_t resp_len = 0;
	char* http_response = NULL;
	
	http_response = http_get(host, file, add_info).data;
	assert(http_response);
	resp_len = strlen(http_response);
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
	
	struct HttpData data = https_get("www.gogle.com", "/", 0);
	assert(data.data);
	if(*data.data)
		resp_len = strlen(data.data);
	else
		resp_len = 0;
	printf("HTTPS Response length: %zu\n", resp_len);
	free(http_response);
	

	puts("\n\nEnd of SimpleHTTPGet Test!\n\n");
	return 0;
}
