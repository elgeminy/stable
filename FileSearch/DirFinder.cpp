
#include "pch.h"
#include "dirfinder.h"

bool ReadArg (DirEnumerator & de, UsageFunc usage_foo)
{
	int num = 0;
	wchar_t ** ppcmd = CommandLineToArgvW (::GetCommandLine (), &num);
	if (nullptr == ppcmd)
		return usage_foo (L"CommandLineToArgvW", true);
	std::unique_ptr <wchar_t *, decltype(&LocalFree)> pp (ppcmd, LocalFree);

	auto ReadList = [&ppcmd, &num](int & n, ListOfStrings & list)
	{
		list.clear ();
		for (; n < num; n++)
		{
			const wchar_t * ptr = ppcmd[n];
			if (ARG_PREFIX == *ptr)
			{
				return !list.empty ();
			}

			list.emplace_back (ptr);
		}

		return true;
	};

	ListOfStrings list;

	struct
	{
		const wchar_t * arg = nullptr;
		std::function <void (ListOfStrings &)> callback;
	} 
	args [] = 
	{
		{ ARG_EXC_DIRS, std::bind (&DirEnumerator::AddExcludeDirectories, &de, std::placeholders::_1) },
		{ ARG_EXC_PATHS, std::bind (&DirEnumerator::AddExcludePaths, &de, std::placeholders::_1) },
		{ ARG_INC_PATHS, std::bind (&DirEnumerator::AddIncludePaths, &de, std::placeholders::_1) },
		{ ARG_SCAN_PATH, std::bind (&DirEnumerator::SetScanDirectories, &de, std::placeholders::_1) },
	};

	bool search_dir_found = false, scan_dir_found = false;

	for (int n = 1; n < num; )
	{
		const wchar_t * ptr = ppcmd[n];

		if (_wcsicmp (ptr, ARG_USAGE) == 0)
		{
			return usage_foo (L"", false);
		}

		bool arg_found = false;
		bool arg_valid = true;

		for (auto & arg : args)
		{
			if (_wcsicmp (ptr, arg.arg) == 0)
			{
				arg_found = true;
				arg_valid = ReadList (++n, list);

				if (arg_valid && arg.callback != nullptr)
					arg.callback (list);

				if (_wcsicmp (ptr, ARG_SCAN_PATH) == 0)
					scan_dir_found = true;
			}
			if (arg_found)
				break;
		}

		if (!arg_found)
		{
			if (arg_valid = ReadList (n, list))
			{
				search_dir_found = true;
				de.AddIncludeDirectories (list);
			}
		}

		if (!arg_valid)
			return usage_foo (L"invalid argument", false);
	}

	if (!scan_dir_found)
	{
		de.SetScanDirectories ({ std::filesystem::current_path () });
	}

	ListOfStrings all { L"*" };
	if (!search_dir_found)
		de.AddIncludeDirectories (all);

	de.AddExcludeFiles (all);

	return true;
}

void DirEnumHandler::OnGivenPathFail (const std::wstring & file, std::wstring error)
{
	_usage (file + L": " + error, false);
}

void DirEnumHandler::OnDirFound (const std::filesystem::path & dir)
{
	_dirs.push_back (dir);
	_dir_found_callback (dir);
}

void DirEnumHandler::OnScanError (const std::string & error)
{
	printf ("%s\n", error.c_str ());
}
