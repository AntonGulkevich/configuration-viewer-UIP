#ifndef CRC32_H
#define CRC32_H  
// Класс подсчитывающий контрольную сумму по алгоритму CRC32
#define UINT unsigned int

class CRC32_n
{
protected:
	UINT table[256];
	UINT _crc32;
public:
	CRC32_n();
	virtual ~CRC32_n() {}
	void ProcessCRC(const void* pData, int nLen);
	UINT GetCRC32() const { return _crc32; }
	void Reset() { _crc32 = 0; }
};
#endif
