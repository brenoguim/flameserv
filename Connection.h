#pragma once

#include <cassert>
#include <optional>
#include <cstring>

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

inline std::string read(const Connection& conn)
{
    char buffer[10*256];
    std::memset(buffer, 0, 10*256);
    auto n = read(conn.fd, buffer, 10*256-1);
    return std::string(buffer, n);
}

inline void write(const Connection& conn, std::string_view buf)
{
    while (!buf.empty())
    {
        auto n = write(conn.fd, buf.data(), buf.size());
        buf = buf.substr(n);
    }
}

