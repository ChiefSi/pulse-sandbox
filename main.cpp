#include <array>
#include <chrono>
#include <cmath>
#include <complex>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>

#include <boost/asio.hpp>

#include "AsioPulse.h"

#include <fftw3.h>

namespace net = boost::asio;

static const double PI = std::acos(-1);

// Fixed window size - informs the frequency bins
static const std::size_t N = 2048;

// Window function to remove FT artefacts (the tail-off from the peak of real frequencies)
// Note if interested in peaks only this is likely not necessary
std::array<double, N> HanningWindow()
{
	std::array<double, N> h;
	for (std::size_t i = 0; i < N; ++i)
	{
		h[i] = 0.5 * (1 - std::cos(2.0 * PI * i / N));
	}
	return h;
}

// Empty window for quick demo of different windowing
std::array<double, N> BoxWindow()
{
	std::array<double, N> box;
	for (std::size_t i = 0; i < N; ++i)
	{
		box[i] = 1.0;
	}

	return box;
}

//static std::array<double, N> WINDOW = HanningWindow();
static std::array<double, N> WINDOW = BoxWindow();

template <typename T>
std::array<double, N> Window(const T& t)
{
	std::array<double, N> data;
	for (std::size_t i = 0; i < N; ++i)
	{
		data[i] = t[i] * WINDOW[i];
	}

	return data;
}

int main()
{
	std::cout << N << std::endl;
	uint16_t amp = 10000;
	float freq = 440.0;
	uint32_t rate = 22050;

	// Real data would likely need a sliding window, main challenge becomes how
	// to combine and perform overall analysis

	// Representation of initial data (uint16_t) wrap-around introduces
	// artefacts... would need to normalize before input to dft
	std::vector<double> data;

	for (int i = 0; i < N; ++i)
	{
		double t = i / static_cast<double>(rate);
		data.emplace_back(amp * std::sin(freq * 2 * PI * t));
	}

	{
		std::ofstream out("pcm.data", std::ios::binary);
		for (auto& i : data) out << i << '\n';
	}

	auto input = Window(data);
	{
		std::ofstream out("window.data", std::ios::binary);
		out << std::fixed << std::setprecision(8);
		for (auto& i : input) out << i << '\n';
	}

	std::array<std::complex<double>, N> output;

	fftw_plan plan = fftw_plan_dft_r2c_1d(N, input.data(), reinterpret_cast<fftw_complex*>(output.data()), FFTW_ESTIMATE);
	fftw_execute(plan);
	fftw_cleanup();

	// Half the output will be empty (due to real input data)

	{
		std::ofstream out("ft.data", std::ios::binary);
		out << std::fixed << std::setprecision(8);

		std::array<double, N> amplitude;
		for (std::size_t i = 0; i < N; ++i)
		{
			double bin = i * rate / N;
			amplitude[i] = std::sqrt((output[i].real() * output[i].real()) + (output[i].imag() * output[i].imag()));
			out << bin << "," << amplitude[i] << '\n';
		}

	}

	/*
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
	*/
	return EXIT_SUCCESS;
}
