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

#include <stdbool.h>

/** \brief Data is handled between this library and the caller through this struct */
struct HttpData {
	int http_code; /**< @brief HTTP Response code of the requested server */
	size_t received_bytes; /**< @brief The total number of received bytes, including HTTP header */
	size_t received_data_length; /**< @brief The total number of received data bytes, excluding HTTP header */
	size_t content_length; /**< @brief The content length of the HTTP response, according to the HTTP header sent by the server */
	char *data;
};

/** \brief This enum is used to tell the library, what method should be used to fetch data in an threaded call */
enum HttpCommand {
	HttpCommand_GetHttp, /**< @brief Request data using regular HTTP */
	HttpCommand_GetHttps, /**< @brief Request data using encrypted HTTPS */
	HttpCommand_GetHttpsUserAgent, /**< @brief Request Data using encrypted HTTPS and send a defined User agent identifer */
};

typedef void HttpCallback(pthread_t threadID, struct HttpData); /**< @brief A Callback Function for this library shall have this form */

/** \brief A very simple http request is being made and the result returned. The returned string needs to be freed by the user
 * \details This function initializes the socket interface, connects to @p host, requests @p file and adds @p add_info into the request header.
 The returned message is being checked for validity. If valid, the http header is removed and the http body returned.
 *
 * \param host char const*const host to be connected
 * \param file char const*const file to be requested
 * \param add_info char const*const Additional informations to be placed into the http request header. If no additional info shall be placed into header, set 0
 * \return char*
 *
 */
struct HttpData http_get(char const *const host, char const *const file,
		char const *const add_info);

/** \brief Checks the internet availability
 *  \details A HTTP request is sent to google and the reponse is checked for validity.
 * \return bool true of internet is available, false otherwise
 *
 */
bool socket_check_connection();

/** \brief A very simple http request is being made and the result returned. The returned string needs to be freed by the user
 * \details This function initializes the socket interface, connects to @p host, requests @p file and adds @p add_info into the request header.
 The returned message is being checked for validity. If valid, the http header is removed and the http body returned.
 *
 * \param host char const*const host to be connected
 * \param file char const*const file to be requested
 * \param add_info char const*const Additional informations to be placed into the http request header. If no additional info shall be placed into header, set 0
 * \return struct HttpData
 *
 */
struct HttpData https_get(char const *const host, char const *const file,
		char const *const add_info);

/** \brief A very simple http request is being made and the result returned. The returned string needs to be freed by the user. This function additionally transmits the user agent.
 * \details This function initializes the socket interface, connects to @p host, requests @p file and adds @p add_info into the request header.
 The returned message is being checked for validity. If valid, the http header is removed and the http body returned.
 *
 * \param host char const*const host to be connected
 * \param file char const*const file to be requested
 * \param user_agent char const*const string containing application name, the string is internally processed to be http conforming 
 * \param add_info char const*const Additional informations to be placed into the http request header. If no additional info shall be placed into header, set 0
 * \return struct HttpData
 *
 */
struct HttpData https_get_with_useragent(char const *const host,
		char const *const file, char const *const user_agent,
		char const *const add_info);

/** \brief Based on the value of @p command, an HTTP or HTTPS request is made in a parallel thread. When finished, @p callback_func is called.
 *
 * \param command enum HttpCommand Determines whether an HTTP, HTTPS or HTTPS with user agent request is made
 * \param host char const*const host to be connected
 * \param file char const*const file to be requested
 * \param user_agent char const*const string containing application name, the string is internally processed to be http conforming
 * \param add_info char const*const Additional informations to be placed into the http request header. If no additional info shall be placed into header, set 0
 * \param callback_func HttpCallback Callback function to be called when the data is fully fetched or the connection timed out
 * \param timeout_ms Maximum time to wait before a timeout event happens
 * \return int thread ID
 *
 */
pthread_t http_get_with_thread(enum HttpCommand command, char const *const host,
		char const *const file, char const *const user_agent,
		char const *const add_info, int timeout_ms,
		HttpCallback *callback_func);
