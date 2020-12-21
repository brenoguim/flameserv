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

inline std::optional<std::string_view> matchAndTake(std::string_view path, std::string_view prefix)
{
    if (path.find(prefix) == 0) return path.substr(prefix.size());
    return {};
}

struct FlameServer
{
    void operator()(ThreadPool&)
    {
        std::cout << "Got a connection" << std::endl;
        auto req = parse_html_request(conn);

        auto response = [&] {
            if (req.isGet())
            {
                if (auto path = matchAndTake(req.path, "/flamegraph"))
                    return getFlameGraphResponse(*path);
                if (auto path = matchAndTake(req.path, "/browse"))
                    return getBrowseResponse(*path);
            }
            if (req.isPost())
            {
                if (auto path = matchAndTake(req.path, "/flamegraph"))
                    return getFlameGraphPostResponse(req, *path);
            }
            return getIndexResponse(req.path);
        }();

        auto buf = response.to_bytes();
        write(conn, buf);
        std::cout << "Wrote back " << buf.size() << " bytes" << std::endl;
    }

    Response getIndexResponse(std::string_view path)
    {
        auto str =
        "<html><head><title>FlameServ</title></head>"
        "<body>"
        "<a href=\"/browse/home/breno\"> Browse over all files </a> <br>"
        "<a href=\"/flamegraph\"> See what Flamegraphs are available </a> <br>"
        "</body></html>";
	return make_html_response(str);
    }

    Response getFlameGraphResponse(std::string_view path)
    {
        return getBrowseResponse(fs::absolute(fs::path{"data"} / path).string());
    }

    Response getFlameGraphPostResponse(const Request& r, std::string_view path)
    {
        path.remove_prefix(1); // remove slash
        if (path.size() == 0)
            return make_html_response("Error: Trying to post a file with no name");
        if (path.find('/') != std::string::npos)
            return make_html_response("Error: Trying to post a file with slash");

        fs::path fspath{"./data"};
        fspath /= path;

        std::fstream of(fspath.string(), std::fstream::out);
        if (!of.is_open())
            return make_html_response("Error: Could not open file");
        of << r.m_body;
        of.close();

        return make_html_response("Your file has been posted");
    }

    Response getBrowseResponse(std::string_view path)
    {
        fs::path fspath {path};

        std::string body;
        if (fs::is_directory(fspath))
        {
            body = "<html><body>\n";
            for (const auto & entry : fs::directory_iterator(fspath, fs::directory_options::skip_permission_denied))
            {
                auto entrypath = entry.path().string();
                body += "<a href=\"/browse" + entrypath + "\">" + entrypath + "</a><br>\n";
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
