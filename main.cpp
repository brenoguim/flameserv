#include "Socket.h"
#include "Http.h"

#include <signal.h>

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
