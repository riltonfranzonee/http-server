#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define MAX_PENDING_CONNECTIONS 10
#define PORT 8888
#define BUFFER_SIZE 4096

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
    int incomingSocketId = accept(socketId, (struct sockaddr*) &client, &clientlen);
    
    char buffer[BUFFER_SIZE];

    ssize_t messageSize = recv(incomingSocketId, buffer, BUFFER_SIZE, 0);

    char* headerStart = strstr(buffer, "\n");
    char* headerEnd = strstr(buffer, "\r\n\r\n");

    ssize_t headerSize = headerEnd - headerStart;

    printf("just received a message! size: %lu\n", headerSize);

    char* jsonResponse = (char*) malloc(headerSize * 2);

    strcat(jsonResponse, "{\n");
    
    for (int i = 0; i < headerSize;) {
      // read line by line
      char* lineStart = headerStart + i;
      char* lineEnd = strstr(lineStart + 1, "\n");
      int lineOffset = lineEnd - lineStart;

      char* contentStart = lineStart + 1; // ignore leading \n
      char* contentEnd = strstr(lineStart, "\r"); // ignore everything after \r
      int contentLength = contentEnd - contentStart;

      char* line = malloc(contentLength);
      strncpy(line, contentStart, contentLength);

      char* key = strtok(line, ": ");
      char* value = strtok(NULL, " ");
      char keyValuePair[1000];

      if(i == 0) {
        sprintf(keyValuePair, "\"%s\": \"%s\"", key, value);
      } else {
        sprintf(keyValuePair, ",\n\"%s\": \"%s\"", key, value);
      }

      strcat(jsonResponse, keyValuePair);     

      memset(line, '\0', contentLength);
      free(line);

      i += lineOffset;
    }

    strcat(jsonResponse, "\n}");

    char* resHeader = "HTTP/1.1 200 OK\r\nContent-Type: text/json\r\n\r\n";

    send(incomingSocketId, resHeader, strlen(resHeader), 0);
    send(incomingSocketId, jsonResponse, strlen(jsonResponse), 0);

    free(jsonResponse);
    close(incomingSocketId);
  }

  return 0;
}