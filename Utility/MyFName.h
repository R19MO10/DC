/**
* @file MyFName.h
*/
#ifndef MYFNAME_H_INCLUDED
#define MYFNAME_H_INCLUDED
#include <iostream>
#include <string>

/**
* stringをhash変換し管理する
*/
class MyFName
{
private:
	std::string name;	// string名
	size_t hash = 0;	// string名のハッシュ変換値

public:
	MyFName() = default;
	MyFName(const std::string& name) 
		:name(name)
	{
		CreateHash(name);
	}
	MyFName(const char* name) 
		:name(name)
	{
		CreateHash(name);
	}

private:
	// stringからハッシュ値を生成
	inline void CreateHash(const std::string& name) {
		std::hash<std::string> hash_fn;
		hash = hash_fn(this->name);
	}

public:
	// 比較演算子
	bool operator==(const MyFName& other) const {
		return hash == other.hash;
	}
	bool operator!=(const MyFName& other) const {
		return hash != other.hash;
	}

	// stringを名取得する
	inline const std::string& GetName() const {
		return name;
	}
	// ハッシュ値を取得する
	inline const size_t& GetHash() const {
		return hash;
	}
};

/**
* std::hashの特殊化
* (標準コンテナでキーとして使用される場合に必要)
*/
template<>
struct std::hash<MyFName>
{
	size_t operator()(const MyFName& fname) const
	{
		return fname.GetHash();
	}
};

#endif // MYFNAME_H_INCLUDED