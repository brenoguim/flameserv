#include "Socket.h"
#include "ThreadPool.h"
#include "FlameServer.h"

#include <signal.h>

void sigintHandler(int sig_num) {}

int main()
{
    signal(SIGINT, sigintHandler);
    Socket s;
    ThreadPool thpool(4);
    while (auto opt_conn = s.waitConnection())
    {
        thpool.add(FlameServer{std::move(*opt_conn)});
    }
    std::cout << "Shutting down" << std::endl;
}
