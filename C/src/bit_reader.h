#pragma once

typedef struct {
	const unsigned char * Buffer;
	int Position;
	int SizeBytes; // < 0: unbounded, otherwise reads past this many bytes return zero bits
} BitReaderCxt;

// Make MSVC compiler happy. Leave const in for value parameters

void InitBitReaderCxt(BitReaderCxt* br, const void * buffer);
void InitBitReaderCxtBounded(BitReaderCxt* br, const void * buffer, int sizeBytes);
int BitReaderOverrun(const BitReaderCxt* br);
int PeekInt(BitReaderCxt* br, const int bits);
int ReadInt(BitReaderCxt* br, const int bits);
int ReadSignedInt(BitReaderCxt* br, const int bits);
int ReadOffsetBinary(BitReaderCxt* br, const int bits);
void AlignPosition(BitReaderCxt* br, const unsigned int multiple);
