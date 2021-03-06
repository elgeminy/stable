
#include "pch.h"
#include "dirfinder.h"

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
			L"  [<dir_1> ... <dir_N>] [" ARG_SCAN_PATH L"]\n" 
				L"\t[" ARG_INC_PATHS L"] [" ARG_EXC_PATHS L"] [" ARG_EXC_DIRS L"]\n"				
			L"\n"

			L"  <file_1...N>\t\t\t- list of dirs's masks to search (symbols (*) and (?) are allowed).\t\t\t\t\t\t\t\tOmit to find all files (e.g. '*')\n"

			L"  " ARG_SCAN_PATH L" <dir_path>\t\t- path to a directory to search in. Currect dir used by default\n"

			L"  " ARG_INC_PATHS L" <mask_1...N>\t\t- list of path' masks to search in (symbols (*) and (?) are allowed)\n"
			L"  " ARG_EXC_PATHS L" <mask_1...N>\t\t- list of paths' masks to skip from searching (symbols (*) and (?) are allowed)\n"

			L"  " ARG_EXC_DIRS L" <mask_1...N>\t\t- list of dirs' masks to skip from searching (symbols (*) and (?) are allowed)\n"
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

		auto PrintDir = [](const std::filesystem::path & dir)
		{
			wprintf (L"%s\n", dir.c_str ());
		};

		DirEnumHandler handler (PrintDir, Usage);
		DirEnumerator de (&handler);
		bool hash = false, verbose = false;
		if (!ReadArg (de, Usage))
			return 0;

		de.EnumerateDirectory ();

		std::wcout << L"\n" << handler._dirs.size () << L" directories found\n";

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
