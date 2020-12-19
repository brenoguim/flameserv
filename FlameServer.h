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
        auto req = parse_html_request(read(conn));

        std::string body = "<html><body>\n";

        fs::path fspath {req.path};

        if (fs::is_directory(fspath))
        {
            for (const auto & entry : fs::directory_iterator(fspath, fs::directory_options::skip_permission_denied))
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

        auto buf = make_html_response(std::move(body)).to_bytes();
        write(conn, buf);
        std::cout << "Wrote back " << buf.size() << " bytes" << std::endl;

    }

    Connection conn;
};
