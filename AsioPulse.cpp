#include "AsioPulse.h"

#include <iostream>

AsioPulse::AsioPulse(net::io_context& ioContext)
	: ioContext_(ioContext)
	, timer_(ioContext)
	, mainloop_(pa_mainloop_new(), &pa_mainloop_free)
	, api_(pa_mainloop_get_api(mainloop_.get()))
	, intervalMs_(0)
{
	scheduleTimer();
}

AsioPulse::~AsioPulse()
{
	boost::system::error_code ec;
	timer_.cancel(ec);

	pa_mainloop_quit(mainloop_.get(), 0);
}

PulseContextPtr AsioPulse::createContext(const std::string& name)
{
	return std::make_shared<PulseContext>(ioContext_, api_, name);
}

void AsioPulse::scheduleTimer()
{
	timer_.expires_after(std::chrono::milliseconds(intervalMs_));
	timer_.async_wait([&](const std::error_code& ec){ if (!ec) iterate(); });
}

void AsioPulse::iterate()
{
	pa_mainloop_prepare(mainloop_.get(), 0);
	pa_mainloop_poll(mainloop_.get());
	if (pa_mainloop_dispatch(mainloop_.get()) == 0)
	{
		incrementInterval();
	}
	else
	{
		resetInterval();
	}

	scheduleTimer();
}

void AsioPulse::incrementInterval()
{
	if (intervalMs_ < 50)
	{
		intervalMs_ += 10;
	}
}

void AsioPulse::resetInterval()
{
	intervalMs_ = 0;
}
