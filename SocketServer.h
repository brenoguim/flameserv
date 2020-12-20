#pragma once

#include "Socket.h"

#include <cassert>
#include <optional>
#include <stdexcept>

#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

struct SocketServer
{
    SocketServer()
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

    ~SocketServer()
    {
        ::close(sockfd);
    }

    std::optional<Socket> waitSocket()
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
        return Socket{newsockfd};
    }

    int sockfd;
};
