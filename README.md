# Lightweight Docker wait tool

`docker-wait` is a C program that waits for a docker container to exit. The
docker built-in command `docker wait` takes at least 20MB memory, while this tiny
program only consumes 7K memory if compiled statically. It will consumes about
700K memory if compiled dynamically, but the binary size is smaller. This will
save the baseline memory for services running on container-optimized operating
system.

## How to build the tool

You can install any GCC compiler and compiled the program statically using the following command:

```
gcc -static -O3 -o docker-wait docker-wait.c
```

## How to run the tool

```
docker-wait CONTAINER_NAME
```

This program connects to the default docker socket file `/var/run/docker.sock`.
But you can override the default value by setting environment variable
`DOCKER_SOCKET_PATH`.

## Licensing

- See [LICENSE][1]

[1]: LICENSE
