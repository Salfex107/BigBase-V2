#pragma once
#include "common.hpp"
#include <g3log/g3log.hpp>
#include <g3log/logworker.hpp>

namespace big
{
	template <typename TP>
	std::time_t to_time_t(TP tp)
	{
		using namespace std::chrono;
		auto sctp = time_point_cast<system_clock::duration>(tp - TP::clock::now()
			+ system_clock::now());
		return system_clock::to_time_t(sctp);
	}

	enum class log_color : std::uint16_t
	{
		Black = 0,
		DarkBlue = FOREGROUND_BLUE,
		DarkGreen = FOREGROUND_GREEN,
		DarkCyan = FOREGROUND_GREEN | FOREGROUND_BLUE,
		DarkRed = FOREGROUND_RED,
		DarkMagenta = FOREGROUND_RED | FOREGROUND_BLUE,
		DarkYellow = FOREGROUND_RED | FOREGROUND_GREEN,
		DarkGray = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,
		Gray = FOREGROUND_INTENSITY,
		Blue = FOREGROUND_INTENSITY | FOREGROUND_BLUE,
		Green = FOREGROUND_INTENSITY | FOREGROUND_GREEN,
		Cyan = FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_BLUE,
		Red = FOREGROUND_INTENSITY | FOREGROUND_RED,
		Magenta = FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_BLUE,
		Yellow = FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN,
		White = FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,
		LightGray = FOREGROUND_INTENSITY | FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED,
		Orange = FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN,
		Pink = FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY,
		LightBlue = FOREGROUND_INTENSITY | FOREGROUND_BLUE,
		LightGreen = FOREGROUND_INTENSITY | FOREGROUND_GREEN,
		LightCyan = FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_BLUE,
		LightRed = FOREGROUND_INTENSITY | FOREGROUND_RED,
		LightMagenta = FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_BLUE,
		LightYellow = FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY
	};

	enum LOG_FLAGS
	{
		FLAG_NO_DISK = (1 << 0),
		FLAG_NO_CONSOLE = (1 << 1)
	};

	static const int kEventValue = 400;
	static const int kRawValue =  600;
	inline constexpr auto max_padding_length = 26;
	inline constexpr auto level_padding_length = 8;

	const LEVELS INFO_TO_FILE{ INFO.value | FLAG_NO_CONSOLE, {"INFO"} },
		HACKER{ INFO.value, {"HACKER"} },
		EVENT{ kEventValue | FLAG_NO_CONSOLE, {"EVENT"} },
		RAW{ kRawValue | FLAG_NO_DISK, {"RAW_GREEN_TO_CONSOLE"} },
		RAW_RED{ kRawValue, {"RAW_RED"} };

	inline log_color operator|(log_color a, log_color b)
	{
		return static_cast<log_color>(static_cast<std::underlying_type_t<log_color>>(a) | static_cast<std::underlying_type_t<log_color>>(b));
	}

	class logger;
	inline logger* g_logger{};

	class logger
	{
	public:
		explicit logger() :
			m_file_path(std::getenv("appdata")),
			m_worker(g3::LogWorker::createLogWorker())
		{
			if ((m_did_console_exist = AttachConsole(GetCurrentProcessId())) == false)
				AllocConsole();

			if ((m_console_handle = GetStdHandle(STD_OUTPUT_HANDLE)) != nullptr)
			{
				SetConsoleTitleA("BigBaseV2");
				SetConsoleOutputCP(CP_UTF8);

				m_console_out.open("CONOUT$", std::ios_base::out | std::ios_base::app);
			}

			m_file_path /= "BigBaseV2";
			std::filesystem::path m_backup_path = m_file_path;
			m_backup_path /= "Backup";
			try
			{
				if (!std::filesystem::exists(m_file_path))
				{
					std::filesystem::create_directory(m_file_path);
				}
				else if (!std::filesystem::is_directory(m_file_path))
				{
					std::filesystem::remove(m_file_path);
					std::filesystem::create_directory(m_file_path);
				}
				if (!std::filesystem::exists(m_backup_path))
				{
					std::filesystem::create_directory(m_backup_path);
				}
				else if (!std::filesystem::is_directory(m_backup_path))
				{
					std::filesystem::remove(m_backup_path);
					std::filesystem::create_directory(m_backup_path);
				}

				m_event_file_path = m_file_path;
				m_file_path /= "BigBaseV2.log";
				m_event_file_path /= "GTAEvents.log";

				if (std::filesystem::exists(m_file_path))
				{
					auto file_time = std::filesystem::last_write_time(m_file_path);
					auto timet = to_time_t(file_time);
					auto local_time = std::localtime(&timet);

					auto bigbase_timestamp = fmt::format("{:0>2}-{:0>2}-{}-{:0>2}-{:0>2}-{:0>2} BigBaseV2.log", local_time->tm_mon + 1, local_time->tm_mday, local_time->tm_year + 1900, local_time->tm_hour, local_time->tm_min, local_time->tm_sec);
					auto gta_events_timestamp = fmt::format("{:0>2}-{:0>2}-{}-{:0>2}-{:0>2}-{:0>2} GTAEvents.log", local_time->tm_mon + 1, local_time->tm_mday, local_time->tm_year + 1900, local_time->tm_hour, local_time->tm_min, local_time->tm_sec);

					std::filesystem::copy_file(m_file_path, m_backup_path / bigbase_timestamp);
					if (std::filesystem::exists(m_event_file_path) && !std::filesystem::is_empty(m_event_file_path))
						std::filesystem::copy_file(m_event_file_path, m_backup_path / gta_events_timestamp);
				}

				m_file_out.open(m_file_path, std::ios_base::out | std::ios_base::trunc);
				m_gta_event_file_out.open(m_event_file_path, std::ios_base::out | std::ios_base::trunc);

				m_worker->addSink(std::make_unique<log_sink>(), &log_sink::callback);
				g3::initializeLogging(m_worker.get());
			}
			catch (std::filesystem::filesystem_error const& error)
			{
				m_console_out << error.what();
			}

			g_logger = this;
		}

		~logger()
		{
			m_worker.reset();
			if (!m_did_console_exist)
				FreeConsole();

			g_logger = nullptr;
		}

		struct log_sink
		{
			std::map<std::string, log_color> log_colors = {
				{INFO.text, log_color::White},
				{WARNING.text, log_color::Red},
				{HACKER.text, log_color::Green},
				{FATAL.text, log_color::Red},
				{G3LOG_DEBUG.text, log_color::Blue},
				{RAW_RED.text, log_color::Red},
				{RAW.text, log_color::Cyan}
			};

			void callback(g3::LogMessageMover log)
			{
				g3::LogMessage log_message = log.get();
				int level_value = log_message._level.value;

				bool is_raw = level_value == RAW_RED.value || level_value == RAW.value;

				if (!(level_value & FLAG_NO_CONSOLE))
				{
					SetConsoleTextAttribute(g_logger->m_console_handle, static_cast<std::uint16_t>(log_colors[log_message._level.text]));
					g_logger->m_console_out << log_message.toString(is_raw ? format_raw : format_file) << std::flush;
				}
				
				if (!(level_value & FLAG_NO_DISK))
				{
					if (level_value == EVENT.value)
						g_logger->m_gta_event_file_out << log_message.toString(format_file) << std::flush;
					else
						g_logger->m_file_out << log_message.toString(is_raw ? format_raw : format_file) << std::flush;
				}
			}

			static std::string format_file(const g3::LogMessage& msg)
			{
				std::string file_name_with_line = "[" + msg.file() + ":" + msg.line() + "]";
				std::stringstream out;
				out << "[" << msg.timestamp("%H:%M:%S") << "] [" << std::left << std::setw(level_padding_length) << msg.level().append("]") << std::setw(max_padding_length) << file_name_with_line;
				return out.str();
			}

			static std::string format_console(const g3::LogMessage& msg)
			{
				std::stringstream out;
				out << "[" << msg.timestamp("%H:%M:%S") << "] ";
				return out.str();
			}

			static std::string format_raw(const g3::LogMessage& msg)
			{
				return "";
			}
		};

		template <typename ...Args>
		inline void log_now(std::string_view format, Args const& ...args)
		{
			auto message = fmt::format(format, args...);
			if (m_file_out)
				m_file_out << message << std::endl << std::flush;
		}
		
	private:
		bool m_did_console_exist{};
		HANDLE m_console_handle{};
		std::ofstream m_console_out;
		std::filesystem::path m_file_path;
		std::filesystem::path m_event_file_path;
		std::ofstream m_file_out;
		std::ofstream m_gta_event_file_out;
		std::unique_ptr<g3::LogWorker> m_worker;
	};


#define LOG_NOW(format, ...) g_logger->log_now(format, __VA_ARGS__)
#define HEX_TO_UPPER(value) "0x" << std::hex << std::uppercase << (DWORD64)value << std::dec << std::nouppercase
}
