#include "Socket.h"
#include "Http.h"
#include "ThreadPool.h"

#include <signal.h>

void sigintHandler(int sig_num) {}

int main()
{
    signal(SIGINT, sigintHandler);
    Socket s;
    ThreadPool thpool(4);
    while (auto opt_conn = s.waitConnection())
    {
        auto fn = [conn = std::move(*opt_conn)] {
            std::cout << "Got a connection" << std::endl;
            // handle connection
            char buffer[10*256];
            std::memset(buffer, 0, 10*256);
            auto n = read(conn.fd, buffer, 10*256-1);
            auto r = Response().to_bytes();;
            n = write(conn.fd, r.data(), r.size());
            std::cout << "Wrote back " << n << " bytes" << std::endl;
        };

        //thpool.add(std::move(fn));
    }
    std::cout << "Shutting down" << std::endl;
}
