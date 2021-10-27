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
#include <stdbool.h>
#include <assert.h>
#include <unistd.h>
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <openssl/applink.c>
#else
#define __USE_XOPEN2K
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <fcntl.h>
#endif
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include "socket.h"


enum {
    SOCK_OK,
    SOCK_ERR_INIT,
    SOCK_ERR_ADDRINFO,
};

#ifdef DIAGNOSTIC
static char const*const error_msg = "Error in SimpleHTTPGet library";

static void myperror(size_t line, char const*const msg, int error) {
	if(msg) {
		size_t buff_len = strlen(error_msg) + strlen(msg) + 50;
		char buffer[buff_len];
		
		sprintf(buffer, "%s\t errno: %d, line: %zu: %s\n", error_msg, errno, line, msg);
		perror(buffer);
	}
}
#else
static void myperror(size_t line, char const*const msg, int error) {
	return;
}
#endif

/** \brief Initialize socket
 *
 */
static int socket_init(void)
{
#ifdef _WIN32
    WSADATA wsaData;

    if(WSAStartup(MAKEWORD(1,1), &wsaData)) {
    	myperror(__LINE__, "Error during Socket initialization.", 0);
        return SOCK_ERR_INIT;
    }
#endif
    return SOCK_OK;
}

/** \brief Deinitialize socket
 *
 */
static int socket_deinit(void)
{
#ifdef _WIN32
    return WSACleanup();
#else
    return 0;
#endif
}

static int get_last_error(void) {
	int ret = 0;
#ifdef _WIN32
	ret = WSAGetLastError();
#else
	ret = errno;
#endif
	return ret;
}

/** \brief Closes Socket
 *
 * \param sock_id int socket to be closed
 * \return int
 *
 */
static int socket_close(int sock_id)
{
#ifdef _WIN32
    return closesocket(sock_id);
#else
    return close(sock_id);
#endif
}


/** \brief Set socket to non blocking
 *
 * \param fd socket
 * \param blocking true for blocking
 * \return true on success
 *
 */
bool socket_set_blocking(int fd, bool blocking)
{
   if (fd < 0) return false;

#ifdef _WIN32
   unsigned long mode = blocking ? 0 : 1;
   return (ioctlsocket(fd, FIONBIO, &mode) == 0) ? true : false;
#else
   int flags = fcntl(fd, F_GETFL, 0);
   if (flags == -1) return false;
   flags = blocking ? (flags & ~O_NONBLOCK) : (flags | O_NONBLOCK);
   return (fcntl(fd, F_SETFL, flags) == 0) ? true : false;
#endif
}

/** \brief Connect to socket
 *
 * \param addr char const*const address information
 * \return int socket
 *
 */
static int socket_connect(char const*const addr)
{
    struct addrinfo hints = {0}, *res = 0;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if(getaddrinfo(addr, "http", &hints, &res)) {
    	int error = get_last_error();
    	myperror(__LINE__, "Error getting addrinfo.", error);
        return -1;
    }

    int s = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if(s == -1) {
    	int error = get_last_error();
    	myperror(__LINE__, "Error creating socket.", error);
    	return -1;
    }

    if(connect(s, res->ai_addr, res->ai_addrlen) == -1) {
    	int error = get_last_error();
    	myperror(__LINE__, "Error connecting to socket.", error);
    	return -1;
    }
    freeaddrinfo(res);
    return s;
}

/** \brief Send data oversocket
 *
 * \param sock_id int id of socket
 * \param msg char const* message to be send
 * \param msg_len size_t length of message
 * \return int
 *
 */
inline
static int socket_send(int sock_id, char const* msg, size_t msg_len)
{
    return send(sock_id, msg, msg_len, 0);
}

/** \brief Send whole message
 *
 * \param sock_id int id of socket
 * \param msg char const* message to be sent
 * \param msg_len size_t length of message
 * \return int
 *
 */
static int socket_sendall(int sock_id, char const* msg, size_t msg_len)
{
    size_t msg_sent = 0;
    do {
        msg_sent += socket_send(sock_id, msg, msg_len);
    } while (msg_sent < msg_len);
    return msg_sent;
}

/** \brief Receive data from socket
 *
 * \param sock_id int id of socket
 * \param msg char* destination array
 * \param max_len size_t max length of @p msg
 * \param flags int additional flags
 * \return int
 *
 */
inline
static int socket_receive(int sock_id, char* msg, size_t max_len, int flags)
{
    return recv(sock_id, msg, max_len, flags);
}

/** \brief Returns HTTP code
 *
 * \param http_response char const*const http response to be checked
 * \return int HTTP/1.1 code
 *
 */
static int http_get_http_code(char const*const http_response) {
	int ret = 0;
	if(http_response) {
		char* header_pos  = strstr(http_response, "HTTP/1.1");
		if(header_pos) {
			ret = strtoul(header_pos + strlen("HTTP/1.1"), NULL, 10);
		}
	}
	return ret;
}

/** \brief Checks http response for validity ("200 OK")
 *
 * \param http_response char const*const http response to be checked
 * \return bool true of OK, false otherwise
 *
 */
static bool http_is_response_ok(char const*const http_response)
{
	return http_get_http_code(http_response) == 200;
}

/** \brief Returns the content length of the http response according to the http header
 *
 * \param http_response char const*const http response of the server
 * \return size_t length according to http header
 *
 */
static size_t http_find_content_length(char const*const http_response)
{
    size_t ret = 0;
    if(http_is_response_ok(http_response)) {            // response valid
        if(strstr(http_response, "\r\n\r\n")) {         // header complete
            char* pos_length = strstr(http_response, "Content-Length: ");
            if(pos_length) {
				int scan = sscanf(pos_length, "Content-Length: %zu", &ret);
				assert(scan);
            }
        }
    }
    return ret;
}

/** \brief Returns whether the http reponse header has content length information
 *
 * \param http_response char const*const http response of the server
 * \return bool
 *
 */
static bool http_has_content_information(char const*const http_response) {
	return http_response && strstr(http_response, "Content-Length: ");
}

/** \brief Return the length of the http header
 *
 * \param http_response char const*const http response of the server
 * \return size_t length of header
 *
 */
static size_t http_find_header_length(char const*const http_response)
{
    size_t ret = 0;
    if(http_response) {
        char* pos_header_end = strstr(http_response, "\r\n\r\n") + strlen("\r\n\r\n");   // find end of header, -1 necessary?
        if(pos_header_end) {
            ptrdiff_t length = pos_header_end - http_response;
            ret = length;
        }
    }
    return ret;
}

/** \brief Generate user agent for http request
 *
 * \param void
 * \return char* string containing user agent, must be freed by user
 *
 */
static char* socket_get_useragent(char const*const user_agent)
{
	char* ret = 0;
	if(user_agent) {
		ret = malloc(100 * sizeof(char));
		if(ret) {
			strcpy(ret, "User-Agent: ");
			strcat(ret, user_agent);
			char* new_mem = realloc(ret, (strlen(ret) + 1) * sizeof(char));
			ret = new_mem ? new_mem : ret;
		}
	}
    return ret;
}


/** \brief Checks whether the http response is complete.
 * The actual content length must match the content length given in the http header.
 *
 * \param http_response char const*const http response of the server
 * \return bool true if complete, false otherwise
 *
 */
static bool http_is_response_complete(char const*const http_response)
{
    bool ret = false;
	if(http_is_response_ok(http_response)) {
		size_t header_length = http_find_header_length(http_response);
		size_t resp_setpoint = http_find_content_length(http_response);
		int actual_resp = strlen(http_response) - header_length;
		if(actual_resp > 0 && header_length && resp_setpoint && actual_resp == resp_setpoint) {
			ret = true;
		}
	}
    return ret;
}

/** \brief Creates http request. needs to be freed by the user
 *
 * \param host char const*const host to be connected
 * \param file char const*const file to be requested
 * \param add_info char const*const additional info to be placed into http header
 * \return char* string containing http 1.1 request
 *
 */
static char* http_create_request(char const*const host, char const*const file, char const*const add_info)
{
	char* request = 0;
    if(host && file) {
		size_t const header_max = 2000;
		request = calloc(header_max, sizeof(char));
		char const*const close = "close";
		//char const*const keep = "keep-alive";
		char const*const method = close;
		if(request) {
			sprintf(request, "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: %s\r\nAccept: text/plain\r\n%s\r\n\r\n", file, host, method, add_info ? add_info : "");
			char* new_req = realloc(request, strlen(request) + 1);
			if(new_req) {
				request = new_req;
			}
		}
	}
    return request;
}

/** \brief removes http header from http response
 *
 * \param http_response char* response of remote computer
 * \return char* containing http body
 *
 */
static char* http_remove_header(char* http_response)
{
    if(http_is_response_ok(http_response)) {
        if(strstr(http_response, "\r\n\r\n")) {
            size_t length = strlen(http_response);
            size_t header_length = http_find_header_length(http_response);
            assert(header_length > 0);
            char buffer[length - header_length + 1];
            strcpy(buffer, http_response + header_length);
            http_response = realloc(http_response, length - header_length + 1);
            if(http_response) {
                strcpy(http_response, buffer);
            }
        }
    }
    return http_response;
}

static char* http_get_error_msg(char* http_response) {
	char* ret = 0;
	if(http_response) {
		char* new_location = strstr(http_response, "Location: ");
		if(new_location) {
			new_location += strlen("Location: ");
			char* end_location = strstr(new_location, "\r\n");
			assert(end_location);
			size_t buff_len = end_location - new_location + 1;
			assert(buff_len);
			char buffer[buff_len];
			strncpy(buffer, new_location, buff_len - 1);
			buffer[buff_len - 1] = '\0';
			char* new_pos = realloc(http_response, buff_len * sizeof(char)); // Todo check
			if(new_pos)
				http_response = new_pos;
			//strcpy(new_pos, buffer);
			memcpy(new_pos, buffer, buff_len);
			ret = new_pos;
		}
	}
	return ret;
}

struct HttpData http_parse_header(char const*const data, size_t received_bytes) {
	struct HttpData ret = {0};
	if(data && received_bytes) {
		ret.http_code = http_get_http_code(data);
		ret.received_bytes = received_bytes;
		ret.received_data_length = received_bytes - http_find_header_length(data);
		ret.content_length = http_find_content_length(data);
	}
	return ret;
}

/** \brief Receive whole message from host
 *
 * \param sock_id int id of socket
 * \param msg char* destination array
 * \param max_len size_t max length of @p msg
 * \param flags int additional flags
 * \return int
 *
 */
static struct HttpData http_receiveall(int sock_id, char* msg, size_t max_len, int flags)
{
	struct HttpData ret = {0};
    int received = 0;
    int buff_pos = 0;
    int err_ret = 0;

    do {
        received = socket_receive(sock_id, msg + buff_pos, max_len - buff_pos, flags);
        if(received == -1) {
        	err_ret = get_last_error();
#ifdef _WIN32
			switch(err_ret) {
				case WSAEWOULDBLOCK:
				case WSAEINPROGRESS:
				case WSAEALREADY:
					break;
					
				default:
					goto ERR_RECV;
			}
#else
			switch(err_ret) {
			case EAGAIN:
				break;

			case ECONNRESET:
				goto END; // or goto ERR_RECV?

			default:
				goto ERR_RECV;
			}
#endif
        }
		if(received > 0) buff_pos += received;
        if(http_is_response_ok(msg)) {
        	if(http_is_response_complete(msg) || received == 0) {
        		break;
        	}
        }
        if(buff_pos && !http_is_response_ok(msg)) {
			ret.http_code = http_get_http_code(msg);
			ret.received_bytes = buff_pos;
			ret.data = http_get_error_msg(msg);
			goto ERR_RECV;
		}			
    } while(true);

END:
	ret = http_parse_header(msg, buff_pos);

    return ret;

ERR_RECV:
#ifdef _WIN32
    if(!ret.http_code) ret.http_code = err_ret;
#else
    if(!ret.http_code) ret.http_code = errno;
#endif // _WIN32
	char buffer[40];
	sprintf(buffer, "Error during recv, error msg: %d", ret.http_code);	
	int error = get_last_error();
	myperror(__LINE__, buffer, error);
    return ret;
}

/** \brief Connect to host and request file using HTTP. Add_info will be sent in request
 *
 * \param host char const*const address of host
 * \param file char const*const requested file
 * \param add_info char const*const additional info to be sent in header
 * \return char* server response, http header removed. 0 if no valid response
 *
 */
struct HttpData http_get(char const*const host, char const*const file, char const*const add_info)
{
    struct HttpData ret = {0};
    int s = 0;
    char* http_request = 0, *buffer = 0;
    if(host && file) {
        if(socket_init() != SOCK_OK) {
        	int error = get_last_error();
			myperror(__LINE__, "Error initializing socket", error);
			return ret;
		}
        s = socket_connect(host);
        http_request = http_create_request(host, file, add_info);
        if(!http_request) goto ERR_SOCKET;
        if(!socket_sendall(s, http_request, strlen(http_request) + 1)) goto ERR_SEND;
        size_t buf_len = 100E3;
        buffer = calloc(buf_len, sizeof(char));
        if(!buffer) goto ERR_SEND;
		size_t received_bytes = 0;
		if(!socket_set_blocking(s, false)) {
	    	int error = get_last_error();
			myperror(__LINE__, "Error setting socket to nonblocking", error);
			return ret;
		}
        ret = http_receiveall(s, buffer + received_bytes, buf_len, 0);
		if(!ret.received_bytes) 
			goto ERR_RECV;
		if(ret.http_code == 200) {
			if(!http_has_content_information(buffer) || http_is_response_complete(buffer)) {
				// Either no content length information or fully received
				buffer = http_remove_header(buffer);
				assert(buffer);
				ret.data = buffer;
			} else {
				goto ERR_RECV; // Could not receive fully
			}
		} else if(ret.http_code && ret.http_code != 200) {
			ret.data = buffer;
		}

        socket_close(s);
        free(http_request);
        socket_deinit();
    }
    return ret;

ERR_RECV:
	(void) ret;
	int error = get_last_error();
	myperror(__LINE__, "Error during receive", error);
    free(ret.data);
	ret.data = 0;
ERR_SEND:
    free(http_request);
ERR_SOCKET:
    socket_close(s);
    return ret;
}

bool socket_check_connection()      // Das ist keine schoene Loesung, sollte aber funktionieren.
{
    struct HttpData ret = http_get("www.google.com", "/", 0);
    if(ret.data) {
        free(ret.data);
        return true;
    }
    return false;
}

/** \brief Reports error message from openSSL library
 *
 */
static void report_and_exit(const char* msg)
{
	int error = get_last_error();
	myperror(__LINE__, msg, error);
    ERR_print_errors_fp(stderr);
    exit(-1);
}

/** \brief Initialize openSSL and HTTPS
 *
 * \return void
 *
 */
static void https_init()
{
    SSL_load_error_strings();
    SSL_library_init();
}

/** \brief Cleanup openSSL and HTTPS
 *
 * \param ctx SSL_CTX*
 * \param bio BIO*
 * \return void
 *
 */
static void https_cleanup(SSL_CTX* ctx, BIO* bio)
{
    SSL_CTX_free(ctx);
    BIO_free_all(bio);
}

/** \brief Connect via HTTP to host
 *
 * \param hostname const char* hostname to be connected to
 * \param ctx_in SSL_CTX**
 * \return BIO*
 *
 */
static BIO* https_connect(const char* hostname, SSL_CTX** ctx_in)
{
    size_t BuffSize = 1000;
    char name[BuffSize];

    const SSL_METHOD* method = TLS_client_method();
    if (NULL == method) report_and_exit("TLSv1_2_client_method...");

    *ctx_in = SSL_CTX_new(method);
    if (NULL == *ctx_in) report_and_exit("SSL_CTX_new...");

    BIO* bio = BIO_new_ssl_connect(*ctx_in);
    if (NULL == bio) report_and_exit("BIO_new_ssl_connect...");

    SSL* ssl = NULL;

    /* link bio channel, SSL session, and server endpoint */

    sprintf(name, "%s:%s", hostname, "https");
    BIO_get_ssl(bio, &ssl); /* session */
    SSL_set_mode(ssl, SSL_MODE_AUTO_RETRY); /* robustness */
    BIO_set_conn_hostname(bio, name); /* prepare to connect */

    /* try to connect */
    if (BIO_do_connect(bio) <= 0) {
        https_cleanup(*ctx_in, bio);
        report_and_exit("BIO_do_connect...");
    }

#define SKIP_VERIFICATION
#ifndef SKIP_VERIFICATION
    /* verify truststore, check cert */

    if (!SSL_CTX_load_verify_locations(*ctx_in,
                                       "/etc/ssl/certs/ca-certificates.crt", /* truststore */
                                       "/etc/ssl/certs/")) /* more truststore */
       report_and_exit("SSL_CTX_load_verify_locations...");

    long verify_flag = SSL_get_verify_result(ssl);
    if (verify_flag != X509_V_OK)
        fprintf(stderr,
                "##### Certificate verification error (%i) but continuing...\n",
                (int) verify_flag);

#endif // SKIP_VERIFICATION

    return bio;
}

/** \brief Receives https reponse from bio
 *
 * \param bio BIO*
 * \return char*
 *
 */
static struct HttpData https_receive(BIO* bio)
{
    size_t resp_len = 1E6, recv_len = 0;
    char* response = calloc(resp_len, sizeof(char));
    /* read HTTP response from server and print to stdout */
    while (1) {
        int n = BIO_read(bio, response + recv_len, resp_len - recv_len);
        if (n <= 0) break; /* 0 is end-of-stream, < 0 is an error */
        recv_len += n;
    }
    response = realloc(response, strlen(response) + 1);

    struct HttpData ret = http_parse_header(response, recv_len);
    ret.data = response;
    return ret;
}

struct HttpData https_get(char const*const host, char const*const file, char const*const add_info)
{
	struct HttpData ret = {0};
    https_init();
    SSL_CTX* ctx = NULL;
    BIO* bio = https_connect(host, &ctx);
    char* http_request = http_create_request(host, file, add_info);
    int sent_bytes = BIO_puts(bio, http_request);
    if(sent_bytes == -1 || sent_bytes == 0) {
    	int error = get_last_error();
    	myperror(__LINE__, "Error while sending data over HTTPS socket!", error);
        return ret;
    }
    assert(strlen(http_request) == sent_bytes);
    free(http_request);
    http_request = NULL;

    ret = https_receive(bio);
    if(ret.received_data_length != ret.content_length) {
    	if(ret.http_code != 200 || !http_has_content_information(ret.data)) {
			int error = get_last_error();
			myperror(__LINE__, "Error during receiving of https_get", error);
		}
	}
    https_cleanup(ctx, bio);
    if(http_is_response_ok(ret.data)) {
    	ret.data = http_remove_header(ret.data);
    }
    return ret;
}

struct HttpData https_get_with_useragent(char const*const host, char const*const file, char const*const user_agent, char const*const add_info)
{
	struct HttpData ret = {0};
    size_t buffer_length = 150;
    if(user_agent) {
		char* http_useragent = socket_get_useragent(user_agent);
        assert(strlen(http_useragent) < buffer_length - 1);
        if(add_info) {
            buffer_length += strlen(add_info);
        }
        char buffer[buffer_length];
        if(add_info) {
            strcpy(buffer, add_info);
            strcat(buffer, http_useragent);
        } else {
            strcpy(buffer, http_useragent);
        }
		free(http_useragent);
		
        ret = https_get(host, file, buffer);
    }
    return ret;
}
