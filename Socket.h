#pragma once

#include <cassert>
#include <optional>
#include <cstring>

#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

struct Socket
{
    explicit Socket(int pfd) : fd(pfd) {}

    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;

    Socket(Socket&& other) noexcept : fd(std::exchange(other.fd, -1)) {}
    Socket& operator=(Socket&& other) noexcept { std::swap(fd, other.fd); return *this; }

    ~Socket()
    {
        if (fd != -1) close(fd);
    }
    int fd {-1};
};

inline std::string read(const Socket& conn)
{
    char buffer[10*256];
    std::memset(buffer, 0, 10*256);
    auto n = read(conn.fd, buffer, 10*256-1);
    return std::string(buffer, n);
}

inline void write(const Socket& conn, std::string_view buf)
{
    while (!buf.empty())
    {
        auto n = write(conn.fd, buf.data(), buf.size());
        buf = buf.substr(n);
    }
}

