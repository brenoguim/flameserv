#pragma once

#include <cassert>
#include <cstring>
#include <iostream>
#include <map>
#include <optional>
#include <stdexcept>
#include <vector>
#include <sstream>

struct HttpMessage
{
    std::map<std::string, std::vector<std::string>> m_attrs;
    std::string m_body;
};

struct Request : public HttpMessage
{
    std::string operation;
    std::string path;
};

struct Response : public HttpMessage
{
    std::string to_bytes() const;
};

std::string Response::to_bytes() const
{
    std::string r = "HTTP/1.1 200 OK\n";
    for (auto& [k, vs] : m_attrs) {
        r += k + ": ";
        bool first = true;
        for (auto& v : vs) {
            if (!first) r += ';';
            first = false;
            r += v;
        }
        r += '\n';
    }
    r += "Content-Length: " + std::to_string(m_body.size()) + '\n';

    r += '\n';
    r += m_body;
    return r;
}

inline Response make_html_response(std::string body)
{
    Response r;
    r.m_attrs["Content-Type"].push_back("text/html");
    r.m_attrs["Content-Type"].push_back("charset=UTF-8");
    r.m_attrs["Connection"].push_back("close");
    r.m_body = std::move(body);
    return r;
}


// GET / HTTP/1.1
// Host: 192.168.15.14:8081
// Connection: keep-alive
// Cache-Control: max-age=0
// DNT: 1
// Upgrade-Insecure-Requests: 1
// User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/87.0.4280.88 Safari/537.36
// Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.9
// Accept-Encoding: gzip, deflate
// Accept-Language: en-US,en;q=0.9,de;q=0.8,pt;q=0.7,es;q=0.6

inline Request parse_html_request(std::string_view str)
{
    std::istringstream ss{std::string(str)};

    Request r;
    ss >> r.operation;
    ss >> r.path;

    return r;
}
