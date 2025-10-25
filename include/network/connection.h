/**
 * @file connection.h
 * @brief Network connection and HTTP/SSL utilities for prayer times API.
 *
 * Provides functions for establishing TCP and SSL/TLS connections,
 * making HTTP requests, and parsing HTTP responses. Used to communicate
 * with the MyQuran API for retrieving prayer times and city data.
 */

#ifndef CONNECTION_H
#define CONNECTION_H

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <stddef.h>
#include <sys/socket.h>

#define PORT                 443                  /**< HTTPS port number */
#define PORTSTR              "443"                /**< Port as string for getaddrinfo */
#define HOST                 "api.myquran.com"    /**< API host domain */
#define API_VERSION          "/v2"                /**< API version path */
#define CITY_ENDPOINT        "/sholat/kota/semua" /**< Cities endpoint */
#define PRAYER_TIME_ENDPOINT "/sholat/jadwal/:kota/:tahun/:bulan" /**< Prayer times endpoint */
#define ADDR_FAMILY          AF_INET                              /**< IPv4 address family */
#define ADDR_TYPE            SOCK_STREAM                          /**< TCP socket type */
#define RADIX                16 /**< Hexadecimal radix for chunk size parsing */
#define CHUNK_SIZE           4096

/**
 * @brief Structure representing a parsed HTTP response.
 *
 * Contains the separated header and body sections of an HTTP response,
 * along with the extracted status code.
 */
struct http_response {
  char *header;   /**< HTTP headers (null-terminated string) */
  char *body;     /**< HTTP body content (null-terminated string) */
  uint8_t status; /**< HTTP status code (e.g., 200, 404) */
};

/**
 * @brief Create a socket connection.
 *
 * Defines a socket connection with the following configuration:
 * - Address family: ADDR_FAMILY
 * - Type: ADDR_TYPE
 * - Protocol: 0 (TCP)
 *
 * @return On success, returns a non-negative integer (socket descriptor).
 *         On error, returns -1.
 */
int fsocket(void);

/**
 * @brief Establish a TCP connection to a remote host.
 *
 * Attempts to make a connection using a socket and a sockaddr structure
 * containing the peer address.
 *
 * @param sockfd   File descriptor associated with the socket (must be TCP).
 * @param host     Host address (e.g., domain name or IP).
 *
 * @return Returns 0 on success, or -1 on failure.
 */
int fconnect(const int sockfd);

/**
 * @brief Convert a hostname to its corresponding IP address.
 *
 * Since direct hostname connections are not supported in C sockets,
 * this function resolves the given domain into its first available IP.
 *
 * @param hostname   The name of the server (e.g., "example.com").
 * @param port       The port number as a string (e.g., "443" for HTTPS).
 * @param ipstr      Buffer to store the resulting IP string.
 * @param ipstr_len  Size of the IP buffer (e.g., sizeof(ipstr)).
 *
 * @return Returns 0 on success, or -1 on failure.
 */
int htoip(const char *hostname, const char *port, char *ipstr, size_t ipstr_len);

/**
 * @brief Initialize an SSL context.
 *
 * @return Returns a pointer to an initialized SSL_CTX object,
 *         or NULL on failure.
 */
SSL_CTX *ssl_init(void);

/**
 * @brief Establish an SSL/TLS connection.
 *
 * Initializes an SSL connection using the provided SSL context and socket.
 *
 * @param ctx       Pointer to the initialized SSL_CTX.
 * @param fd        Socket file descriptor.
 * @param hostname  The hostname of the server.
 *
 * @return Returns a pointer to an SSL object on success, or NULL on failure.
 *
 * @warning You must free the SSL session after use with ssl_session_free().
 */
SSL *ssl_connect(SSL_CTX *ctx, int fd, const char *hostname);

/**
 * @brief Free memory used by an SSL session.
 *
 * Calls SSL_shutdown() and SSL_free() to release resources.
 *
 * @param ssl   Pointer to the SSL session to free.
 */
void ssl_session_free(SSL *ssl);

/**
 * @brief Free memory used by an SSL context.
 *
 * Calls SSL_CTX_free() to release resources.
 *
 * @param ctx   Pointer to the SSL_CTX to free.
 */
void ssl_ctx_free(SSL_CTX *ctx);

/**
 * @brief Extract status code from a raw HTTP Response header
 *
 * The status line is always in the first line in the HTTP header and had the format
 *
 * `HTTP/<version> <status> <messages>`
 *
 * this function read status by looking at space between <status>
 * and convert it to integer
 *
 * @param header    raw c-string header
 *
 * @return Shall return non-negative number when success otherwise -1 will be returned
 */
int http_response_status_code(const char *header);

/**
 * @brief Extract raw http response to struct http_response
 *
 * return a http_response containing header and body extraction from
 * raw http response.
 *
 * @param raw_response   Full HTTP response as a C-string.
 *
 * @return Returns a struct http_response
 *
 * @warning the returned struct must be freed after used by calling
 *          http_response_free() function
 */
struct http_response http_response_extract(const char *raw_response);

/**
 * @brief Free memory allocation for http_response_extract
 *
 * currently http_response_extract_header call malloc to initiate a header response
 * this allocation need to be free after used, we make it cleaner by creating
 * dedicated free memory alocation function for http_response_extract
 *
 * @param response  a pointer for struct http_response that need to be free
 */
void http_response_free(struct http_response *response);

int get(const char *host, const char *path, struct http_response *dest);

#endif
