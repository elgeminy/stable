
#pragma once
#include "pch.h"
#include "direnum.h"
#include "comparer.h"

#define ARG_PREFIX		L'-'
#define ARG_USAGE		L"-?"

#define ARG_INC_FILES	L"-inf"
#define ARG_EXC_FILES	L"-exf"

#define ARG_INC_DIRS	L"-ind"
#define ARG_EXC_DIRS	L"-exd"

#define ARG_INC_PATHS	L"-inp"
#define ARG_EXC_PATHS	L"-exp"

#define ARG_MIN_SIZE	L"-min"
#define ARG_MAX_SIZE	L"-max"

#define ARG_PRINT_HASH	L"-hash"

using UsageFunc = std::function <bool (std::wstring, bool)>;

bool ReadArg (DirEnumerator & de, bool find_all_hashes, UsageFunc usage_foo);
void PrintEqualGroup (const ListOfFiles & group);
void PrintFailedFiles (const ListOfFiles & failed);

struct DirEnumHandler : public IDirEnumHandler
{
	DirEnumHandler (UsageFunc usage) :
		_usage (usage)
	{}

	void OnGivenPathFail (const std::wstring & file, std::wstring error);

	void OnFileFound (std::filesystem::path && file, uintmax_t size);
	void OnFileFound (const std::filesystem::path & file, uintmax_t size);

	void OnDirFound (const std::filesystem::path & dir) {}

	void OnScanError (const std::string & error);

	std::map <uintmax_t, ListOfFiles> _files;
	UsageFunc _usage;
};
