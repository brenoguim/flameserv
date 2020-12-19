#pragma once

#include "Socket.h"
#include "Http.h"

#include <cstring>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <streambuf>
#include <unordered_set>

namespace fs = std::filesystem;

class ThreadPool;

const std::unordered_set<std::string> textExtensions {
      ".txt"
    , ".log"
};

const std::unordered_set<std::string> htmlExtensions {
      ".svg"
    , ".html"
};


struct FlameServer
{
    void operator()(ThreadPool&)
    {
        std::cout << "Got a connection" << std::endl;
        auto req = parse_html_request(read(conn));
        auto buf = getResponse(req.path).to_bytes();
        write(conn, buf);
        std::cout << "Wrote back " << buf.size() << " bytes" << std::endl;
    }

    Response getResponse(std::string_view path)
    {
        fs::path fspath {path};

        std::string body;
        if (fs::is_directory(fspath))
        {
            body = "<html><body>\n";
            for (const auto & entry : fs::directory_iterator(fspath, fs::directory_options::skip_permission_denied))
            {
                auto entrypath = entry.path().string();
                body += "<a href=\"" + entrypath + "\">" + entrypath + "</a><br>\n";
            }
            body += "\n</body></html>";
            return make_html_response(std::move(body));
        }
        else
        {
            if (textExtensions.count(fspath.extension()))
            {
                std::ifstream ifs(fspath.string());
                body = std::string((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
                return make_text_response(std::move(body));
            }
            else if (htmlExtensions.count(fspath.extension()))
            {
                std::ifstream ifs(fspath.string());
                body = std::string((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
                return make_html_response(std::move(body));
            }
            else
            {
                return make_html_response("We don't serve this kind of file here: " + fspath.string());
            }
        }
    }

    Socket conn;
};
