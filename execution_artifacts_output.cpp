#include "execution_artifacts_output.hpp"
#include <recent_apps.hpp>
#include <shim_cache.hpp>
#include <usn_journal.hpp>
#include <app_compat_flags.hpp>
#include <jump_lists.hpp>
#include <mui_cache.hpp>
#include <user_assist.hpp>
#include <sid.hpp>

#include <thread>
#include <sstream>

namespace ea {

	std::string get_app_compat_flags_info()
	{
		return {}; // unimpl.
	}

	std::string get_jump_lists_info()
	{
		ea::test_jump_lists();
		return {}; // unimpl.
	}

	std::string get_mui_cache_info()
	{
		return {}; // unimpl.
	}

	std::string get_shim_cache_info()
	{
		return {}; // unimpl.
	}

	std::string get_user_assist_info()
	{
		wchar_t buffer[256];
		auto    end = buffer;
		ea::acquire_and_format_sid(end);
		std::wstring_view sid(buffer, end - buffer);

		std::stringstream ss;
		ea::enum_user_assist(sid, [&ss](ea::user_assist_entry_t& entry) {
			ss << "Ran Count: [" << entry.run_counter << "] " << std::string(entry.name.begin(), entry.name.end()) << std::endl;
		});
		return ss.str();
	}

	std::string get_usn_journal_info()
	{
		return {}; // unimpl.
	}

} // namespace ea