
#include "pch.h"
#include "sha1.h"

Sha1::Sha1 ()
{
	Reset ();
}

void Sha1::ComputeHash (const unsigned char* data, uintmax_t size) noexcept
{
	Update (data, size);
	Finalize ();
}

char Sha1::HexDigit (unsigned char b) const noexcept
{
	if (b <= 9)
		return '0' + b;
	return 'A' + b - 0xa;
}

std::string Sha1::GetReport () const noexcept
{
	std::stringstream report;

	std::vector <unsigned char> digest (GetDigestSize ());
	GetDigest (&digest [0]);

	for (size_t i = 0; i < digest.size (); i++)
	{
		report << HexDigit (digest [i] >> 4);
		report << HexDigit (digest [i] & 0x0f);
	}

	return report.str ();
}

void Sha1::Update (const unsigned char * data, uintmax_t size) noexcept
{
	if (nullptr == data)
		return;

	if (m_bIsFinal)
		return;

	bool corrupted = false;

	while (size-- && !corrupted)
	{
		m_pMessageBlock [m_lMessageBlock++] = (*data & 0xFF);

		m_lBitsLow += 8;
		m_lBitsLow &= 0xFFFFFFFF;				// Force it to 32 bits
		
		if (0 == m_lBitsLow)
		{
			m_lBitsHigh++;
			m_lBitsHigh &= 0xFFFFFFFF;			// Force it to 32 bits
			if (0 == m_lBitsHigh)
				corrupted = true;				// Message is too long
		}

		if (64 == m_lMessageBlock)
			ProcessMessageBlock ();

		data++;
	}
}

void Sha1::Finalize () noexcept
{
	PadMessage ();
	m_bIsFinal = true;
}

void Sha1::Reset () noexcept
{
	m_lBitsLow = 0;
	m_lBitsHigh = 0;
	m_lMessageBlock = 0;

	m_digest [0] = 0x67452301;
	m_digest [1] = 0xEFCDAB89;
	m_digest [2] = 0x98BADCFE;
	m_digest [3] = 0x10325476;
	m_digest [4] = 0xC3D2E1F0;

	m_bIsFinal = false;
}

void Sha1::GetDigest (unsigned char * buffer) const noexcept
{
	if (nullptr == buffer)
		return;

	if (!m_bIsFinal)
		return;

	const unsigned char * digest = reinterpret_cast <const unsigned char *> (m_digest);
	const size_t sizeOfInt = sizeof (m_digest [0]);

	for (size_t i = 0; i < m_lDigestSize; i += sizeOfInt)
	{
		for (size_t j = 0; j < sizeOfInt; j++)
		{
			buffer [i + j] = digest [i + sizeOfInt - 1 - j];
		}
	}
}

void Sha1::ProcessMessageBlock () noexcept
{
	// Constants defined for SHA-1
	const size_t K [] =
	{
		0x5A827999,
		0x6ED9EBA1,
		0x8F1BBCDC,
		0xCA62C1D6
	};

	size_t W[80] = {};		// Word sequence	

	for (size_t t = 0; t < 16; t++)
	{
		W [t] = ((size_t) m_pMessageBlock [t * 4]) << 24;
		W [t] |= ((size_t) m_pMessageBlock [t * 4 + 1]) << 16;
		W [t] |= ((size_t) m_pMessageBlock [t * 4 + 2]) << 8;
		W [t] |= ((size_t) m_pMessageBlock [t * 4 + 3]);
	}

	for (size_t t = 16; t < 80; t++)
	{
		W [t] = DoCircularShift (1, W [t-3] ^ W [t-8] ^ W [t-14] ^ W [t-16]);
	}

	// Word buffers
	size_t A = m_digest [0];
	size_t B = m_digest [1];
	size_t C = m_digest [2];
	size_t D = m_digest [3];
	size_t E = m_digest [4];

	size_t temp = 0;
	for (size_t t = 0; t < 20; t++)
	{
		temp = DoCircularShift (5, A) + ((B & C) | ((~B) & D)) + E + W [t] + K [0];
		temp &= 0xFFFFFFFF;
		E = D;
		D = C;
		C = DoCircularShift (30, B);
		B = A;
		A = temp;
	}

	for (size_t t = 20; t < 40; t++)
	{
		temp = DoCircularShift (5, A) + (B ^ C ^ D) + E + W [t] + K [1];
		temp &= 0xFFFFFFFF;
		E = D;
		D = C;
		C = DoCircularShift (30, B);
		B = A;
		A = temp;
	}

	for (size_t t = 40; t < 60; t++)
	{
		temp = DoCircularShift (5, A) + ((B & C) | (B & D) | (C & D)) + E + W [t] + K [2];
		temp &= 0xFFFFFFFF;
		E = D;
		D = C;
		C = DoCircularShift (30, B);
		B = A;
		A = temp;
	}

	for (size_t t = 60; t < 80; t++)
	{
		temp = DoCircularShift (5, A) + (B ^ C ^ D) + E + W [t] + K [3];
		temp &= 0xFFFFFFFF;
		E = D;
		D = C;
		C = DoCircularShift (30, B);
		B = A;
		A = temp;
	}

	m_digest [0] = (m_digest [0] + A) & 0xFFFFFFFF;
	m_digest [1] = (m_digest [1] + B) & 0xFFFFFFFF;
	m_digest [2] = (m_digest [2] + C) & 0xFFFFFFFF;
	m_digest [3] = (m_digest [3] + D) & 0xFFFFFFFF;
	m_digest [4] = (m_digest [4] + E) & 0xFFFFFFFF;

	m_lMessageBlock = 0;
}

void Sha1::PadMessage () noexcept
{
	/*
	*	Check to see if the current message block is too small to hold
	*	the initial padding bits and length.  If so, we will pad the
	*	block, process it, and then continue padding into a second block.
	*/
	if (m_lMessageBlock > 55)
	{
		m_pMessageBlock [m_lMessageBlock++] = 0x80;
		while (m_lMessageBlock < 64)
		{
			m_pMessageBlock [m_lMessageBlock++] = 0;
		}

		ProcessMessageBlock ();

		while (m_lMessageBlock < 56)
		{
			m_pMessageBlock [m_lMessageBlock++] = 0;
		}
	}
	else
	{
		m_pMessageBlock [m_lMessageBlock++] = 0x80;
		while (m_lMessageBlock < 56)
		{
			m_pMessageBlock [m_lMessageBlock++] = 0;
		}

	}

	/*
	*	Store the message length as the last 8 octets
	*/
	m_pMessageBlock [56] = static_cast <unsigned char> ((m_lBitsHigh >> 24) & 0xFF);
	m_pMessageBlock [57] = static_cast <unsigned char> ((m_lBitsHigh >> 16) & 0xFF);
	m_pMessageBlock [58] = static_cast <unsigned char> ((m_lBitsHigh >> 8) & 0xFF);
	m_pMessageBlock [59] = static_cast <unsigned char> ((m_lBitsHigh) & 0xFF);
	m_pMessageBlock [60] = static_cast <unsigned char> ((m_lBitsLow >> 24) & 0xFF);
	m_pMessageBlock [61] = static_cast <unsigned char> ((m_lBitsLow >> 16) & 0xFF);
	m_pMessageBlock [62] = static_cast <unsigned char> ((m_lBitsLow >> 8) & 0xFF);
	m_pMessageBlock [63] = static_cast <unsigned char> ((m_lBitsLow) & 0xFF);

	ProcessMessageBlock ();
}
