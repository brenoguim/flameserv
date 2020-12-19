#include <optional>
#include <stdexcept>
#include <iostream>
#include <cassert>
#include <cstring>
#include <map>
#include <vector>

#include <netinet/in.h>
#include <signal.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

struct Connection
{
    explicit Connection(int pfd) : fd(pfd) {}

    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;

    Connection(Connection&& other) : fd(std::exchange(other.fd, -1)) {}
    Connection& operator=(Connection&& other) { std::swap(fd, other.fd); return *this; }

    ~Connection()
    {
        if (fd != -1) close(fd);
    }
    int fd {-1};
};

struct Socket
{
    Socket()
    {
        sockfd = ::socket(/*domain*/AF_INET, /*type*/SOCK_STREAM, /*protocol*/0);
        if (sockfd < 0)
            throw std::runtime_error("Failure to create socket");

        sockaddr_in serv_addr {};
        int portno = 8081;
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = INADDR_ANY;
        serv_addr.sin_port = htons(portno);
        if (auto err = bind(sockfd, (const sockaddr*) &serv_addr, sizeof(serv_addr)); err < 0) 
            throw std::runtime_error("Failure to bind socket: " + std::to_string(err));

        if (::listen(sockfd, 5) < 0)
            throw std::runtime_error("Error on listen");
    }

    ~Socket()
    {
        ::close(sockfd);
    }

    std::optional<Connection> waitConnection()
    {
        // wait until there is a connection
        timeval timeout = {20000, 0};

        fd_set read_fd_set;
        FD_ZERO(&read_fd_set);
        FD_SET(sockfd, &read_fd_set);

        int r = select(sockfd + 1, &read_fd_set, nullptr, nullptr, &timeout);
        if (r == 0)
            return {};
        if (r < 0)
            return {};
        assert(r == 1);

        sockaddr_in cli_addr {};
        socklen_t clilen = sizeof(cli_addr);
        auto newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0) 
            return {};
        return Connection{newsockfd};
    }

    int sockfd;
};

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

void sigintHandler(int sig_num) {}

int main()
{
    signal(SIGINT, sigintHandler);
    Socket s;
    while (auto opt_conn = s.waitConnection())
    {
        std::cout << "Got a connection" << std::endl;
        // handle connection
        auto& conn = *opt_conn; 
        char buffer[10*256];
        std::memset(buffer, 0, 10*256);
        auto n = read(conn.fd, buffer, 10*256-1);
        auto r = Response().to_bytes();;
        n = write(conn.fd, r.data(), r.size());
        std::cout << "Wrote back " << n << " bytes" << std::endl;
    }
    std::cout << "Shutting down" << std::endl;
}
