#ifndef ASIOPULSE_H_
#define ASIOPULSE_H_

#include <memory>
#include <string>
#include <vector>

#include <boost/asio.hpp>

#include <pulse/mainloop.h>

#include "PulseContext.h"

namespace net = boost::asio;

class AsioPulse
{
	using MainloopPtr = std::unique_ptr<pa_mainloop, decltype(&pa_mainloop_free)>;

public:
	explicit AsioPulse(net::io_context& ioContext);
	~AsioPulse();

	PulseContextPtr createContext(const std::string& name);

private:
	void scheduleTimer();
	void iterate();

	void incrementInterval();
	void resetInterval();

	net::io_context& ioContext_;
	net::steady_timer timer_;

	MainloopPtr mainloop_;
	pa_mainloop_api* api_;

	std::size_t intervalMs_;
};

#endif // ASIOPULSE_H_
