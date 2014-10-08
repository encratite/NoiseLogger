#pragma once

#include <Fall/Exception.hpp>

#include <Common/Serialization.hpp>

void lzmaCompress(const ByteBuffer & input, ByteBuffer & output, uint32_t compressionLevel);