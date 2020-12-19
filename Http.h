#pragma once

#include <cassert>
#include <cstring>
#include <iostream>
#include <map>
#include <optional>
#include <stdexcept>
#include <vector>

struct HttpMessage
{
    std::map<std::string, std::vector<std::string>> m_attrs;
    std::string m_body;
};

struct Request : public HttpMessage
{
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
