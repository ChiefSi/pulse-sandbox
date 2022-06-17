#include "PulseContext.h"

#include <iostream>

#include <pulse/error.h>

PulseContext::PulseContext(net::io_context& ioContext, pa_mainloop_api* api, const std::string& name)
	: strand_(ioContext)
	, ctx_(pa_context_new(api, name.c_str()))
	, connectCallback_(nullptr)
	, disconnectCallback_(nullptr)
{
	pa_context_set_event_callback(ctx_, &PulseContext::EventCallback, this);
	pa_context_set_state_callback(ctx_, &PulseContext::StateCallback, this);
}

void PulseContext::connect(CallbackFn&& callback)
{
	// Check state and only attempt connect if unconnected
	if (pa_context_connect(ctx_, nullptr, PA_CONTEXT_NOFLAGS, nullptr) < 0)
	{
		std::cerr << "Failed to connect: " << pa_strerror(pa_context_errno(ctx_)) << std::endl;
		strand_.post([cb = std::forward<CallbackFn>(callback)](){ cb(std::make_error_code(std::errc::connection_aborted)); });
	}
	else
	{
		if (connectCallback_)
		{
			strand_.post([cb = connectCallback_](){ cb(std::make_error_code(std::errc::connection_aborted)); });
		}

		connectCallback_ = callback;
	}
}

void PulseContext::disconnect(CallbackFn&& callback)
{
	pa_context_disconnect(ctx_);
}

void PulseContext::EventCallback(pa_context* ctx, const char* name, pa_proplist* proplist, void* userdata)
{
	auto me = reinterpret_cast<PulseContext*>(userdata);
	me->eventCallback(name, proplist);
}

void PulseContext::StateCallback(pa_context* ctx, void* userdata)
{
	auto me = reinterpret_cast<PulseContext*>(userdata);
	me->stateCallback();
}

void PulseContext::eventCallback(const char* name, pa_proplist* proplist)
{
	// TODO
}

void PulseContext::stateCallback()
{
	switch (pa_context_get_state(ctx_))
	{
		case PA_CONTEXT_UNCONNECTED: std::cout << "PA_CONTEXT_UNCONNECTED" << std::endl; break;
		case PA_CONTEXT_CONNECTING: std::cout << "PA_CONTEXT_CONNECTING" << std::endl; break;
		case PA_CONTEXT_AUTHORIZING: std::cout << "PA_CONTEXT_AUTHORIZING" << std::endl; break;
		case PA_CONTEXT_SETTING_NAME: std::cout << "PA_CONTEXT_SETTING_NAME" << std::endl; break;
		case PA_CONTEXT_READY:
		{
			std::cout << "PA_CONTEXT_READY" << std::endl;
			if (connectCallback_)
			{
				strand_.post([cb = connectCallback_]()
					{
						std::error_code ec;
						cb(ec);
					});
				connectCallback_ = nullptr;
			}
			break;
		}
		case PA_CONTEXT_FAILED:
		{
			std::cout << "PA_CONTEXT_FAILED" << std::endl;
			if (connectCallback_)
			{
				strand_.post([cb = connectCallback_]()
					{
						cb(std::make_error_code(std::errc::connection_aborted));
					});
				connectCallback_ = nullptr;
			}
			break;
		}

		case PA_CONTEXT_TERMINATED:
		{
			std::cout << "PA_CONTEXT_TERMINATED" << std::endl;
			if (connectCallback_)
			{
				strand_.post([cb = connectCallback_]()
					{
						cb(std::make_error_code(std::errc::connection_aborted));
					});
				connectCallback_ = nullptr;
			}
			break;
		}

		default: std::cout << "Unknown" << std::endl; break;
	}
}

