#include <chrono>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>

#include <pulse/simple.h>
#include <pulse/error.h>

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

	int err = 0;

	std::unique_ptr<pa_simple, decltype(&pa_simple_free)> s(
		pa_simple_new(nullptr, "test output", PA_STREAM_PLAYBACK, nullptr,
			"simple tone output", &ss, nullptr, nullptr, &err), &pa_simple_free);

	if (!s || err > 0)
	{
		std::cerr << "Failed to create simple playback context: " << pa_strerror(err) << std::endl;
		return EXIT_FAILURE;
	}

	// generate 1s buffer of 2 tone data
	std::vector<uint16_t> data1;
	std::vector<uint16_t> data2;

	for (int i = 0; i < rate; ++i)
	{
		float t = i / static_cast<float>(rate);
		data1.emplace_back(static_cast<uint16_t>(amplitude * std::sin(freq1 * 2 * PI * t)));
		data2.emplace_back(static_cast<uint16_t>(amplitude * std::sin(freq2 * 2 * PI * t)));
	}

	bool first = true;
	int delayMs = 250;
	for (;;)
	{
		auto start = std::chrono::steady_clock::now();

		// switch between the 2 tones
		std::vector<uint16_t>& data = first ? data1 : data2;
		first = !first;


		pa_simple_write(s.get(), reinterpret_cast<uint8_t*>(data.data()), data.capacity() * sizeof(uint16_t), &err);
		if (err > 0)
		{
			std::cerr << "Failed to write: " << pa_strerror(err) << std::endl;
			break;
		}
		else
		{
			// Account for time taken to write data and the time taken to play the data
			// Assumption that it should never take longer than the duration to play

			int durationMs = data.capacity() / rate * 1000;

			auto finish = std::chrono::steady_clock::now();
			auto writeOverhead = std::chrono::duration_cast<std::chrono::milliseconds>(finish - start).count();
			std::this_thread::sleep_for(std::chrono::milliseconds(durationMs + delayMs - writeOverhead));
		}
	}
	
	return EXIT_SUCCESS;
}
