#ifndef __linux__
// windows includes
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#include <Windows.h>
#endif

#include <iostream>

#include <SDL.h>        // SDL_ShowSimpleMessageBox

#include "Log.hpp"

Log::~Log() {
	stop();
}

void Log::stop() {
	Log &s_log = get();
	if (!s_log.m_stopped) {
		// send stopEventProcessing signal
		{
			std::scoped_lock l_lock(s_log.m_queueMutex);
			s_log.m_do_stop = true;
		}
		s_log.m_queueCondition.notify_all();

		// wait for log thread termination
		s_log.m_logThread.join();
		s_log.m_stopped = true;
#ifdef __linux__
		// TODO: We're not a reusable lib, if we don't deinitialize
//			SDL_Quit();
#endif
	}
}

Log::Log()
		: m_stopped(false)
		, m_queueCondition()
		, m_queueMutex()
		, m_do_stop(false)
		, m_messageQueue()
		, m_logThread(&Log::logThreadWork, this) {
#ifdef __linux__
	pthread_setname_np(m_logThread.native_handle(), "LOG");
	if (SDL_Init(0) < 0) {
		// TODO: what to do when SDL init fails
		fprintf(stderr, "Couldn't initialize SDL: %s\n", SDL_GetError());
	}
#else
	SetThreadDescription(m_logThread.native_handle(), L"LOG");
#endif
}

Log &Log::get() {
	static Log s_log;
	return s_log;
}

void Log::doLog(const LogJob &job) {
	switch (job.m_channel) {
		case Channel::SILENT:
			break;
#ifdef __linux__
		case Channel::STD_OUT:
			// TODO(nix): find out why cout crashes
			// no std::cout yet
			printf("%s\n", job.m_message.c_str());
			fflush(stdout);
			break;
		case Channel::MESSAGE_BOX:
			SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION,
			                         "Log",
			                         job.m_message.c_str(),
			                         nullptr);
			break;
#else
		case Channel::STD_OUT:
			std::cout << job.m_message << std::endl;
			break;
		case Channel::MESSAGE_BOX:
			MessageBox(nullptr, job.m_message.c_str(), "Log", MB_OK);
			break;
#endif
		default: {
			std::string l_error("Error: Log called with unimplemented channel");
#ifdef __linux__
			SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION,
			                         "Log",
			                         l_error.c_str(),
			                         nullptr);
#else
			MessageBox(nullptr, l_error.c_str(), "Log", MB_OK);
#endif
			throw std::domain_error(l_error);
		}
	}
}

void Log::logLater(LogJob job) {
	// TODO throw exception or something when trying to log async whilst stopped
	{
		std::scoped_lock l_lock(m_queueMutex);
		m_messageQueue.push(std::move(job));
	}
	m_queueCondition.notify_all();
}

void Log::logThreadWork() {
	while (true) {
		LogJob l_job;
		// retrieve logJob
		{
			std::unique_lock<std::mutex> l_lock(m_queueMutex);

			while (!m_do_stop && m_messageQueue.empty()) {
				m_queueCondition.wait(l_lock);
			}

			if (m_do_stop && m_messageQueue.empty()) {
				// exit thread
				break;
			}

			l_job = std::move(m_messageQueue.front());
			m_messageQueue.pop();
		}

		doLog(l_job);
	}
}
