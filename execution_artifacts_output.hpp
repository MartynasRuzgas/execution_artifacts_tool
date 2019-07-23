#pragma once
#include <string>

namespace ea {

	std::string get_app_compat_flags_info();
	std::string get_jump_lists_info();
	std::string get_mui_cache_info();
	std::string get_shim_cache_info();
	std::string get_recent_apps_info();
	std::string get_user_assist_info();
	void get_usn_journal_info_deferred(std::string& result, bool& completed);

} // namespace ea