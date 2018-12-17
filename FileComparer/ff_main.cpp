
#include "pch.h"
#include "filefinder.h"

bool Usage (std::wstring error, bool sys_fail = false)
{
	std::vector <wchar_t> path (MAX_32PATH);
	const wchar_t * pfile = nullptr;
	if (::GetModuleFileName (nullptr, path.data (), MAX_32PATH) > 0)
		pfile = ::PathFindFileName (path.data ());
	if (nullptr == pfile)
		pfile = L"FileFinder.exe";

	if (!error.empty ())
	{
		if (sys_fail)
			std::wcout << L"System failure in func " << error << L", error " << ::GetLastError () << std::endl;
		else
			std::wcout << L"Invalid command: " << error << std::endl;
	}
	std::wcout 
		<< "Usage:\n\n"
		<< pfile <<
			L"  [<file_1> ... <file_N>] [" ARG_SCAN_PATH L"]\n" 
				L"\t[" ARG_MIN_SIZE L"] [" ARG_MAX_SIZE L"]\n" 
				L"\t[" ARG_EXC_FILES L"] [" ARG_INC_PATHS L"] [" ARG_EXC_PATHS L"] [" ARG_INC_DIRS L"] [" ARG_EXC_DIRS L"]\n"
				L"\t[" ARG_HASH L"] [" ARG_SIZE L"]\n" 
				L"\t[" ARG_GROUP_EXT L"] [" ARG_GROUP_HASH L"] [" ARG_GROUP_SIZE L"]\n"
			L"\n"
			L"  <file_1...N>\t\t\t- list of file's masks to search (symbols (*) and (?) are allowed).\t\t\t\t\t\t\t\tOmit to find all files (e.g. '*')\n"

			L"  " ARG_SCAN_PATH L" <dir_path>\t\t- path to a directory to search in. Currect dir used by default\n"

			L"  " ARG_MIN_SIZE L" <file_size>\t\t- min file size to search\n"
			L"  " ARG_MAX_SIZE L" <file_size>\t\t- max file size to search\n"

			L"  " ARG_EXC_FILES L" <mask_1> ... <mask_N>\t- list of files' masks to skip from searching (symbols (*) and (?) are allowed)\n"

			L"  " ARG_INC_PATHS L" <mask_1> ... <mask_N>\t- list of path' masks to search in (symbols (*) and (?) are allowed)\n"
			L"  " ARG_EXC_PATHS L" <mask_1> ... <mask_N>\t- list of paths' masks to skip from searching (symbols (*) and (?) are allowed)\n"

			L"  " ARG_INC_DIRS L" <mask_1> ... <mask_N>\t- list of dirs' masks to search in (symbols (*) and (?) are allowed)\n"
			L"  " ARG_EXC_DIRS L" <mask_1> ... <mask_N>\t- list of dirs' masks to skip from searching (symbols (*) and (?) are allowed)\n"

			L"  " ARG_HASH L"\t\t\t\t- print found file' SHA1 hash\n"
			L"  " ARG_SIZE L"\t\t\t\t- print found file' size\n"

			L"  " ARG_GROUP_EXT L"\t\t\t\t- print found files grouping by extension\n"
			L"  " ARG_GROUP_HASH L"\t\t\t\t- print found files grouping by SHA1 hash\n"
			L"  " ARG_GROUP_SIZE L"\t\t\t\t- print found files grouping by size\n"
		L"\n"
	;

	return false;
}

int main ()
{
	::_wsetlocale (LC_CTYPE, L"");

	try
	{
		Opt opts;
		auto PrintFile = [&opts, hash_func = CalcHashAndPrint](File * file)
		{
			if (opts.group_ext || opts.group_hash || opts.group_size)
				return;

			if (!opts.print_hash)
			{
				if (opts.print_size)
					wprintf (L"%s [%s]\n", file->Path (), file->SizeFormatted ().c_str ());
				else
					wprintf (L"%s\n", file->Path ());

				return;
			}

			hash_func (file, opts.print_size);
		};

		DirEnumHandler handler (PrintFile, Usage);
		DirEnumerator de (&handler);
		bool hash = false, verbose = false;
		if (!ReadArg (de, opts, Usage))
			return 0;

		de.EnumerateDirectory ();

		if (opts.group_ext || opts.group_hash || opts.group_size)
			PrintGrouped (opts, handler._files);

		std::wcout << L"\n" << handler._files.size () << L" files found\n";
	}
	catch (EnumException & ex)
	{
		std::cout << "Unexpected error occuped: " << ex.error << std::endl;
	}
	catch (std::exception & ex)
	{
		std::cout << "Unexpected error occuped: " << ex.what () << std::endl;
	}
	catch (...)
	{
		std::wcout << L"Unexpected error occuped" << std::endl;
	}

	return 0;
}
