#pragma once
#include <iostream>
#include <queue>
#include <string>
#include <sstream>      // stringstream
#include <thread>
#include <mutex>
#include <condition_variable>

#ifdef __linux__

#include <SDL.h>        // SDL_ShowSimpleMessageBox

#else
#include "windows.h"

#endif

class Log {
public:
	enum class Channel {
		MESSAGE_BOX,
		STD_OUT,
		SILENT
	};

	enum LogTime {
		LATER,
		FLUSH
	};

	~Log() {
		stop();
	}

	// stopEventProcessing logging thread
	static void stop() {
		Log &s_log = get();
		if (!s_log.m_stopped) {
			// send stopEventProcessing signal
			{
				std::lock_guard<std::mutex> l_lock(s_log.m_queueMutex);
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

	// log with custom log level
	template<LogTime time = LogTime::LATER, typename ...Args>
	static void log(Channel level, const Args &...args) {
		if (level == Channel::SILENT) {
			// do nothing
			return;
		}

		std::stringstream message;
		addToStream(message, args...);
		LogJob l_job(level, message.str());
//		if (level == Channel::MESSAGE_BOX) {
		if constexpr (time == LogTime::LATER) {
			// lazy initialization on first log use
			// TODO put into lessons
			// important to place the log static into get(), because the log function is templated
			// and thus generates a Log object for per call with different template types
			Log &instance = get();
			if(instance.m_do_stop == false) {
				instance.logLater(std::move(l_job));
			} else {
				instance.doLog(std::move(l_job));
			}
		} else {
			doLog(l_job);
		}
	}

	// log with default log level
	template<LogTime time = LogTime::LATER, typename ...Args>
	static void log(const Args &...args) {
		Channel default_level;

		// TODO only gets set once at first include because of include barrier
#ifdef DEFAULT_LOG_CHANNEL
		default_level = DEFAULT_LOG_CHANNEL;
#else
		default_level = Channel::SILENT;
#endif

		log<time>(default_level, args...);
	}
private:
	struct LogJob {
		LogJob()
				: m_channel(Channel::MESSAGE_BOX)
				, m_message() {
		}

		LogJob(Channel channel, std::string message)
				: m_channel(channel)
				, m_message(std::move(message)) {
		}

		Channel m_channel;
		std::string m_message;
	};

	bool m_stopped;
	std::condition_variable m_queueCondition;
	std::mutex m_queueMutex;
	bool m_do_stop;
	std::queue<LogJob> m_messageQueue;
	std::thread m_logThread;


	Log()
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
#endif
	}

	static Log &get() {
		static Log s_log;
		return s_log;
	}

	// from: https://stackoverflow.com/questions/9204780/accepting-variable-number-of-arguments-for-logger
	template<typename Stream, typename Arg1>
	static Stream &addToStream(Stream &stream, const Arg1 &arg1) {
		stream << arg1;
		return stream;
	}

	// from: https://stackoverflow.com/questions/9204780/accepting-variable-number-of-arguments-for-logger
	template<typename Stream, typename Arg1, typename ...Args>
	static Stream &addToStream(Stream &stream, const Arg1 &arg1, const Args &...args) {
		stream << arg1;
		return addToStream(stream, args...);
	}

	static void doLog(const LogJob &job) {
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
		default:
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
			default:
			case Channel::MESSAGE_BOX:
				MessageBox(nullptr, job.m_message.c_str(), "Log", MB_OK);
				break;
#endif
		}
	}

	void logLater(LogJob job) {
		// TODO throw exception or something when trying to log async whilst stopped
		{
			std::lock_guard<std::mutex> l_lock(m_queueMutex);
			m_messageQueue.push(std::move(job));
		}
		m_queueCondition.notify_all();
	}

	void logThreadWork() {
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
};