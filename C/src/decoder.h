#pragma once

#include "error_codes.h"
#include "structures.h"

At9Status Decode(Atrac9Handle* handle, const unsigned char* audio, unsigned char* pcm, int* bytesUsed);
At9Status DecodeBounded(Atrac9Handle* handle, const unsigned char* audio, int audioSize, unsigned char* pcm, int* bytesUsed);
int GetCodecInfo(Atrac9Handle* handle, CodecInfo* pCodecInfo);
