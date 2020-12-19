#pragma once

#include <cassert>
#include <optional>

#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

struct Connection
{
    explicit Connection(int pfd) : fd(pfd) {}

    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;

    Connection(Connection&& other) noexcept : fd(std::exchange(other.fd, -1)) {}
    Connection& operator=(Connection&& other) noexcept { std::swap(fd, other.fd); return *this; }

    ~Connection()
    {
        if (fd != -1) close(fd);
    }
    int fd {-1};
};
