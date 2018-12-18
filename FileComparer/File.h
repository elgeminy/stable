#pragma once

using FileHandle = std::unique_ptr <void, decltype (&CloseHandle)>;
using ViewHandle = std::unique_ptr <unsigned char, decltype (&UnmapViewOfFile)>;

class File
{
	std::filesystem::path m_filepath;
	uintmax_t m_size = 0;

	std::wstring m_hash;
	DWORD m_error = NO_ERROR;
	bool m_filtering_result = true;

	ViewHandle m_pmap;
	FileHandle m_hmap;
	FileHandle m_hfile;

	std::vector <unsigned char> m_buffer;

public:
	static uintmax_t m_lMaxHeapSize;

	File (std::filesystem::path && path);
	File (std::filesystem::path && path, uintmax_t size);
	~File ();

	File (const File &) = delete;
	File & operator = (const File &) = delete;

	File (File &&) noexcept = default;
	File & operator = (File &&) noexcept = default;

	inline const wchar_t * Path () const noexcept
	{
		return m_filepath.c_str ();
	}

	inline std::wstring Ext () const noexcept
	{
		return m_filepath.extension ().wstring ();
	}

	inline uintmax_t Size () const noexcept
	{
		return m_size;
	}

	std::wstring SizeFormatted () const noexcept;

	inline const std::wstring & Hash () const noexcept
	{
		return m_hash;
	}

	inline const bool Failed () const noexcept
	{
		return m_error != NO_ERROR;
	}

	inline const DWORD Error () const noexcept
	{
		return m_error;
	}

	inline const bool FilteringResult () const noexcept
	{
		return m_filtering_result;
	}

	bool CalcHash () noexcept;
	bool CompareTo (File & obj) noexcept;
	bool MatchFilter (const std::list <std::wstring> & hashes, const std::basic_string <unsigned char> & content) noexcept;

private:
	bool OpenFile () noexcept;
	void CloseFile () noexcept;
	const unsigned char * FilePtr () const noexcept;
};

using ListOfFiles = std::list <File>;