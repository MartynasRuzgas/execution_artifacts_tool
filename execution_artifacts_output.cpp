#include "execution_artifacts_output.hpp"
#include <app_compat_flags.hpp>
#include <jump_lists.hpp>
#include <mui_cache.hpp>
#include <recent_apps.hpp>
#include <shim_cache.hpp>
#include <user_assist.hpp>
#include <usn_journal.hpp>

#include <thread>

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
		return {}; // unimpl.
	}

	std::string get_usn_journal_info()
	{
		return {}; // unimpl.
	}

} // namespace ea