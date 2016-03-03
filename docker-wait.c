// Copyright 2016 Google Inc. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// docker-wait is is a C program that waits for a docker container to exit. The
// docker built-in command `docker wait` takes at least 20MB memory, while this
// tiny program only consumes 7K memory if compiled statically. It will consumes
// about 700K memory if compiled dynamically, but the binary size is smaller.

#include <sys/socket.h>
#include <unistd.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
  int sock, rval;
  struct sockaddr_un server;
  char request[128] = {0};
  char response[512] = {0};
  if (argc != 2) {
    printf("Usage: docker-wait CONTAINER\n\
            Block until a container stops, then returns its exit code.\n");
    exit(1);
  }
  char* docker_socket_path = getenv("DOCKER_SOCKET_PATH");
  if (docker_socket_path == NULL) {
    docker_socket_path = "/var/run/docker.sock";
  }
  if (strlen(docker_socket_path) >= sizeof(server.sun_path)) {
    printf("DOCKER_SOCKET_PATH is too long: %s\n", docker_socket_path);
    exit(1);
  }
  if ((sock = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
    perror("fail to open stream socket");
    exit(1);
  }

  server.sun_family = AF_UNIX;
  strcpy(server.sun_path, docker_socket_path);
  if (connect(
          sock, (struct sockaddr*)&server, sizeof(struct sockaddr_un)) < 0) {
    perror("socket");
    exit(1);
  }
  sprintf(request, "POST /containers/%s/wait HTTP/1.0\r\n\r\n", argv[1]);
  int request_len = strlen(request);
  if ((rval = write(sock, request, request_len)) != request_len) {
    perror("write");
    exit(1);
  }
  if ((rval = read(sock, response, sizeof(response))) < 0) {
    perror("read");
    exit(1);
  }
  close(sock);

  char* msg = strstr(response, "\r\n\r\n") + 4;
  printf("docker wait response:\n%s\n", msg);
  if (strstr(msg, "no such id")) {
    exit(1);
  }
  char* status_code_str = strstr(msg, "{\"StatusCode\":");
  if (status_code_str == NULL) {
    exit(1);
  }
  // skip the prefix "{\"StatusCode\":" which has 14 characters.
  status_code_str += 14;
  char* status_code_end = strstr(status_code_str, "}");
  *status_code_end = 0;
  int status_code = atoi(status_code_str);
  exit(status_code);
}
