/**
* @file Range.h
*/
#ifndef RANGE_H_INCLUDED
#define RANGE_H_INCLUDED

template<typename T>
class Range
{
public:
	T min, max;

public:
	constexpr Range()
		: min(0), max(0) 
	{}
	constexpr Range(T min, T max)
		: min(min), max(max) 
	{}
	constexpr Range(const Range<T>& v)
		: min(v.min), max(v.max) 
	{}
};

#endif // RANGE_H_INCLUDED