
#include "pch.h"
#include "filecomparer.h"

bool Usage (std::wstring error, bool sys_fail = false)
{
	std::vector <wchar_t> path (MAX_32PATH);
	const wchar_t * pfile = nullptr;
	if (::GetModuleFileName (nullptr, path.data (), MAX_32PATH) > 0)
		pfile = ::PathFindFileName (path.data ());
	if (nullptr == pfile)
		pfile = L"FileComparer.exe";

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
			L"  [<file_or_dir_1> ... <file_or_dir_N>] "
			L"\t[" ARG_MIN_SIZE L"] [" ARG_MAX_SIZE L"]\n"
			L"\t[" ARG_INC_FILES L"] [" ARG_EXC_FILES L"] [" ARG_INC_PATHS L"] [" ARG_EXC_PATHS L"] [" ARG_INC_DIRS L"] [" ARG_EXC_DIRS L"]\n"
			L"\t[" ARG_PRINT_HASH L"]\n"
			L"\n"

			L"  <file_or_dir_1...N>\t\t- list of files and/or directories to scan for comparing files.\t\t\t\t\t\t\t\tOmit to scan current directory\n"

			L"  " ARG_MIN_SIZE L" <file_size>\t\t- min file size to compare\n"
			L"  " ARG_MAX_SIZE L" <file_size>\t\t- max file size to compare\n"

			L"  " ARG_INC_FILES L" <mask_1...N>\t\t- list of files' masks to compare (symbols (*) and (?) are allowed)\n"
			L"  " ARG_EXC_FILES L" <mask_1...N>\t\t- list of files' masks to skip from comparison (symbols (*) and (?) are allowed)\n"

			L"  " ARG_INC_PATHS L" <mask_1...N>\t\t- list of path' masks to compare (symbols (*) and (?) are allowed)\n"
			L"  " ARG_EXC_PATHS L" <mask_1...N>\t\t- list of paths' masks to skip from comparison (symbols (*) and (?) are allowed)\n"

			L"  " ARG_INC_DIRS L" <mask_1...N>\t\t- list of dirs' masks to compare (symbols (*) and (?) are allowed)\n"
			L"  " ARG_EXC_DIRS L" <mask_1...N>\t\t- list of dirs' masks to skip from comparison (symbols (*) and (?) are allowed)\n"

			L"  " ARG_PRINT_HASH L"\t\t\t\t- print files' SHA1 hash\n"
		L"\n"
	;

	return false;
}

int main ()
{
	::_wsetlocale (LC_CTYPE, L"");

	try
	{
		auto t_start = std::chrono::high_resolution_clock::now ();

		bool find_all_hashes = false;
		DirEnumHandler handler (Usage);
		DirEnumerator de (&handler);
		if (!ReadArg (de, find_all_hashes, Usage))
			return 0;

		de.EnumerateDirectory ();

		size_t found_count = 0, ready_count = 0;
		for (auto & f : handler._files)
		{
			found_count += f.second.size ();
			if (f.second.size () > 1)
				ready_count += f.second.size ();
		}
		std::wcout 
			<< found_count << L" files found. " 
			<< ready_count << L" of them ready to compare. "
			<< L"Start comparing...\n\n";

		std::list <ListOfFiles> equal;
		ListOfFiles failed;

		Comparer cmp (handler._files, find_all_hashes);
		cmp.FindEqualFiles (equal, failed, PrintEqualGroup);

		PrintEqualGroup ({});
		PrintFailedFiles (failed);

		auto t_end = std::chrono::high_resolution_clock::now ();
		auto t_elapsed = t_end - t_start;
		auto t_seconds = t_elapsed.count () / 1'000'000'000;
		std::cout << "Elapsed time: " << t_seconds << " seconds\n";
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
