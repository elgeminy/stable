
#pragma once

class Sha1
{
public:
	Sha1 ();

	void ComputeHash (const unsigned char * data, uintmax_t size) noexcept;

	size_t GetDigestSize () const noexcept { return m_lDigestSize; }
	void GetDigest (unsigned char * buffer) const noexcept;

	std::string GetReport () const noexcept;

private:
	void Update (const unsigned char * data, uintmax_t size) noexcept;
	void Finalize () noexcept;
	void Reset () noexcept;

	void ProcessMessageBlock () noexcept;
	void PadMessage () noexcept;
	char HexDigit (unsigned char b) const noexcept;

	inline size_t DoCircularShift (int bits, size_t word) const noexcept
	{
		return ((word << bits) & 0xFFFFFFFF) | ((word & 0xFFFFFFFF) >> (32-bits));
	}

private:
	bool m_bIsFinal = false;
	const size_t m_lDigestSize = 20;

	unsigned int m_digest[5] = {};			// Message digest buffers

	size_t m_lBitsLow = 0;					// Message length in bits
	size_t m_lBitsHigh = 0;					// Message length in bits

	unsigned char m_pMessageBlock[64] = {};	// 512-bit message blocks
	int m_lMessageBlock = 0;				// Index into message block array
};
