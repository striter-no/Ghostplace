#pragma once
#include <netinet/in.h> 
#include <stdlib.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>

// Используем функцию для полного чтения
ssize_t full_read(int fd, void *buf, size_t count);

// Используем функцию для полной записи
ssize_t full_write(int fd, const void *buf, size_t count);