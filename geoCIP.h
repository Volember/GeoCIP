#ifndef IPLOCATE_H
#define IPLOCATE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
typedef SOCKET socket_t;
#define INVALID_SOCKET_VAL INVALID_SOCKET
#define SOCKET_ERROR_VAL SOCKET_ERROR
#define CLOSE_SOCKET(s) closesocket(s)
#define GET_LAST_ERROR() WSAGetLastError()
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
typedef int socket_t;
#define INVALID_SOCKET_VAL -1
#define SOCKET_ERROR_VAL -1
#define CLOSE_SOCKET(s) close(s)
#define GET_LAST_ERROR() errno
#endif

typedef struct
{
  char country[128];
  char countryCode[16];
  char city[128];
  char timezone[128];
  char isp[128];
  char ip[64];
  int success;
} ip_info_t;

/* Function prototypes */
int iplocate_init(void);
void iplocate_cleanup(void);
ip_info_t iplocate_get_info(void);

#ifdef IPLOCATE_IMPLEMENTATION

int iplocate_init(void)
{
#ifdef _WIN32
  WSADATA wsaData;
  if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
  {
    return 0;
  }
#endif
  return 1;
}

void iplocate_cleanup(void)
{
#ifdef _WIN32
  WSACleanup();
#endif
}

static socket_t connect_to_host(const char *host, const char *port)
{
  struct addrinfo hints, *res, *p;
  socket_t sockfd = INVALID_SOCKET_VAL;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  if (getaddrinfo(host, port, &hints, &res) != 0)
  {
    return INVALID_SOCKET_VAL;
  }

  for (p = res; p != NULL; p = p->ai_next)
  {
    sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if (sockfd == INVALID_SOCKET_VAL)
      continue;

    if (connect(sockfd, p->ai_addr, p->ai_addrlen) == SOCKET_ERROR_VAL)
    {
      CLOSE_SOCKET(sockfd);
      sockfd = INVALID_SOCKET_VAL;
      continue;
    }
    break;
  }

  freeaddrinfo(res);
  return sockfd;
}

ip_info_t iplocate_get_info(void)
{
  ip_info_t info;
  memset(&info, 0, sizeof(info));
  info.success = 0;

  socket_t sockfd = connect_to_host("ip-api.com", "80");
  if (sockfd == INVALID_SOCKET_VAL)
    return info;

  const char *request = "GET /line/ HTTP/1.0\r\nHost: ip-api.com\r\nUser-Agent: iplocate-c-lib\r\nConnection: close\r\n\r\n";
  if (send(sockfd, request, (int)strlen(request), 0) == SOCKET_ERROR_VAL)
  {
    CLOSE_SOCKET(sockfd);
    return info;
  }

  char buffer[4096];
  int total_received = 0;
  int received;
  while ((received = recv(sockfd, buffer + total_received, sizeof(buffer) - total_received - 1, 0)) > 0)
  {
    total_received += received;
    if (total_received >= (int)sizeof(buffer) - 1)
      break;
  }
  buffer[total_received] = '\0';
  CLOSE_SOCKET(sockfd);

  /* Skip HTTP headers */
  char *body = strstr(buffer, "\r\n\r\n");
  if (body)
  {
    body += 4;
  }
  else
  {
    body = buffer;
  }

  /* Parse ip-api /line format manually to handle empty lines */
  char *current = body;
  int line_count = 0;
  while (current && *current)
  {
    char *next_line = strchr(current, '\n');
    if (next_line)
      *next_line = '\0';

    // Remove possible \r
    char *r = strchr(current, '\r');
    if (r)
      *r = '\0';

    line_count++;
    if (line_count == 1)
    { // status
      if (strcmp(current, "success") == 0)
        info.success = 1;
      else
        break;
    }
    else if (line_count == 2)
      strncpy(info.country, current, sizeof(info.country) - 1);
    else if (line_count == 3)
      strncpy(info.countryCode, current, sizeof(info.countryCode) - 1);
    else if (line_count == 6)
      strncpy(info.city, current, sizeof(info.city) - 1);
    else if (line_count == 10)
      strncpy(info.timezone, current, sizeof(info.timezone) - 1);
    else if (line_count == 11)
      strncpy(info.isp, current, sizeof(info.isp) - 1);
    else if (line_count == 14)
      strncpy(info.ip, current, sizeof(info.ip) - 1);

    if (!next_line)
      break;
    current = next_line + 1;
  }

  return info;
}

#endif /* IPLOCATE_IMPLEMENTATION */

#endif /* IPLOCATE_H */
