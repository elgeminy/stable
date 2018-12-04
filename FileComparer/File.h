#pragma once

using FileHandle = std::unique_ptr <void, decltype (&CloseHandle)>;
using ViewHandle = std::unique_ptr <unsigned char, decltype (&UnmapViewOfFile)>;

class File
{
	std::filesystem::path m_filepath;
	uintmax_t m_size = 0;

	std::string m_hash;
	DWORD m_error = NO_ERROR;

	ViewHandle m_pmap;
	FileHandle m_hmap;
	FileHandle m_hfile;

	std::vector <unsigned char> m_buffer;

public:
	static uintmax_t m_lMaxHeapSize;

	File (std::filesystem::path && path);
	~File ();

	File (const File &) = delete;
	File & operator = (const File &) = delete;

	File (File &&) noexcept = default;
	File & operator = (File &&) noexcept = default;

	inline const wchar_t * Path () const noexcept
	{
		return m_filepath.c_str ();
	}

	inline uintmax_t Size () const noexcept
	{
		return m_size;
	}

	inline const std::string & Hash () const noexcept
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

	bool CalcHash () noexcept;
	bool CompareTo (File & obj) noexcept;

private:
	bool OpenFile () noexcept;
	void CloseFile () noexcept;
	const unsigned char * FilePtr () const noexcept;
};

using ListOfFiles = std::list <File>;