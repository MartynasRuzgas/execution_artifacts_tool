/*
Copyright (c) 2019 Martynas Ruzgas

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "eat_output.hpp"
#include <recent_apps.hpp>
#include <shim_cache.hpp>
#include <usn_journal.hpp>
#include <app_compat_flags.hpp>
#include <mui_cache.hpp>
#include <user_assist.hpp>
#include <sid.hpp>
#include <mru.hpp>

#include <vector>
#include <sstream>
#include <ctime>
#include <thread>
#include <chrono>

namespace eat {

    namespace detail {

        inline std::string format_time(int64_t time)
        {
            /* y/m/d */
            std::stringstream ss;
            if(time / 3600 > 24) {
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

        inline int64_t filetime_to_unix_timestamp(int64_t filetime)
        {
            return filetime / 10000000.0 - 11644473600.0;
        }

        inline int64_t filetime_to_unix_timestamp(FILETIME filetime)
        {
            return filetime_to_unix_timestamp(*(int64_t*)&filetime);
        }

    } // namespace detail

    std::string get_run_mru()
    {
        wchar_t buffer[256];
        auto    end = buffer;
        ea::acquire_and_format_sid(end);
        std::wstring_view sid(buffer, end - buffer);

        std::stringstream ss;
        auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        ss << "Dumping RunMRU, Timestamp: " << std::ctime(&now);
        ss << "Note: Entries are sorted from most recent to least." << std::endl
           << std::endl;

        ea::enum_runmru(sid, [&](ea::run_mru_t& entries) {
            for(auto& entry : entries) {
                ss << std::string(entry.begin(), entry.end()).c_str() << std::endl;
            }
        });

        return ss.str();
    }

    std::string get_recent_docs_mru()
    {
        wchar_t buffer[256];
        auto    end = buffer;
        ea::acquire_and_format_sid(end);
        std::wstring_view sid(buffer, end - buffer);

        std::stringstream ss;
        auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        ss << "Dumping RecentDocsMRU, Timestamp: " << std::ctime(&now);
        ss << "Note: Entries are sorted from most recent to least." << std::endl
           << std::endl;

        ea::enum_recentdocsmru(sid, [&](ea::recent_docs_mru_t& entries) {
            for(auto& entry : entries) {
                ss << std::string(entry.begin(), entry.end()).c_str() << std::endl;
            }
        });

        return ss.str();
    }

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
            ss << "[Path = " << std::string(entry.path.begin(), entry.path.end())
               << "] [Description = "
               << std::string(entry.description.begin(), entry.description.end()) << "]"
               << std::endl;
        });

        return ss.str();
    }

    std::string get_shim_cache_info()
    {
        std::stringstream ss;
        auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        ss << "Dumping ShimCache, Timestamp: " << std::ctime(&now) << std::endl;

        ea::enum_shim_cache([&](ea::shim_entry_t& entry) {
            auto last_modification_time =
                detail::filetime_to_unix_timestamp(entry.last_modification_time);
            ss << "Time: ["
               << (entry.last_modification_time.dwLowDateTime
                       ? detail::format_time(last_modification_time)
                       : "Invalid")
               << "], " << std::string(entry.path.begin(), entry.path.end()) << std::endl;
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
            ss << "Launch Count: [" << entry.launch_count() << "], Last Access Time: ["
               << (entry.last_access() ? detail::format_time(last_access) : "Invalid")
               << "], " << std::string(entry.path().begin(), entry.path().end())
               << std::endl;

            entry.enum_recent_items([&](ea::recent_app::recent_item& item_entry) {
                auto item_last_access =
                    detail::filetime_to_unix_timestamp(item_entry.last_access());
                ss << "    Last Acccess Time: ["
                   << (item_entry.last_access() ? detail::format_time(item_last_access)
                                                : "Invalid")
                   << "], "
                   << std::string(item_entry.display_name().begin(),
                                  item_entry.display_name().end())
                   << std::endl;
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
            auto last_execution_time =
                detail::filetime_to_unix_timestamp(entry.last_execution_time);
            ss << "Ran Count: [" << entry.run_counter << "], Focus Time: ["
               << (entry.focus_time_had ? detail::format_time(entry.focus_time_had / 1000)
                                        : "Invalid")
               << "], Last Execution Time: ["
               << (entry.last_execution_time ? detail::format_time(last_execution_time)
                                             : "Invalid")
               << "], " << std::string(entry.name.begin(), entry.name.end()) << std::endl;
        });
        return ss.str();
    }

    void get_usn_journal_info_deferred(std::string& result, bool& completed)
    {
        // This takes an unreasonably long time without multithreading with
        // multiple drives are mounted on the system.
        std::thread([&]() {
            std::stringstream ss;
            auto              now =
                std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            ss << "Dumping UsnJournal, Timestamp: " << std::ctime(&now);
            ss << "Visit https://docs.microsoft.com/en-us/windows/win32/api/winioctl/ns-winioctl-usn_record_v2 for the reason values.\n\n";

            int                                            drive_count = 0;
            std::vector<std::pair<std::stringstream, int>> sstreams;
            ntw::obj::process_ref{}.enum_fixed_drives([&](auto drive_idx) {
                sstreams.emplace_back(std::make_pair(std::stringstream(), drive_idx));
                drive_count++;
            });

			// drive_count gets written to in other threads so use a copy.
			int drive_count_temp = drive_count;
            for(int i = 0; i < drive_count_temp; i++) {
                std::thread([&, i]() {
                    auto& [stream, drive_index] = sstreams[i];
                    stream << "NTFS Journal data for drive " << (char)(drive_index + 'A')
                           << ":\\" << std::endl;

                    auto status = ea::enum_drive_usn_journal(
                        drive_index, [&](ea::usn_journal_entry_t& entry) {
                            auto interact_time_utf = detail::filetime_to_unix_timestamp(
                                entry.interact_time_utf);
                            sstreams[i].first
                                << "    Time: ["
                                << (entry.interact_time_utf
                                        ? detail::format_time(interact_time_utf)
                                        : "Invalid")
                                << "], Reason: [0x" << std::hex << entry.reason << "], "
                                << std::string(entry.path.begin(), entry.path.end())
                                << std::endl;
                        });

                    // Error handling
                    if(!NT_SUCCESS(status)) {
                        if(status == STATUS_ACCESS_DENIED)
                            sstreams[i].first
                                << "Access denied to UsnJournal, try running as administrator.\n";
                        else
                            sstreams[i].first << "Unknown error " << status << std::endl;
                    }

                    drive_count--;
                })
                    .detach();
            }

            // Wait until all drives are processed.
            while(drive_count != 0)
                Sleep(100);

            result += ss.str();
            for(auto& [stream, drive_index] : sstreams)
                result += stream.str();
            completed = true;
        })
            .detach();
    }

} // namespace eat