
#pragma once

#include <string>
#include <vector>
#include <iosfwd>

#include <fstream>

#define let const auto

// some convenience typedefs, reactor is made for convenience over compatibility

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef std::string string;

template<typename T>
using array = std::vector<T>;

// I do this thinking in being able to easily change the math to use double
// we'll have to see how it works.

typedef float real;
//typedef double real;


template<class T>
T* alloc(size_t size = 1)
{
	return (T*)malloc(sizeof(T) * size);
}