#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define MAX_PENDING_CONNECTIONS 10
#define PORT 8888
#define BUFFER_SIZE 4096

// converts a string of "key: value\n" lines into a JSON object
char* to_json(char* key_value_lines, ssize_t string_size) {
  char* jsonResponse = (char*) malloc(string_size * 2);
  strcpy(jsonResponse, ""); // initialize it as an empty string

  strcat(jsonResponse, "{\n");
  
  for (int offset = 0; offset < string_size;) {
    char* line_start = key_value_lines + offset;
    char* line_end = strstr(line_start + 1, "\n");
    int line_offset = line_end - line_start;

    char* content_start = line_start + 1; // ignore leading \n
    char* content_end = strstr(line_start, "\r"); // ignore everything after \r
    int content_length = content_end - content_start;

    char* line = (char*) malloc(content_length);
    strncpy(line, content_start, content_length);

    char* key = strtok(line, ": ");
    char* value = strtok(NULL, " ");
    char key_value_pair[1000];

    if(offset == 0) {
      sprintf(key_value_pair, "\"%s\": \"%s\"", key, value);
    } else {
      sprintf(key_value_pair, ",\n\"%s\": \"%s\"", key, value);
    }

    strcat(jsonResponse, key_value_pair);
    memset(line, '\0', content_length);
    free(line);

    offset += line_offset;
  }

  strcat(jsonResponse, "\n}");

  return jsonResponse;
}

int main() {
  int socketId = socket(PF_INET, SOCK_STREAM, 0);

  struct sockaddr_in server, client;

  server.sin_family = AF_INET;
  server.sin_port = htons(PORT);
  server.sin_addr.s_addr = inet_addr("127.0.0.1");

  socklen_t serverlen = (socklen_t) sizeof(server);
  socklen_t clientlen = (socklen_t) sizeof(client);

  bind(socketId, (struct sockaddr*) &server, serverlen);

  listen(socketId, MAX_PENDING_CONNECTIONS);

  printf("server listening on %d\n", PORT);

  while(1) {
    int client_socket = accept(socketId, (struct sockaddr*) &client, &clientlen);
    
    char buffer[BUFFER_SIZE];

    recv(client_socket, buffer, BUFFER_SIZE, 0);

    char* header_start = strstr(buffer, "\n");
    char* header_end = strstr(buffer, "\r\n\r\n");
    ssize_t header_size = header_end - header_start;

    char* headers_as_json = to_json(header_start, header_size);

    char* response_header = "HTTP/1.1 200 OK\r\nContent-Type: text/json\r\n\r\n";

    send(client_socket, response_header, strlen(response_header), 0);
    send(client_socket, headers_as_json, strlen(headers_as_json), 0);

    free(headers_as_json);
    close(client_socket);
  }

  return 0;
}