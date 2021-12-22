#pragma once

#include <queue>
#include <string>
#include <sstream>      // stringstream
#include <thread>
#include <mutex>
#include <condition_variable>

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

	~Log();

	// stopEventProcessing logging thread
	static auto stop() -> void;

	// log with custom log level
	template<LogTime time = LogTime::LATER, typename ...Args>
	static auto log(Channel level, const Args &...args) -> void {
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
			if (instance.m_do_stop == false) {
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
	static auto log(const Args &...args) -> void {
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

	Log();

	static auto get() -> Log&;

	// TODO: Use fold expressions instead of recursion
	// from: https://stackoverflow.com/questions/9204780/accepting-variable-number-of-arguments-for-logger
	template<typename Stream, typename Arg1>
	static auto addToStream(Stream &stream, const Arg1 &arg1) -> Stream& {
		stream << arg1;
		return stream;
	}

	// from: https://stackoverflow.com/questions/9204780/accepting-variable-number-of-arguments-for-logger
	template<typename Stream, typename Arg1, typename ...Args>
	static auto addToStream(Stream &stream, const Arg1 &arg1, const Args &...args) -> Stream& {
		stream << arg1;
		return addToStream(stream, args...);
	}

	static auto doLog(const LogJob &job) -> void;

	auto logLater(LogJob job) -> void;

	auto logThreadWork() -> void;

	auto joinWorkerThread() -> void;
};
