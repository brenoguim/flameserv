#pragma once

#include "Connection.h"
#include "Http.h"

#include <cstring>
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

class ThreadPool;

struct FlameServer
{
    void operator()(ThreadPool&)
    {
        std::cout << "Got a connection" << std::endl;
        char buffer[10*256];
        std::memset(buffer, 0, 10*256);
        auto n = read(conn.fd, buffer, 10*256-1);

        auto req = parse_html_request({buffer, static_cast<unsigned long>(n)});

        std::string body = "<html><body>\n";

        fs::path fspath {req.path};

        if (fs::is_directory(fspath))
        {
            for (const auto & entry : fs::directory_iterator(fspath))
            {
                auto entrypath = entry.path().string();
                body += "<a href=\"" + entrypath + "\">" + entrypath + "</a><br>\n";
            }
        }
        else
        {
            body += "You would see this file now: " + fspath.string();
        }

        body += "\n</body></html>";

        Response r = make_html_response(std::move(body));
        auto buf = r.to_bytes();;
        n = write(conn.fd, buf.data(), buf.size());
        std::cout << "Wrote back " << n << " bytes" << std::endl;

    }

    Connection conn;
};
