#include "listener.hpp"
#include <stdexcept>
#include <stdio.h>

int main()
{
	SocketListener *s;

	try
	{
		s = new SocketListener("::FFFF:1270.0.1", 8787);
	}
	catch(std::runtime_error& x)
	{
		printf("%s\n", x.what());
	}

	try
	{
		s->accept();
	}
	catch(std::runtime_error& x)
	{
		printf("%s\n", x.what());
	}

	delete s;
}
