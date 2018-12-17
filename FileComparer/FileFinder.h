
#pragma once
#include "pch.h"
#include "direnum.h"
#include "file.h"

#define ARG_PREFIX		L'-'
#define ARG_USAGE		L"-?"

#define ARG_SCAN_PATH	L"-path"

#define ARG_EXC_FILES	L"-exf"

#define ARG_INC_DIRS	L"-ind"
#define ARG_EXC_DIRS	L"-exd"

#define ARG_INC_PATHS	L"-inp"
#define ARG_EXC_PATHS	L"-exp"

#define ARG_MIN_SIZE	L"-min"
#define ARG_MAX_SIZE	L"-max"

#define ARG_HASH		L"-hash"

#define ARG_PRINT_HASH	L"-psh"
#define ARG_PRINT_SIZE	L"-psz"

#define ARG_GROUP_EXT	L"-gex"
#define ARG_GROUP_SIZE	L"-gsz"
#define ARG_GROUP_HASH	L"-gsh"

using UsageFunc = std::function <bool (std::wstring, bool)>;

struct Opt
{
	ListOfStrings hashes;
	bool print_hash = false;
	bool print_size = false;
	bool group_size = false;
	bool group_ext = false;
	bool group_hash = false;
};

bool ReadArg (DirEnumerator & de, Opt & opts, UsageFunc usage_foo);
void CalcHashAndPrint (File * file, bool print_size, Opt & opts);
void PrintGrouped (const Opt & opts, ListOfFiles & files);

struct DirEnumHandler : public IDirEnumHandler
{
	DirEnumHandler (std::function <void (File *)> fc, UsageFunc usage) :
		_file_found_callback (fc),
		_usage (usage)
	{}

	void OnGivenPathFail (const std::wstring & file, std::wstring error);

	void OnFileFound (std::filesystem::path && file, uintmax_t size);
	void OnFileFound (const std::filesystem::path & file, uintmax_t size);

	void OnDirFound (const std::filesystem::path & dir) {}

	void OnScanError (const std::string & error);


	std::function <void (File *)> _file_found_callback;
	UsageFunc _usage;
	ListOfFiles _files;
};

