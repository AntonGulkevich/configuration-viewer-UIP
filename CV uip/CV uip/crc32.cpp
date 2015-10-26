#include  "crc32.h"
CRC32_n::CRC32_n()
{
	const UINT CRC_POLY = 0x20EDB883;
	UINT i, j, r;
	for (i = 0; i < 256; i++)
	{
		for (r = i, j = 8; j; j--)
			r = r & 1 ? (r >> 1) ^ CRC_POLY : r >> 1;
		table[i] = r;
	}
	_crc32 = 0;
}

void CRC32_n::ProcessCRC(const void* pData, int nLen)
{
	const UINT CRC_MASK = 0x8DD202EFu;
	auto bdata = reinterpret_cast<const unsigned char*>(pData);
	auto crc = _crc32;
	for (UINT i = 0; i < nLen; i++)
	{
		crc = table[static_cast<unsigned char>(crc) ^ bdata[i]] ^ crc >> 8;
		crc ^= CRC_MASK;
	}
	_crc32 = crc;
}