
#include "pch.h"
#include "comparer.h"

Comparer::Comparer (std::map <uintmax_t, ListOfFiles> & files, bool find_all_hashes) :
	m_find_all_hashes (find_all_hashes),
	m_files (files)
{
}

void Comparer::BinaryCompare (File & f1, File & f2, std::promise <Res> p)
{
	Res res;
	if (f1.CompareTo (f2))
	{
		res.equal.push_back (std::move (f1));
		res.equal.push_back (std::move (f2));
	}
	else
	{
		if (f1.Failed ())
			res.failed.push_back (std::move (f1));
		if (f2.Failed ())
			res.failed.push_back (std::move (f2));
	}

	p.set_value (std::move (res));
}

void Comparer::HashCompare (ListOfFiles & files, std::promise <Res> p)
{
	auto Hashing = [](File * pfile)
	{
		pfile->CalcHash ();
	};

	std::list <std::pair <std::future <void>, File*>> hashed;

	for (auto & obj : files)
	{
		auto f = std::async (Hashing, &obj);
		hashed.push_back (std::make_pair <std::future <void>, File*> (std::move (f), &obj));
	}

	Res res;
	std::map <std::wstring, ListOfFiles> hashes;
	for (auto & hash : hashed)
	{
		hash.first.wait ();
		if (!hash.second->Failed ())
			hashes[hash.second->Hash ()].push_back (std::move (*hash.second));
		else
			res.failed.push_back (std::move (*hash.second));
	}

	for (auto & hash : hashes)
	{
		if (hash.second.size () > 1)
		{
			res.equal = std::move (hash.second);
		}
	}

	p.set_value (std::move (res));
}

void Comparer::FindEqualFiles (std::list <ListOfFiles> & equal, ListOfFiles & failed, std::function <void (const ListOfFiles &)> equal_callback)
{
	std::list <std::future <Res>> futures;

	for (auto & file : m_files)
	{
		// compare only files with equal file sizes
		if (file.second.size () == 1)
			continue;

		// files with zero-length size are always equal. Do not calc hash
		if (0 == file.first)
		{
			equal.push_back (std::move (file.second));
			continue;
		}

		std::promise <Res> p;
		futures.push_back (p.get_future ());

		// if there are only 2 files with same size, just open them and compare byte to byte
		if (file.second.size () == 2 && !m_find_all_hashes)
		{
			auto & first = file.second.front ();
			auto & last = file.second.back ();
			std::async (&Comparer::BinaryCompare, this, std::ref (first), std::ref (last), std::move (p));
		}

		// if there are more than 2 files, binary comparing may be too expensive, so
		// calc file' hash and compare hashes
		else
		{
			std::async (&Comparer::HashCompare, this, std::ref (file.second), std::move (p));
		}
	}

	for (auto & fut : futures)
	{
		fut.wait ();
		Res res = fut.get ();
		if (!res.equal.empty ())
		{
			equal_callback (res.equal);
			equal.push_back (std::move (res.equal));
		}
		if (!res.failed.empty ())
			failed.splice (failed.end (), res.failed);
	}
}


