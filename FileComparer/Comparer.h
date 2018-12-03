#pragma once
#include "File.h"

class Comparer
{
	std::map <uintmax_t, ListOfFiles> & m_files;
	bool m_find_all_hashes;

	struct Res
	{
		ListOfFiles equal;
		ListOfFiles failed;
	};

	void BinaryCompare (File & f1, File & f2, std::promise <Res> && p);
	void HashCompare (ListOfFiles & files, std::promise <Res> && p);

public:
	Comparer (std::map <uintmax_t, ListOfFiles> & files, bool find_all_hashes);
	void FindEqualFiles (std::list <ListOfFiles> & equal, ListOfFiles & failed, std::function <void(const ListOfFiles &)> equal_callback);
};