#include "bit_reader.h"
#include "utility.h"

static int PeekIntFallback(BitReaderCxt* br, int bitCount);
static int PeekIntBounded(BitReaderCxt* br, int bitCount);

void InitBitReaderCxt(BitReaderCxt* br, const void * buffer)
{
	br->Buffer = buffer;
	br->Position = 0;
	br->SizeBytes = -1;
}

void InitBitReaderCxtBounded(BitReaderCxt* br, const void * buffer, int sizeBytes)
{
	br->Buffer = buffer;
	br->Position = 0;
	br->SizeBytes = sizeBytes < 0 ? 0 : sizeBytes;
}

// True once the reader has been advanced past the end of a bounded buffer
int BitReaderOverrun(const BitReaderCxt* br)
{
	return br->SizeBytes >= 0 && br->Position > br->SizeBytes * 8;
}

int ReadInt(BitReaderCxt* br, const int bits)
{
	const int value = PeekInt(br, bits);
	br->Position += bits;
	return value;
}

int ReadSignedInt(BitReaderCxt* br, const int bits)
{
	const int value = PeekInt(br, bits);
	br->Position += bits;
	return SignExtend32(value, bits);
}

int ReadOffsetBinary(BitReaderCxt* br, const int bits)
{
	const int offset = 1 << (bits - 1);
	const int value = PeekInt(br, bits) - offset;
	br->Position += bits;
	return value;
}

int PeekInt(BitReaderCxt* br, const int bits)
{
	// The fast paths below read up to 4 bytes at once. Near (or past) the end of a
	// bounded buffer use the slow path, which returns zeros instead of reading out of bounds.
	if (br->SizeBytes >= 0 && br->Position / 8 + 4 > br->SizeBytes)
	{
		return PeekIntBounded(br, bits);
	}

	const int byteIndex = br->Position / 8;
	const int bitIndex = br->Position % 8;
	const unsigned char* buffer = br->Buffer;

	if (bits <= 9)
	{
		int value = buffer[byteIndex] << 8 | buffer[byteIndex + 1];
		value &= 0xFFFF >> bitIndex;
		value >>= 16 - bits - bitIndex;
		return value;
	}

	if (bits <= 17)
	{
		int value = buffer[byteIndex] << 16 | buffer[byteIndex + 1] << 8 | buffer[byteIndex + 2];
		value &= 0xFFFFFF >> bitIndex;
		value >>= 24 - bits - bitIndex;
		return value;
	}

	if (bits <= 25)
	{
		int value = buffer[byteIndex] << 24
			| buffer[byteIndex + 1] << 16
			| buffer[byteIndex + 2] << 8
			| buffer[byteIndex + 3];

		value &= (int)(0xFFFFFFFF >> bitIndex);
		value >>= 32 - bits - bitIndex;
		return value;
	}
	return PeekIntFallback(br, bits);
}

void AlignPosition(BitReaderCxt* br, const unsigned int multiple)
{
	const int position = br->Position;
	if (position % multiple == 0)
	{
		return;
	}

	br->Position = position + multiple - position % multiple;
}

static int PeekIntFallback(BitReaderCxt* br, int bitCount)
{
	int value = 0;
	int byteIndex = br->Position / 8;
	int bitIndex = br->Position % 8;
	const unsigned char* buffer = br->Buffer;

	while (bitCount > 0)
	{
		if (bitIndex >= 8)
		{
			bitIndex = 0;
			byteIndex++;
		}

		int bitsToRead = bitCount;
		if (bitsToRead > 8 - bitIndex)
		{
			bitsToRead = 8 - bitIndex;
		}

		const int mask = 0xFF >> bitIndex;
		const int currentByte = (mask & buffer[byteIndex]) >> (8 - bitIndex - bitsToRead);

		value = (value << bitsToRead) | currentByte;
		bitIndex += bitsToRead;
		bitCount -= bitsToRead;
	}
	return value;
}

// Same result as the fast paths, but any byte at or after SizeBytes reads as zero
static int PeekIntBounded(BitReaderCxt* br, int bitCount)
{
	unsigned int value = 0;
	int position = br->Position;

	while (bitCount > 0)
	{
		const int byteIndex = position / 8;
		const int bitIndex = position % 8;
		const int available = 8 - bitIndex;
		const int bitsToRead = bitCount < available ? bitCount : available;

		const int currentByte = byteIndex < br->SizeBytes ? br->Buffer[byteIndex] : 0;
		const unsigned int bits = ((unsigned int)currentByte & (0xFFu >> bitIndex)) >> (available - bitsToRead);

		value = (value << bitsToRead) | bits;
		position += bitsToRead;
		bitCount -= bitsToRead;
	}
	return (int)value;
}
