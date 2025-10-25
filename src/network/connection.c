/**
 * @file connection.c
 * @brief Implementation of network connection and HTTP/SSL utilities.
 *
 * Contains the implementation of TCP socket operations, SSL/TLS connections,
 * hostname resolution, and HTTP response parsing functions.
 */

#include "network/connection.h"
#include <stdio.h>
#include <unistd.h>

int fsocket() { return socket(AF_INET, SOCK_STREAM, 0); }

int fconnect(const int sockfd) {
  if (sockfd < 0) {
    perror("Creating socket is failure");
    return -1;
  }

  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));

  char ip[INET_ADDRSTRLEN];

  int hostname_to_ip = htoip(HOST, PORTSTR, ip, sizeof(ip));

  if (hostname_to_ip < 0) {
    printf("htoip error\n");
    close(sockfd);
    return -1;
  }

  addr.sin_family = AF_INET;
  addr.sin_port = htons(PORT);

  if (inet_pton(AF_INET, ip, &addr.sin_addr.s_addr) < 1) {
    perror("Failed to create server address");
    close(sockfd);
    return -1;
  }

  return connect(sockfd, (const struct sockaddr *)&addr, sizeof(addr));
}

int htoip(const char *hostname, const char *port, char *ipstr, size_t ipstr_len) {
  struct addrinfo hints, *res, *p;
  memset(&hints, 0, sizeof(hints));

  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;

  int status = getaddrinfo(hostname, port, &hints, &res);
  if (status != 0) {
    fprintf(stderr, "getaddrinfo %s\n", gai_strerror(status));
    return -1;
  }

  for (p = res; p != NULL; p = p->ai_next) {
    struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
    if (inet_ntop(AF_INET, &(ipv4->sin_addr), ipstr, ipstr_len) == NULL) {
      perror("inet_ntop");
      freeaddrinfo(res);
      return -1;
    }

    break;
  }

  if (p == NULL) {
    fprintf(stderr, "Failed to connect into any address\n");
    freeaddrinfo(res);
    return -1;
  }

  freeaddrinfo(res);

  return 0;
}

SSL_CTX *ssl_init() {
  SSL_CTX *ctx;

  ctx = SSL_CTX_new(TLS_client_method());
  if (!ctx) {
    ERR_print_errors_fp(stderr);
    return NULL;
  }

  return ctx;
}

SSL *ssl_connect(SSL_CTX *ctx, int fd, const char *hostname) {
  if (ctx == NULL) {
    printf("ssl_connect SSL_CTX NULL\n");
    return NULL;
  }

  SSL *ssl;
  ssl = SSL_new(ctx);
  if (!ssl) {
    ERR_print_errors_fp(stderr);
    return NULL;
  }

  if (SSL_set_tlsext_host_name(ssl, hostname) != 1) {
    ERR_print_errors_fp(stderr);
    SSL_free(ssl);
    return NULL;
  }

  SSL_set_fd(ssl, fd);

  if (SSL_connect(ssl) <= 0) {
    ERR_print_errors_fp(stderr);
    SSL_free(ssl);
    return NULL;
  }

  return ssl;
}

static void ssl_session_free(SSL *ssl) {
  if (ssl) {
    SSL_shutdown(ssl);
    SSL_free(ssl);
  }
}

static void ssl_ctx_free(SSL_CTX *ctx) {
  if (ctx) {
    SSL_CTX_free(ctx);
  }
}

char *http_response_extract_header(const char *raw_response) {
  if (raw_response == NULL) {
    printf("find_header() raw_response is NULL\n");
    return NULL;
  }

  const char *body = strstr(raw_response, "\r\n\r\n");
  if (!body) {
    printf("No Content");
    return NULL;
  }

  size_t header_len = body - raw_response;

  // +1 is for null terminator
  char *headers = malloc(header_len + 1);
  if (headers == NULL) {
    printf("find_header() Cannot allocate memory\n");
    return NULL;
  }

  strncpy(headers, raw_response, header_len);
  headers[header_len] = '\0';
  return headers;
}

int http_response_status_code(const char *header) {
  if (header == NULL) {
    printf("http_response_status_code header NULL");
    return -1;
  }

  const char *space1 = strchr(header, ' ');
  if (space1 == NULL) {
    printf("http_response_status_code header NULL");
    return -1;
  }

  const char *space2 = strchr(space1 + 1, ' ');
  if (space2 == NULL) {
    printf("http_response_status_code header NULL");
    return -1;
  }

  size_t len = space2 - space1 - 1;
  char status[4] = "";
  if (len > 3)
    len = 3;

  strncpy(status, space1 + 1, len);
  status[len] = '\0';

  return atoi(status);
}

char *http_response_extract_body(const char *raw_response) {
  if (raw_response == NULL) {
    printf("find_body() raw_response is NULL\n");
    return NULL;
  }

  const char *body = strstr(raw_response, "\r\n\r\n");
  if (!body) {
    printf("No Content\n");
    return NULL;
  }

  // skip \r\n\r\n
  body += 4;

  if (!strstr(raw_response, "Transfer-Encoding: chunked")) {
    return strndup(body, strlen(body));
  }

  const char *p = body;
  char *output = malloc(strlen(body) + 1);
  if (output == NULL) {
    printf("http_response_extract_body cannot allocate memory for output");
    return NULL;
  }

  char *out = output;

  while (*p) {
    while (*p == '\r' || *p == '\n')
      p++;

    char *endptr;
    long chunk_size = strtol(p, &endptr, RADIX);
    if (chunk_size <= 0)
      break;

    p = strchr(p, '\n');
    if (p == NULL)
      break;
    p++;

    memcpy(out, p, chunk_size);
    out += chunk_size;
    p += chunk_size;

    if (p[0] == '\r' && p[1] == '\n')
      p += 2;
  }
  *out = '\0';
  return output;
}

struct http_response http_response_extract(const char *raw_response) {
  struct http_response response;
  memset(&response, 0, sizeof(response));
  char *header = http_response_extract_header(raw_response);

  response.header = header;
  response.body = http_response_extract_body(raw_response);
  response.status = http_response_status_code(header);

  return response;
}

void http_response_free(struct http_response *response) {
  if (response == NULL) {
    printf("http_response respose is NULL");
    return;
  }

  free(response->header);
  free(response->body);
}

int get(const char *host, const char *path, struct http_response *dest) {
  SSL_CTX *ctx = ssl_init();
  if (ctx == NULL) {
    fprintf(stderr, "SSL_CTX NULL\n");
    return -1;
  }

  int sockfd = fsocket();
  if (fconnect(sockfd) < 0) {
    fprintf(stderr, "Failed to connect");
    ssl_ctx_free(ctx);
    close(sockfd);
    return -1;
  }

  SSL *ssl = ssl_connect(ctx, sockfd, HOST);
  if (ssl == NULL) {
    ssl_ctx_free(ctx);
    close(sockfd);
    return -1;
  }

  int request_len = snprintf(NULL, 0,
                             "GET %s HTTP/1.1\r\n"
                             "Host: %s\r\n"
                             "Connection: close\r\n\r\n",
                             path, host);

  char *request = malloc(request_len + 1);
  if (request == NULL) {
    fprintf(stderr, "Cannot allocate memory\n");
    ssl_session_free(ssl);
    ssl_ctx_free(ctx);
    close(sockfd);
    return -1;
  }

  snprintf(request, request_len + 1,
           "GET %s HTTP/1.1\r\n"
           "Host: %s\r\n"
           "Connection: close\r\n\r\n",
           path, host);

  int send_request = SSL_write(ssl, request, request_len);
  free(request);

  if (send_request != request_len) {
    fprintf(stderr, "GET request to %s%s fail", host, path);
    ssl_session_free(ssl);
    ssl_ctx_free(ctx);
    close(sockfd);
    return -1;
  }

  char *response = NULL;
  size_t total = 0, capacity = 0;

  char buffer[CHUNK_SIZE];
  int rv;

  while ((rv = SSL_read(ssl, buffer, sizeof(buffer))) > 0) {
    size_t tmp_capacity = (capacity == 0) ? CHUNK_SIZE : capacity * 2;

    if (total + rv >= tmp_capacity) {
      tmp_capacity = total + rv + 1;
    }

    char *tmp_response = realloc(response, tmp_capacity);
    if (tmp_response == NULL) {
      fprintf(stderr, "Cannot realloc response\n");
      ssl_session_free(ssl);
      ssl_ctx_free(ctx);
      close(sockfd);
      free(response);
      return -1;
    }

    response = tmp_response;
    memcpy(response + total, buffer, rv);
    total += rv;
    capacity = tmp_capacity;
  }

  if (response != NULL) {
    response[total] = '\0';
  }

  ssl_session_free(ssl);
  ssl_ctx_free(ctx);
  close(sockfd);

  struct http_response response_extract = http_response_extract(response);
  if (response_extract.header == NULL && response_extract.body == NULL) {
    fprintf(stderr, "Cannot extract json response or response invalid\n");
    free(response);
    return -1;
  }

  *dest = response_extract;

  free(response);
  return 0;
}
