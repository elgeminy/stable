
#pragma once
#include "File.h"

using ListOfStrings = std::list <std::wstring>;
using VectorOfStrings = std::vector <std::wstring>;
using ListOfVecsOfStrings = std::list <VectorOfStrings>;
using ListOfPaths = std::list <std::filesystem::path>;

struct EnumException
{
	std::string error;
};

class DirEnumerator
{
	ListOfPaths m_dir_pathes;
	ListOfPaths m_file_pathes;

	uintmax_t m_min_size = 0;
	uintmax_t m_max_size = (uintmax_t)-1;

	ListOfVecsOfStrings m_exc_file_mask;
	ListOfVecsOfStrings m_exc_dir_mask;

	ListOfVecsOfStrings m_inc_file_mask;
	ListOfVecsOfStrings m_inc_dir_mask;

	bool m_opt_file = false;
	bool m_opt_dir = false;

	bool MatchMask (const VectorOfStrings & mask, const std::wstring & str);
	void AddFileList (ListOfStrings & list, ListOfVecsOfStrings & add_to);
	bool IsObjectIgnored (const wchar_t * pobj, bool is_dir, uintmax_t filesize);
	void EnumerateDirectory (const std::filesystem::path & root, ListOfStrings & ignored, std::map <uintmax_t, ListOfFiles> & found);
	void EnumerateDirectory_Win7 (const std::filesystem::path & root, ListOfStrings & ignored, std::map <uintmax_t, ListOfFiles> & files);
	void EnumerateDirectory_WinXp (const std::filesystem::path & root, ListOfStrings & ignored, std::map <uintmax_t, ListOfFiles> & files);

public:
	bool SetScanDirectories (const ListOfStrings & list, std::function <bool(std::wstring, bool)> fail_callback);
	void AddExcludeDirectories (ListOfStrings & list);
	void AddExcludeFiles (ListOfStrings & list);
	void AddIncludeDirectories (ListOfStrings & list);
	void AddIncludeFiles (ListOfStrings & list);
	void SetFileLimit (uintmax_t minsize = 0, uintmax_t maxsize = (uintmax_t)-1);

	void EnumerateDirectory (ListOfStrings & ignored, std::map <uintmax_t, ListOfFiles> & found);
};