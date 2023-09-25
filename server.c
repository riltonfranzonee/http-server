#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define MAX_PENDING_CONNECTIONS 10
#define PORT 8888
#define BUFFER_SIZE 1024

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

    char* buffer = (char*) malloc(BUFFER_SIZE);
    memset(buffer, '\0', BUFFER_SIZE);

    int messageSize = recv(incomingSocketId, buffer, BUFFER_SIZE, 0);

    char* headerStart = strstr(buffer, "\n");
    char* headerEnd = strstr(buffer, "\r\n\r\n");

    int headerSize = headerEnd - headerStart;

    printf("just received a message! size: %d\n", headerSize);

    char* jsonResponse = (char*) malloc(BUFFER_SIZE);

    strcat(jsonResponse, "{ ");

    for (int i = 0; i < headerSize;) {
      // read line bt line
      char* lineStart = headerStart + i;
      char* lineEnd = strstr(lineStart + 1, "\n");
      int lineOffset = lineEnd - lineStart;

      char* contentStart = lineStart + 1; // ignore leading \n
      char* contentEnd = strstr(lineStart, "\r"); // ignore everything after \r
      int contentLength = contentEnd - contentStart;

      char* line = malloc(contentLength);
      strncpy(line, contentStart, contentLength);

      printf("line: %s\n", line);

      char* delimiter = strstr(line, ":");
      char val[200];
      memset(val, '\0', 200);
      strcpy(val, delimiter + 2);

      char key[200];
      memset(key, '\0', 200);
      strncpy(key, line, delimiter - line);

      size_t keyLen = strlen(key) + 3;
      char jsonKey[keyLen];
      snprintf(jsonKey, keyLen, "\"%s\"", key);
    
      size_t valueLen = strlen(val) + 3;
      char jsonValue[valueLen];
      snprintf(jsonValue, valueLen, "\"%s\"", val);

      printf("json key: %s\n", jsonKey);
      printf("json val: %s\n", jsonValue);

      memset(line, '\0', contentLength);
      free(line);

      i += lineOffset;
    }

    strcat(jsonResponse, " }");

    char str[] = "hello";

    send(incomingSocketId, (void*)str, 5, 0);

    free(buffer);
    close(incomingSocketId);
  }

  return 0;
}