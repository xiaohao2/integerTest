#ifndef _UD_INTEGER_
#define _UD_INTEGER_

#define w64

#include <iostream>
#include <fstream>
#include <exception>
#include <string>
#include <Windows.h>

#pragma comment(lib,"winmm.lib")

const int MINNUMSIZE = 1;
bool flag = true;
bool flag1 = false;

#ifdef w64

#define UD_UINT unsigned long long
const UD_UINT MAX_UD_INT = 0x7FFFFFFFFFFFFFFF;
const UD_UINT MIN_UD_INT = 0x8000000000000000;
const UD_UINT MAX_UD_UINT = 0xFFFFFFFFFFFFFFFF;

const int BUFFERSIZE = 19;
const UD_UINT MAX_UDEC = 10000000000000000000;

#else

#define UD_UINT unsigned int
const UD_UINT MAX_UD_INT = 0x7FFFFFFF;
const UD_UINT MIN_UD_INT = 0x80000000;
const UD_UINT MAX_UD_UINT = 0xFFFFFFFF;

const int BUFFERSIZE = 9;
const UD_UINT MAX_UDEC = 1000000000;

#endif

class ud_integer_block{

private:
	UD_UINT * data;
	int size;
	ud_integer_block * prev;
	ud_integer_block * next;

public:

	ud_integer_block();
	~ud_integer_block();
	ud_integer_block(int size) throw(const std::bad_alloc &);

	inline int getSize();
	inline UD_UINT * getData();
	inline ud_integer_block * getPrev();
	inline void setPrev(ud_integer_block * prev);
	inline ud_integer_block * getNext();
	inline void setNext(ud_integer_block * next);

	friend void allocateBlockOP(int size, ud_integer_block ** firstBlock, ud_integer_block ** currentLastBlock) throw(const std::bad_alloc &);
	friend void allocateBlock(int size, ud_integer_block ** firstBlock, ud_integer_block ** lastBlock) throw(const std::bad_alloc &);
};

class ud_integer{

public:
	int * refCount;
	int dataSize;
	int memorySize;
	int sign;
	UD_UINT * lastData;
	ud_integer_block * firstBlock;
	ud_integer_block * lastBlock;

	char buffer[BUFFERSIZE];

	void expandMemorySize(int size) throw(const std::bad_alloc &);
	void minimizeNumberSize();
	void minimizeMemorySize();
	void addPP(ud_integer & rp);
	void subPP(ud_integer & rp);
	/*
	void initIntegerOP(int size, ud_integer_block ** currentLastBlock) throw(const std::bad_alloc &);
	void initInteger(int size) throw(const std::bad_alloc &);
	*/

public:
	ud_integer();
	ud_integer(const ud_integer & rp);
	~ud_integer();

	friend int sign(ud_integer * lp);

	friend bool operator <(ud_integer & lp, ud_integer & rp);
	friend bool operator ==(ud_integer & lp, ud_integer & rp);
	friend bool operator >(ud_integer & lp, ud_integer & rp);

	friend ud_integer operator -(ud_integer & rp);
	
	ud_integer & operator +=(ud_integer & rp);
	ud_integer & operator -=(ud_integer & rp);
	ud_integer & operator *=(ud_integer & rp);
	ud_integer & operator /=(ud_integer & rp);

	friend ud_integer operator +(ud_integer & lp, ud_integer & rp);
	friend ud_integer operator -(ud_integer & lp, ud_integer & rp);
	friend ud_integer operator *(ud_integer & lp, ud_integer & rp);
	friend ud_integer operator /(ud_integer & lp, ud_integer & rp);

	friend void multiply(ud_integer & lp, ud_integer & rp, ud_integer & integerRet);

	friend std::ofstream & operator <<(std::ofstream & os, ud_integer & rp);
	friend std::ifstream & operator >>(std::ifstream & is, ud_integer & rp);

	ud_integer & operator =(ud_integer & rp);

	void copyFrom(ud_integer & rp);
	void destroy();
};

#endif