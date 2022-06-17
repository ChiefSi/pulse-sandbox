#ifndef PULSECONTEXT_H_
#define PULSECONTEXT_H_

#include <boost/asio.hpp>

#include <pulse/context.h>
#include <pulse/mainloop.h>

namespace net = boost::asio;

class AsioPulse;

class PulseContext;
using PulseContextPtr = std::shared_ptr<PulseContext>;

class PulseContext
{
	using CallbackFn = std::function<void(const std::error_code&)>;

public:
	void connect(CallbackFn&& callback);
	void disconnect(CallbackFn&& callback);
	PulseContext(net::io_context& ioContext, pa_mainloop_api* api, const std::string& name);

private:
	static void EventCallback(pa_context* ctx, const char* name, pa_proplist* proplist, void* userdata);
	static void StateCallback(pa_context* ctx, void* userdata);

	void eventCallback(const char* name, pa_proplist* proplist);
	void stateCallback();

	net::io_context::strand strand_;
	pa_context* ctx_;
	CallbackFn connectCallback_ = nullptr;
	CallbackFn disconnectCallback_ = nullptr;
};

#endif // PULSECONTEXT_H_
