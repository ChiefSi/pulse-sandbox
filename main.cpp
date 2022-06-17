#include <chrono>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>

#include <boost/asio.hpp>

#include "AsioPulse.h"

namespace net = boost::asio;

static const float PI = std::acos(-1);

int main()
{
	uint16_t amplitude = 10000;
	float freq1 = 440.0;
	float freq2 = 880.0;
	uint32_t rate = 22050;

	pa_sample_spec ss;
	ss.format = PA_SAMPLE_S16LE;
	ss.channels = 1;
	ss.rate = rate;

	// generate 1s buffer of 2 tone data
	std::vector<uint16_t> data1;
	std::vector<uint16_t> data2;

	for (int i = 0; i < rate; ++i)
	{
		float t = i / static_cast<float>(rate);
		data1.emplace_back(static_cast<uint16_t>(amplitude * std::sin(freq1 * 2 * PI * t)));
		data2.emplace_back(static_cast<uint16_t>(amplitude * std::sin(freq2 * 2 * PI * t)));
	}

	net::io_context ioContext;

	AsioPulse asioPulse(ioContext);

	auto ctx = asioPulse.createContext("test");
	ctx->connect([](const std::error_code& ec)
		{
			if (ec)
			{
				std::cout << "Error: " << ec.message() << std::endl;
			}
			else
			{
				std::cout << "Connected" << std::endl;
			}
		});

	ioContext.run();
	
	return EXIT_SUCCESS;
}
