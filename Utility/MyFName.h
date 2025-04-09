/**
* @file MyFName.h
*/
#ifndef MYFNAME_H_INCLUDED
#define MYFNAME_H_INCLUDED
#include <iostream>
#include <string>

/**
* string��hash�ϊ����Ǘ�����
*/
class MyFName
{
private:
	std::string name;	// string��
	size_t hash = 0;	// string���̃n�b�V���ϊ��l

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
	// string����n�b�V���l�𐶐�
	inline void CreateHash(const std::string& name) {
		std::hash<std::string> hash_fn;
		hash = hash_fn(this->name);
	}

public:
	// ��r���Z�q
	bool operator==(const MyFName& other) const {
		return hash == other.hash;
	}
	bool operator!=(const MyFName& other) const {
		return hash != other.hash;
	}

	// string�𖼎擾����
	inline const std::string& GetName() const {
		return name;
	}
	// �n�b�V���l���擾����
	inline const size_t& GetHash() const {
		return hash;
	}
};

/**
* std::hash�̓��ꉻ
* (�W���R���e�i�ŃL�[�Ƃ��Ďg�p�����ꍇ�ɕK�v)
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