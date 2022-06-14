#pragma once

#include <stdexcept>
#include <string.h>
#include <string>
#include <unistd.h>
#include <stdint.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

class Socket
{
	public:

	/**
	 * @brief Initialize Socket
	 *
	 * @param[in]	address		IPv4, IPv6 or hostname. If NULL will listen on all addresses
	 * @param[in]	port		Port number (0 - 65535)
	 *
	 * @throws	std::runtime_error
	 */
	Socket(const char *address, in_port_t port)
	{
		int v = 1;
		int sockopt_val;
		struct in6_addr bin_addr = in6addr_any;
		struct sockaddr_in6 srv_addr;

		if(address && (v = inet_pton(AF_INET6, address, &bin_addr)) == 0)
		{
			v = inet_pton(AF_INET6, ("::FFFF:" + std::string(address)).c_str(), &bin_addr);
			if(v == 0)
			{
				throw std::runtime_error("Invalid address");
			}
		}

		if(v == -1)
		{
			throw std::runtime_error("Failed to convert address to binary form: " + std::string(strerror(errno)));
		}

		this->sockfd = socket(AF_INET6, SOCK_STREAM, 0);
		if(this->sockfd == -1)
		{
			throw std::runtime_error("Failed to create socket: " + std::string(strerror(errno)));
		}

		memset(&srv_addr, 0, sizeof(srv_addr));

		srv_addr.sin6_family = AF_INET6;
		srv_addr.sin6_addr = bin_addr;
		srv_addr.sin6_port = htons(port);

		/* Allow IPv4 on IPv6 */
		sockopt_val = 0;
		if(setsockopt(this->sockfd, IPPROTO_IPV6, IPV6_V6ONLY, &sockopt_val, sizeof(sockopt_val)))
		{
			throw std::runtime_error("Failed to set sockopt IPV6_V6ONLY: " + std::string(strerror(errno)));
		}

		/* Allow reusing socket immediately after exit */
		sockopt_val = 1;
		if(setsockopt(this->sockfd, SOL_SOCKET, SO_REUSEADDR, &sockopt_val, sizeof(sockopt_val)))
		{
			throw std::runtime_error("Failed to set sockopt SO_REUSEADDR: " + std::string(strerror(errno)));
		}

		if(bind(this->sockfd, (struct sockaddr *)&srv_addr, sizeof(srv_addr)))
		{
			close(this->sockfd);
			throw std::runtime_error("Failed to bind socket: " + std::string(strerror(errno)));
		}

		if(listen(this->sockfd, 5))
		{
			close(this->sockfd);
			throw std::runtime_error("Failed to listen on bound socket: " + std::string(strerror(errno)));
		}
	}

	~Socket(void)
	{
		close(sockfd);
	}

	int accept(void)
	{
		struct sockaddr_in6 addr;
		socklen_t addr_len = sizeof(addr);
		int ret = ::accept(this->sockfd, (struct sockaddr *)&addr, &addr_len);
		if(ret == -1)
		{
			throw std::runtime_error("Failed to accept incoming connection: " + std::string(strerror(errno)));
		}

		return ret;
	}

	private:

	int sockfd;
};
