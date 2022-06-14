#include "socket.hpp"
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <thread>
#include <stdexcept>
#include <stdio.h>
#include <getopt.h>
#include <sys/wait.h>
#include <list>
#include <mutex>
#include <unistd.h>

#define DEFAULT_PORT (uint16_t)8787

struct aport
{
	const char *address;
	uint16_t port;
};

struct
{
	std::list<struct aport> addresses;
	const char *welcome_msg;
}
args;

class SocketListener
{
	public:

	SocketListener(Socket *sock)
	{
		this->s = sock;
		this->thread = std::thread(&SocketListener::listen, this);
	}

	~SocketListener(void)
	{
		SocketListener::thread.join();
		delete s;
	}

	protected:

	void listen(void)
	{
		int connfd;
		pid_t pid;

		while(1)
		{
			try
			{
				connfd = this->s->accept();
			}
			catch(std::runtime_error& x)
			{
				fprintf(stderr, "%s\n", x.what());
				continue;
			}

			pid = fork();
			if(pid == -1)
			{
				perror("Failed to create child process");
				continue;
			}

			if(pid == 0)
			{
				delete this->s;

				write(connfd, args.welcome_msg, strlen(args.welcome_msg));

				exit(0);
			}
			else
			{
				close(connfd);

				printf("Creating worker %d\n", pid);
			}
		}
	}

	private:

	std::thread thread;
	Socket *s;
};

std::list<SocketListener *> listeners;

static void args_parse(int argc, char *argv[])
{
	int option;
	int index;
	int should_parse = 1;

	struct aport tmp_aport = {nullptr, DEFAULT_PORT};
	args.addresses.push_back(tmp_aport);

	args.welcome_msg = "220 Good morning sir. Please kindly do the needful\n";

	struct option long_options[] =
	{
		{"help", no_argument, 0, 'h'},
		{"address", required_argument, 0, 'a'},
		{"welcome_msg", required_argument, 0, 'w'},
		{"p", required_argument, 0, 'p'},
		{0, 0, 0, 0},
	};

	while(should_parse)
	{
		option = getopt_long(argc, argv, "ha:p:w:", long_options, &index);

		switch(option)
		{
			case 0:
				break;

			case 'h':
				exit(EXIT_SUCCESS);

			case 'a':
				tmp_aport = {optarg, DEFAULT_PORT};
				args.addresses.push_back(tmp_aport);
				break;

			case 'p':
				args.addresses.back().port = atoi(optarg);
				break;

			case 'w':
				args.welcome_msg = optarg;
				break;

			case '?':
				fprintf(stderr, "Try '%s --help' for more information\n", argv[0]);
				exit(EXIT_FAILURE);

			default:
				should_parse = 0;
		}
	}
}

static void sigchild_handler(int signum)
{
	(void)signum;

	pid_t pid;
	int status;

	pid = wait(&status);
	if(pid == -1)
	{
		perror("wait failed");
		return;
	}

	if(WIFEXITED(status))
	{
		printf("Worker %d exited with exit code %d\n", pid, WEXITSTATUS(status));
	}
	else if(WIFSIGNALED(status))
	{
		fprintf(stderr, "Worker %d killed due to signal %d: %s\n",
				pid, WTERMSIG(status), strsignal(WTERMSIG(status)));
	}
	else
	{
		fprintf(stderr, "Unexpected exit status of worker %d\n", pid);
	}
}

int main(int argc, char *argv[])
{
	/* Setup signal handlers */
	signal(SIGCHLD, sigchild_handler);

	args_parse(argc, argv);

	if(args.addresses.size() < 1)
	{
		fprintf(stderr, "Invalid number of addresses\n");
		return -1;
	}

	auto i = args.addresses.begin();

	if(args.addresses.size() > 1)
		i++;

	for(; i != args.addresses.end(); i++)
	{
		Socket *s = new Socket(i->address, i->port);
		SocketListener *sl = new SocketListener(s);
		listeners.push_back(sl);
	}

	for(auto &li: listeners)
	{
		delete li;
	}

	return 0;
}
