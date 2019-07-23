#include "execution_artifacts_output.hpp"
#include <recent_apps.hpp>
#include <shim_cache.hpp>
#include <usn_journal.hpp>
#include <app_compat_flags.hpp>
#include <mui_cache.hpp>
#include <user_assist.hpp>
#include <sid.hpp>

#include <vector>
#include <sstream>
#include <ctime>
#include <thread>
#include <chrono>

namespace ea {

	namespace detail {

		inline std::string format_time(int64_t time) {
			/* y/m/d */
			std::stringstream ss;
			if (time / 3600 > 24) {
				ss << 1970 + time / 31557600 << '/';
				time = time % 31557600;
				ss << 1 + time / 2629800 << '/';
				time = time % 2629800;
				ss << time / 86400 << '-';
				time = time % 86400;
			}
			/* h:m:s */
			ss << time / 3600 << ':';
			time = time % 3600;
			ss << time / 60 << ':';
			time = time % 60;
			ss << time;
			return ss.str();
		}

		inline int64_t filetime_to_unix_timestamp(int64_t filetime) {
			return filetime / 10000000.0 - 11644473600.0;
		}

		inline int64_t filetime_to_unix_timestamp(FILETIME filetime) {
			return filetime_to_unix_timestamp(*(int64_t*)&filetime);
		}

	} // namespace detail

	std::string get_app_compat_flags_info()
	{
		std::stringstream ss;
		auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		ss << "Dumping AppCompatFlags, Timestamp: " << std::ctime(&now) << std::endl;

		ea::enum_app_compat_flag_paths([&](std::wstring_view path) {
			ss << std::string(path.begin(), path.end()) << std::endl;
		});

		return ss.str();
	}

	std::string get_mui_cache_info()
	{
		wchar_t buffer[256];
		auto    end = buffer;
		ea::acquire_and_format_sid(end);
		std::wstring_view sid(buffer, end - buffer);

		std::stringstream ss;
		auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		ss << "Dumping MUI Cache, Timestamp: " << std::ctime(&now) << std::endl;

		ea::enum_mui_cache(sid, [&](ea::mui_cache_entry&& entry) {
			ss <<
				"[Path = " << std::string(entry.path.begin(), entry.path.end()) <<
				"] [Description = " << std::string(entry.description.begin(), entry.description.end()) << "]" << std::endl;
		});

		return ss.str();
	}

	std::string get_shim_cache_info()
	{
		std::stringstream ss;
		auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		ss << "Dumping ShimCache, Timestamp: " << std::ctime(&now) << std::endl;

		ea::enum_shim_cache([&](ea::shim_entry_t& entry) {
			auto last_modification_time = detail::filetime_to_unix_timestamp(entry.last_modification_time);
			ss <<
				"Time: [" << (entry.last_modification_time.dwLowDateTime ? detail::format_time(last_modification_time) : "0") <<
				"], " << std::string(entry.path.begin(), entry.path.end()) << std::endl;
		});

		return ss.str();
	}

	std::string get_recent_apps_info()
	{
		wchar_t buffer[256];
		auto    end = buffer;
		ea::acquire_and_format_sid(end);
		std::wstring_view sid(buffer, end - buffer);

		std::stringstream ss;
		auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		ss << "Dumping RecentApps, Timestamp: " << std::ctime(&now) << std::endl;

		ea::enum_recent_apps(sid, [&](ea::recent_app& entry) {
			auto last_access = detail::filetime_to_unix_timestamp(entry.last_access());
			ss <<
				"Launch Count: [" << entry.launch_count() <<
				"], Last Access Time: [" << (entry.last_access() ? detail::format_time(last_access) : "0") << "], " <<
				std::string(entry.path().begin(), entry.path().end()) << std::endl;

			entry.enum_recent_items([&](ea::recent_app::recent_item& item_entry) {
				auto item_last_access = detail::filetime_to_unix_timestamp(item_entry.last_access());
				ss << "    Last Acccess Time: [" << (item_entry.last_access() ? detail::format_time(item_last_access) : "0") << "], " <<
					std::string(item_entry.display_name().begin(), item_entry.display_name().end()) << std::endl;
			});
		});

		return ss.str();
	}

	std::string get_user_assist_info()
	{
		wchar_t buffer[256];
		auto    end = buffer;
		ea::acquire_and_format_sid(end);
		std::wstring_view sid(buffer, end - buffer);

		std::stringstream ss;
		auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		ss << "Dumping UserAssist, Timestamp: " << std::ctime(&now) << std::endl;
		ss << "[!] Note: Ran Count may not be 100\% valid." << std::endl;
		ea::enum_user_assist(sid, [&ss](ea::user_assist_entry_t& entry) {
			auto last_execution_time = detail::filetime_to_unix_timestamp(entry.last_execution_time);
			ss << 
				"Ran Count: [" << entry.run_counter <<
				"], Focus Time: [" << (entry.focus_time_had ? detail::format_time(entry.focus_time_had / 1000) : "0") <<
				"], Last Execution Time: [" << (entry.last_execution_time ? detail::format_time(last_execution_time) : "0") << "], " <<
				std::string(entry.name.begin(), entry.name.end()) << std::endl;
		});
		return ss.str();
	}

	void get_usn_journal_info_deferred(std::string& result, bool& completed)
	{
		// This takes an unreasonably long time without multithreading when
		// multiple drives are mounted on the system.
		std::thread main_thread([&]() {
			std::stringstream ss;
			auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
			ss << "Dumping UsnJournal, Timestamp: " << std::ctime(&now) << std::endl;

			int drive_count = 0;
			std::vector<std::pair<std::stringstream, int>> sstreams;
			ntw::obj::process_ref{}.enum_fixed_drives([&](auto drive_idx) {
				sstreams.emplace_back(std::make_pair(std::stringstream(), drive_idx));
				drive_count++;
			});

			for (int i = 0; i < drive_count; i++) {
				std::thread worked_thread([&, i]() {
					auto&[stream, drive_index] = sstreams[i];
					stream << "NTFS Journal data for drive " << (char)(drive_index + 'A') << ":\\" << std::endl;
					ea::enum_drive_usn_journal(drive_index, [&](ea::usn_journal_entry_t& entry) {
						auto interact_time_utf = detail::filetime_to_unix_timestamp(entry.interact_time_utf);
						sstreams[i].first <<
							"    Time: [" << (entry.interact_time_utf ? detail::format_time(interact_time_utf) : "0") <<
							"], Reason: [0x" << std::hex << entry.reason << "], " <<
							std::string(entry.path.begin(), entry.path.end()) << std::endl;
					});
					drive_count--;
				});
				worked_thread.detach();
			}

			while (drive_count != 0)
				Sleep(200);

			for (auto&[stream, drive_index] : sstreams)
				result += stream.str();
			completed = true;
		});
		main_thread.detach();
	}

} // namespace ea