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
    std::string to_bytes() const;
    std::string httpversion;
    std::map<std::string, std::vector<std::string>> m_attrs;
    std::string m_body;
};

struct Request : public HttpMessage
{
    std::string operation;
    std::string path;
    bool isPost() const { return operation == "POST"; }
    bool isGet() const { return operation == "GET"; }
};

struct Response : public HttpMessage
{
};

std::string HttpMessage::to_bytes() const
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

inline Response make_text_response(std::string body, std::string_view type = "plain")
{
    Response r;
    r.m_attrs["Content-Type"].push_back("text/" + std::string(type));
    r.m_attrs["Content-Type"].push_back("charset=UTF-8");
    r.m_attrs["Connection"].push_back("close");
    r.m_body = std::move(body);
    return r;
}

inline Response make_html_response(std::string body) { return make_text_response(std::move(body), "html"); }


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

inline Request parse_html_request(const Socket& s)
{
    std::string buf = read(s);
    std::string_view str = buf;
    auto by_line = [&str] () mutable -> std::optional<std::string_view> {
        if (str.size() == 0)
            return {};
        auto eol = str.find('\r');
        if (eol == std::string::npos)
            eol = str.size();
        auto ret = str.substr(0, eol);
        str.remove_prefix(eol + 2);
        return ret;
    };

    Request r;
    bool header = true;
    while (auto opt_line = by_line())
    {
        auto& line = *opt_line;
        if (header)
        {
            std::istringstream ssh{std::string(line)};
            ssh >> r.operation;
            ssh >> r.path;
            ssh >> r.httpversion;
            header = false;
        }
        else
        {
            auto separator = line.find(':');
            if (separator == std::string::npos)
                break;
            auto key = line.substr(0, separator);
            auto value = line.substr(separator+2);
            r.m_attrs[std::string(key)].emplace_back(value);
        }
    }
    r.m_body = str;
    if (r.m_attrs.count("Content-Length"))
    {
        std::size_t len = std::atol(r.m_attrs["Content-Length"].front().c_str());
        while (r.m_body.size() != len)
        {
            r.m_body += read(s);
        }
    }

    return r;
}
