#include <gtest/gtest.h>
#include <stdexcept>

#include "../src/socket.hpp"

/**
 * @brief Test if server accepts both IPv4 and IPv6 addresses
 *
 */
TEST(server, Socket)
{
	struct test
	{
		const char *address;
		const bool result;
		const bool does_throw;
	};

	/*  */
	const struct test test_addresses[]
	{
		{"127.0.0.1", true, false},
		{"::1", true, false},
		{"::FFFF:127.0.0.1", true, false},
		{nullptr, true, false},

		{":FFFF:127.0.0.1", false, true},
		{"127.0.0.1,", false, true},
		{"1270.0.1", false, true},
	};

	size_t len = sizeof(test_addresses)/sizeof(*test_addresses);

	/* Valid addresses */
	for(size_t i = 0; i < len; i++)
	{
		const struct test *t = &test_addresses[i];
		Socket *sl = nullptr;

		if(t->does_throw)
		{
			EXPECT_THROW(sl = new Socket(t->address, 8787), std::runtime_error);
		}
		else
		{
			EXPECT_NO_THROW(sl = new Socket(t->address, 8787));
		}

		EXPECT_EQ((bool)sl, t->result);

		if(sl != nullptr)
			delete sl;
	}
}
