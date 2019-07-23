#pragma once
#include <string>

#include <app_compat_flags.hpp>
#include <jump_lists.hpp>
#include <mui_cache.hpp>
#include <recent_apps.hpp>
#include <shim_cache.hpp>
#include <user_assist.hpp>
#include <usn_journal.hpp>

namespace ea {

	std::string get_app_compat_flags_info();
	std::string get_jump_lists_info();
	std::string get_mui_cache_info();
	std::string get_shim_cache_info();
	std::string get_user_assist_info();
	std::string get_usn_journal_info();

} // namespace ea