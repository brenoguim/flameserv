#pragma once

#include "Connection.h"
#include "Http.h"

#include <iostream>
#include <cstring>

class ThreadPool;

struct FlameServer
{
    void operator()(ThreadPool&)
    {
        std::cout << "Got a connection" << std::endl;
        char buffer[10*256];
        std::memset(buffer, 0, 10*256);
        auto n = read(conn.fd, buffer, 10*256-1);

        Response r = make_html_response(__PRETTY_FUNCTION__);
        auto buf = r.to_bytes();;
        n = write(conn.fd, buf.data(), buf.size());
        std::cout << "Wrote back " << n << " bytes" << std::endl;

    }

    Connection conn;
};
