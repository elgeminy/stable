
#pragma once

using ListOfStrings = std::list <std::wstring>;
using ListOfPaths = std::list <std::filesystem::path>;

struct IDirEnumHandler
{
	virtual void OnGivenPathFail (const std::wstring & file, std::wstring error) = 0;

	virtual void OnFileFound (std::filesystem::path && file, uintmax_t size) = 0;
	virtual void OnFileFound (const std::filesystem::path & file, uintmax_t size) = 0;

	virtual void OnDirFound (const std::filesystem::path & dir) = 0;

	virtual void OnScanError (const std::string & error) = 0;
};


struct EnumException
{
	std::string error;
};

class DirEnumerator
{
	struct Mask
	{
		std::wstring mask;
		bool use_regex = true;
	};
	using ListOfMasks = std::list <Mask>;

	IDirEnumHandler * m_pHandler = nullptr;

	ListOfPaths m_dir_pathes;
	ListOfPaths m_file_pathes;

	uintmax_t m_min_size = 0;
	uintmax_t m_max_size = (uintmax_t)-1;

	ListOfMasks m_exc_file_mask;
	ListOfMasks m_exc_dir_mask;

	ListOfMasks m_inc_file_mask;
	ListOfMasks m_inc_dir_mask;

	ListOfMasks m_inc_path_mask;
	ListOfMasks m_exc_path_mask;

	bool m_opt_file = false;
	bool m_opt_dir = false;
	bool m_opt_path = false;

	enum class ObjType
	{
		file, directory, path
	};

private:
	bool MatchMask (const Mask & mask, const std::wstring & str);

	void AddFileList (ListOfStrings & list, ListOfMasks & add_to);
	bool IsObjectIgnored (std::wstring & obj, uintmax_t filesize, ObjType type, bool scan_dir_includes);
	void EnumerateDirectory (const std::filesystem::path & root);
	void EnumerateDirectory_Win7 (const std::filesystem::path & root);
	void EnumerateDirectory_WinXp (const std::filesystem::path & root);

public:
	DirEnumerator (IDirEnumHandler * handler);

	bool SetScanDirectories (const ListOfStrings & list);
	void AddExcludeDirectories (ListOfStrings & list);
	void AddExcludeFiles (ListOfStrings & list);
	void AddExcludePaths (ListOfStrings & list);
	void AddIncludeDirectories (ListOfStrings & list);
	void AddIncludeFiles (ListOfStrings & list);
	void AddIncludePaths (ListOfStrings & list);
	void SetFileLimit (uintmax_t minsize = 0, uintmax_t maxsize = (uintmax_t)-1);

	void EnumerateDirectory ();
};
