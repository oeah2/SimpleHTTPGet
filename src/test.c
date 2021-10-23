
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
	resp_len = strlen(http_response);
	printf("HTTP Response length: %zu\n", resp_len);
	free(http_response);
	
	http_response = https_get("www.google.com", "/", 0);
	assert(http_response);
	resp_len = strlen(http_response);
	printf("HTTPS Response length: %zu\n", resp_len);
	free(http_response);

	puts("\n\nEnd of SimpleHTTPGet Test!\n\n");
	return 0;
}
