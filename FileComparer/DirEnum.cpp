
#include "pch.h"
#include "direnum.h"


bool DirEnumerator::SetScanDirectories (const ListOfStrings & list, std::function <bool (std::wstring, bool)> fail_callback)
{
	for (const auto & path : list)
	{
		if (::PathIsRelative (path.c_str ()))
		{
			std::vector <wchar_t> fullpath;
			DWORD size = GetFullPathName (path.c_str (), 0, fullpath.data (), nullptr);
			if (0 == size)
				return fail_callback (L"GetFullPathName", true);

			fullpath.resize (size + 1);
			size = GetFullPathName (path.c_str (), size, fullpath.data (), nullptr);
			if (0 == size)
				return fail_callback (L"GetFullPathName", true);

			m_dir_pathes.emplace_back (fullpath.data ());
		}
		else
			m_dir_pathes.push_back (path);

		try
		{
			if (!std::filesystem::exists (m_dir_pathes.back ()))
				return fail_callback (L"Path \'" + m_dir_pathes.back ().wstring () + L"' not found", false);

			if (!std::filesystem::is_directory (m_dir_pathes.back ()))
			{
				m_file_pathes.push_back (std::move (m_dir_pathes.back ()));
				m_dir_pathes.pop_back ();
			}
		}
		catch (...)
		{
			return fail_callback (L"Path \'" + m_dir_pathes.back ().wstring () + L"' is invalid", false);
		}
	}

	return true;
}

void DirEnumerator::AddFileList (ListOfStrings & list, ListOfVecsOfStrings & add_to)
{
	std::transform (list.begin (), list.end (), list.begin (), 
		[] (std::wstring & buff)
	{
		std::transform (buff.begin (), buff.end (), buff.begin (),
			[](wchar_t c)
			{
				return std::towlower (c);
			}
		);
		return buff;
	});

	VectorOfStrings spl;
	for (auto & mask : list)
	{
		std::wstring buff;
		buff.reserve (mask.size ());
		for (wchar_t c : mask)
		{
			if (L'*' == c)
			{
				if (!buff.empty ())
					spl.push_back (buff);

				if (spl.empty () || !spl.empty () && spl.back () != L"*")
					spl.emplace_back (L"*");

				buff.clear ();
			}
			else
				buff += c;
		}
		if (!buff.empty ())
			spl.push_back (std::move (buff));

		if (!spl.empty ())
			add_to.push_back (std::move (spl));
	}
}

void DirEnumerator::AddExcludeDirectories (ListOfStrings & list)
{
	AddFileList (list, m_exc_dir_mask);
}

void DirEnumerator::AddExcludeFiles (ListOfStrings & list)
{
	AddFileList (list, m_exc_file_mask);
}

void DirEnumerator::AddIncludeDirectories (ListOfStrings & list)
{
	AddFileList (list, m_inc_dir_mask);
}

void DirEnumerator::AddIncludeFiles (ListOfStrings & list)
{
	AddFileList (list, m_inc_file_mask);
}

void DirEnumerator::SetFileLimit (uintmax_t minsize /*= 0*/, uintmax_t maxsize /*= (uintmax_t)-1*/)
{
	m_min_size = minsize;
	m_max_size = maxsize;
}

bool DirEnumerator::MatchMask (const VectorOfStrings & mask, const std::wstring & str)
{
	auto StartsWith = [](const std::wstring & search, const std::wstring & str)
	{
		size_t p = str.find (search);
		return 0 == p;
	};

	auto EndsWith = [](const std::wstring & search, const std::wstring & str)
	{
		size_t p = str.rfind (search);
		if (std::wstring::npos == p)
			return false;
		return str.size () - search.size () == p;
	};

	if (mask.size () == 1)
	{
		if (L"*" == mask.front () || str == mask.front ())
			return true;
		return false;
	}
	if (mask.size () == 2)
	{
		const auto & f = mask.front ();
		const auto & b = mask.back ();
		if (L"*" == f)
			return EndsWith (b, str);
		return StartsWith (f, str);
	}

	const auto & f = mask.front ();
	const auto & b = mask.back ();

	size_t pos_b = 0, pos_e = str.size () - 1;

	if (f != L"*")
	{
		if (!StartsWith (f, str))
			return false;
		pos_b = f.size ();
	}

	if (b != L"*")
	{
		if (!EndsWith (b, str))
			return false;
		pos_e = str.size () - b.size ();
	}

	for (size_t i = 1; i < mask.size () - 1; i++)
	{
		const auto & search = mask[i];
		if (L"*" == search)
			continue;

		size_t pos = str.find (search, pos_b);
		if (std::wstring::npos == pos)
			return false;
		if (pos + search.size () > pos_e)
			return false;
		pos_b = pos + search.size ();
	}

	return true;
}

bool DirEnumerator::IsObjectIgnored (const wchar_t * pobj, bool is_dir, uintmax_t filesize)
{
	if (!is_dir && m_min_size > 0 && filesize < m_min_size)
		return true;
	if (!is_dir && m_max_size != (uintmax_t)-1 && filesize > m_max_size)
		return true;

	if (!m_opt_file && !is_dir || !m_opt_dir && is_dir)
		return false;

	auto MakeLower = [](const wchar_t * buff) -> std::wstring
	{
		std::vector <wchar_t> tmp;
		size_t size = wcslen (buff);
		tmp.resize (size + 1);
		wcscpy_s (tmp.data (), tmp.size (), buff);
		::CharLowerBuff (tmp.data (), static_cast <DWORD> (size));
		return tmp.data ();
	};
	std::wstring obj = MakeLower (pobj);

	struct
	{
		ListOfVecsOfStrings & mask;
		bool exclude_mask;
		bool for_dirs;
	}
	mask [] = 
	{
		{ m_exc_file_mask, true, false },
		{ m_exc_dir_mask, true, true },
		{ m_inc_file_mask, false, false },
		{ m_inc_dir_mask, false, true },
	};

	for (auto & m : mask)
	{
		if (is_dir && m.for_dirs || !is_dir && !m.for_dirs)
		{
			bool match_found = false;

			for (auto & mask_item : m.mask)
			{
				if (match_found = MatchMask (mask_item, obj))
					break;
			}

			if (m.exclude_mask && match_found)
				return true;
			if (!m.exclude_mask && !match_found && !m.mask.empty ())
				return true;
		}
	}

	return false;
}

void DirEnumerator::EnumerateDirectory_Win7 (const std::filesystem::path & root, ListOfStrings & ignored, std::map <uintmax_t, ListOfFiles> & files)
{
	try
	{
		for (auto & obj : std::filesystem::recursive_directory_iterator (root))
		{
			uintmax_t size = obj.file_size ();
			bool is_dir = std::filesystem::is_directory (obj);

			if (IsObjectIgnored (::PathFindFileName (obj.path ().c_str ()), is_dir, size))
			{
				ignored.emplace_back (obj.path ().wstring ());
				continue;
			}

			if (!is_dir)
			{
				files[size].push_back (std::filesystem::path (obj.path ()));
			}
		}
	}
	catch (const std::exception & ex)
	{
		throw EnumException { ex.what () };
	}
}

void DirEnumerator::EnumerateDirectory_WinXp (const std::filesystem::path & root, ListOfStrings & ignored, std::map <uintmax_t, ListOfFiles> & files)
{
	ListOfPaths dirs;
	dirs.push_back (root);

	WIN32_FIND_DATA fd = {};

	do
	{
		auto dir = dirs.begin ();
		(*dir) /= L"*.*";

		HANDLE hfind = ::FindFirstFile (dir->c_str (), &fd);
		if (hfind != INVALID_HANDLE_VALUE)
		{
			std::unique_ptr <void, decltype (&FindClose)> h (hfind, FindClose);
			dir->remove_filename ();

			do
			{
				if (wcscmp (fd.cFileName, L".") == 0 || wcscmp (fd.cFileName, L"..") == 0)
					continue;

				uintmax_t size = (fd.nFileSizeHigh * (MAXDWORD + 1I64)) + fd.nFileSizeLow;
				bool is_dir = (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY;

				std::filesystem::path obj (*dir);
				obj /= fd.cFileName;

				if (IsObjectIgnored (fd.cFileName, is_dir, size))
				{
					ignored.emplace_back (obj.wstring ());
					continue;
				}

				if (is_dir)
				{
					dirs.push_back (std::move (obj));
				}
				else
				{
					files[size].push_back (std::move (obj));
				}
			} 
			while (::FindNextFile (hfind, &fd));
		}

		dirs.pop_front ();
	} 
	while (!dirs.empty ());
}

void DirEnumerator::EnumerateDirectory (const std::filesystem::path & root, ListOfStrings & ignored, std::map <uintmax_t, ListOfFiles> & found)
{
	// will not ignore root folders
	if (!m_opt_dir && IsWindows7OrGreater ())
		return EnumerateDirectory_Win7 (root, ignored, found);

	// std::filesystem::recursive_directory_iterator will throw an exception on WinXP
	return EnumerateDirectory_WinXp (root, ignored, found);
}

void DirEnumerator::EnumerateDirectory (ListOfStrings & ignored, std::map <uintmax_t, ListOfFiles> & found)
{
	m_opt_file = !m_exc_file_mask.empty () || !m_inc_file_mask.empty ();
	m_opt_dir = !m_exc_dir_mask.empty () || !m_inc_dir_mask.empty ();

	for (auto & file : m_file_pathes)
	{
		uintmax_t size = std::filesystem::file_size (file);
		found[size].push_back (std::move (file));
	}

	for (auto & dir : m_dir_pathes)
	{
		EnumerateDirectory (dir, ignored, found);
	}
}
