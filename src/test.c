
#include "socket.c"

int main(void) {
	puts("\n\nStart of SimpleHTTPGet Test: \n\n");
	char const* host = "10.223.7.203";
	char const* file = "/mantisbt2/my_view_page.php";
	char const* add_info = 0;
	
	char* http_response = http_get(host, file, add_info);
	assert(http_response);
	size_t resp_len = strlen(http_response);
	printf("HTTP Response length: %zu\n", resp_len);
	
	puts("\n\nEnd of SimpleHTTPGet Test!\n\n");
	return 0;
}