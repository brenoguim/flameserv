#pragma once

#include <cassert>
#include <cstring>
#include <iostream>
#include <map>
#include <optional>
#include <stdexcept>
#include <vector>

char fake[] = R"( 
<html>
  <head>
    <title>An Example Page</title>
  </head>
  <body>
    <p>Hello World, this is a very simple HTML document. Breno</p>
  </body>
</html>
)";


struct Response
{
    Response() {
        m_attrs["Content-Type"].push_back("text/html");
        m_attrs["Content-Type"].push_back("charset=UTF-8");
        m_attrs["Connection"].push_back("close");
        m_body = fake;
    }
    std::string to_bytes() const
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
    std::map<std::string, std::vector<std::string>> m_attrs;
    std::string m_body;
};
