#include "ud_integer_4.h"

ud_integer_block::ud_integer_block(){
	this->data = NULL;
	this->size = 0;
	this->prev = NULL;
	this->next = NULL;
}

ud_integer_block::ud_integer_block(int size) throw(const std::bad_alloc &){
	this->prev = NULL;
	this->next = NULL;
	if (size <= 0){
		this->data = NULL;
		this->size = 0;
		return;
	}
	try{
		this->data = new UD_UINT[size];
	}
	catch (const std::bad_alloc & e){
		this->data = NULL;
		this->size = 0;
		throw;
	}
	this->size = size;
}

ud_integer_block::~ud_integer_block(){
	if (this->data != NULL){
		delete[] this->data;
	}
}

inline int ud_integer_block::getSize(){
	return size;
}

inline UD_UINT * ud_integer_block::getData(){
	return data;
}

inline ud_integer_block * ud_integer_block::getPrev(){
	return prev;
}

inline void ud_integer_block::setPrev(ud_integer_block * prev){
	this->prev = prev;
}

inline ud_integer_block * ud_integer_block::getNext(){
	return next;
}

inline void ud_integer_block::setNext(ud_integer_block * next){
	this->next = next;
}

void allocateBlockOP(int size, ud_integer_block ** firstBlock, ud_integer_block ** currentLastBlock) throw(const std::bad_alloc &){
	ud_integer_block * blockTemp;
	try{
		blockTemp = new ud_integer_block(size);
	}
	catch (const std::bad_alloc & e){
		if (size <= 1){
			throw;
		}
		try{
			allocateBlockOP((size + 1) / 2, firstBlock, currentLastBlock);
			allocateBlockOP(size / 2, firstBlock, currentLastBlock);
		}
		catch (const std::bad_alloc & e){
			throw;
		}
	}
	if (*currentLastBlock == NULL){
		*currentLastBlock = blockTemp;
		*firstBlock = *currentLastBlock;
	}
	else{
		(*currentLastBlock)->next = blockTemp;
		blockTemp->prev = *currentLastBlock;
		currentLastBlock = &blockTemp;
	}
}

void allocateBlock(int size, ud_integer_block ** firstBlock, ud_integer_block ** lastBlock) throw(const std::bad_alloc &){
	ud_integer_block * currentLastBlock;
	if (firstBlock != NULL && lastBlock != NULL && size > 0){
		*firstBlock = NULL;
		*lastBlock = NULL;
		currentLastBlock = NULL;
		try{
			allocateBlockOP(size, firstBlock, &currentLastBlock);
		}
		catch (const std::bad_alloc & e){
			while (*firstBlock != NULL){
				*lastBlock = (*firstBlock)->next;
				delete *firstBlock;
				*firstBlock = *lastBlock;
			}
			*firstBlock = NULL;
			*lastBlock = NULL;
			throw;
		}
		*lastBlock = currentLastBlock;
	}
}

void ud_integer::expandMemorySize(int size) throw(const std::bad_alloc &){
	ud_integer_block * firstBlockTemp, *lastBlockTemp;
	if (size <= 0){
		return;
	}
	try{
		allocateBlock(size, &firstBlockTemp, &lastBlockTemp);
	}
	catch (const std::bad_alloc & e){
		throw;
	}
	if (this->memorySize == 0){
		firstBlockTemp->setPrev(lastBlockTemp);
		lastBlockTemp->setNext(firstBlockTemp);
		this->firstBlock = firstBlockTemp;
		this->refCount = new int;
		*(this->refCount) = 1;
	}
	else{
		this->firstBlock->getPrev()->setNext(firstBlockTemp);
		firstBlockTemp->setPrev(this->firstBlock->getPrev());
		this->firstBlock->setPrev(lastBlockTemp);
		lastBlockTemp->setNext(this->firstBlock);
	}
	this->memorySize += size;
}

void ud_integer::minimizeNumberSize(){
	UD_UINT *currentLastDataTemp;
	int count = 0;
	if (lastBlock == NULL){
		return;
	}
	currentLastDataTemp = lastBlock->getData();
	while (true){
		if (lastData < currentLastDataTemp){
			if (lastBlock == this->firstBlock){
				lastData = currentLastDataTemp;
				this->sign = 0;
				count--;
				break;
			}
			lastBlock = lastBlock->getPrev();
			lastData = lastBlock->getData() + lastBlock->getSize() - 1;
			currentLastDataTemp = lastBlock->getData();
		}
		if (*lastData == 0){
			lastData--;
			count++;
		}
		else{
			break;
		}
	}
	this->dataSize -= count;
}

void ud_integer::minimizeMemorySize(){
	ud_integer_block * blockTemp, *blockTemp1;
	UD_UINT * dataTemp, *dataTemp1, *currentLastDataTemp;
	int count = 0;
	if (this->memorySize = 0){
		return;
	}
	if (this->lastBlock == NULL){
		blockTemp = this->firstBlock;
		while (true){
			blockTemp1 = blockTemp->getPrev();
			delete blockTemp;
			blockTemp = blockTemp1;
			if (blockTemp == this->firstBlock){
				break;
			}
		}
		delete this->refCount;
		this->memorySize = 0;
		this->firstBlock = NULL;
		return;
	}

	this->minimizeNumberSize();

	blockTemp = this->firstBlock->getPrev();
	while (blockTemp != lastBlock){
		blockTemp1 = blockTemp->getPrev();
		count += blockTemp->getSize();
		delete blockTemp;
		blockTemp = blockTemp1;
	}
	if (lastData < lastBlock->getData() + lastBlock->getSize() - 1){
		try{
			allocateBlock(lastData - lastBlock->getData() + 1, &blockTemp, &blockTemp1);
		}
		catch (const std::bad_alloc & e){
			lastBlock->setNext(this->firstBlock);
			this->firstBlock->setPrev(lastBlock);
			this->memorySize -= count;
			return;
		}
		if (lastBlock == this->firstBlock){
			blockTemp->setPrev(blockTemp1);
			blockTemp1->setNext(blockTemp);
			this->firstBlock = blockTemp;
		}
		else{
			blockTemp->setPrev(lastBlock->getPrev());
			blockTemp1->setNext(this->firstBlock);
			lastBlock->getPrev()->setNext(blockTemp);
			this->firstBlock->setPrev(blockTemp1);
		}
		dataTemp1 = lastBlock->getData();
		dataTemp = blockTemp->getData();
		currentLastDataTemp = dataTemp + blockTemp->getSize() - 1;
		while (true){
			if (dataTemp1 > lastData){
				break;
			}
			if (dataTemp > currentLastDataTemp){
				blockTemp = blockTemp->getNext();
				dataTemp = blockTemp->getData();
				currentLastDataTemp = dataTemp + blockTemp->getSize() - 1;
			}
			*dataTemp = *dataTemp1;
		}
		delete lastBlock;
		count += lastBlock->getData() + lastBlock->getSize() - 1 - lastData;
		this->memorySize -= count;
		return;
	}
	else{
		lastBlock->setNext(this->firstBlock);
		this->firstBlock->setPrev(lastBlock);
		this->memorySize -= count;
		return;
	}
}


ud_integer::ud_integer(){
	this->dataSize = 0;
	this->memorySize = 0;
	this->firstBlock = NULL;
	this->lastBlock = NULL;
	this->lastData = NULL;
	this->refCount = NULL;
	this->sign = 2;
}

ud_integer::ud_integer(const ud_integer & rp){
	if (rp.dataSize == 0){
		this->dataSize = 0;
		this->memorySize = 0;
		this->firstBlock = NULL;
		this->lastBlock = NULL;
		this->lastData = NULL;
		this->refCount = NULL;
		this->sign = 2;
	}
	else{
		(*(rp.refCount))++;
		this->firstBlock = rp.firstBlock;
		this->lastBlock = rp.lastBlock;
		this->lastData = rp.lastData;
		this->memorySize = rp.memorySize;
		this->sign = rp.sign;
		this->dataSize = rp.dataSize;
		this->refCount = rp.refCount;
	}
}

ud_integer::~ud_integer(){
	ud_integer_block * blockTemp, *blockTemp1;
	if (this->refCount != NULL){
		if (*(this->refCount) > 1){
			(*(this->refCount))--;
		}
		else{
			blockTemp = this->firstBlock;
			while (true){
				blockTemp1 = blockTemp->getPrev();
				delete blockTemp;
				blockTemp = blockTemp1;
				if (blockTemp == this->firstBlock){
					break;
				}
			}
			delete this->refCount;
		}
	}
}

void ud_integer::destroy(){
	ud_integer_block * blockTemp, *blockTemp1;
	if (this->refCount != NULL){
		if (*(this->refCount) > 1){
			(*(this->refCount))--;
		}
		else{
			blockTemp = this->firstBlock;
			while (true){
				blockTemp1 = blockTemp->getPrev();
				delete blockTemp;
				blockTemp = blockTemp1;
				if (blockTemp == this->firstBlock){
					break;
				}
			}
			delete this->refCount;
		}
	}
	this->dataSize = 0;
	this->memorySize = 0;
	this->firstBlock = NULL;
	this->lastBlock = NULL;
	this->lastData = NULL;
	this->refCount = NULL;
	this->sign = 2;
}


std::ifstream & operator >>(std::ifstream & is, ud_integer & rp){
	rp.destroy();
	ud_integer_block * blockTemp, *blockTemp1, *blockTemp2;
	UD_UINT * dataTemp, *dataTemp1, *currentLastDataTemp, *lastDataTemp, overflowDataTemp, multiplier = MAX_UDEC, numTemp = 0;
	int isOffTemp = 0, signTemp = 1, currentBufferSize, count, sizeTemp = 0;
	bool numEndFlag;
	if (!is.is_open() || is.eof()){
		return is;
	}
	rp.buffer[0] = is.get();
	while (rp.buffer[0] == ' ' || rp.buffer[0] == '\t' || rp.buffer[0] == '\n'){
		rp.buffer[0] = is.get();
	}
	isOffTemp--;
	if (rp.buffer[0] == '+' || rp.buffer[0] == '-'){
		if (rp.buffer[0] == '-'){
			signTemp = -1;
		}
		if (!is.eof()){
			rp.buffer[0] = is.get();
			isOffTemp--;
		}
		else{
			is.seekg(isOffTemp, std::ios::cur);
			return is;
		}
	}
	if (rp.buffer[0] < '0' || rp.buffer[0] > '9'){
		is.seekg(isOffTemp, std::ios::cur);
		return is;
	}
	else if (rp.buffer[0] == '0'){
		try{
			rp.expandMemorySize(1);
			*(rp.firstBlock->getData()) = 0;
		}
		catch (const std::bad_alloc & e){
			is.seekg(isOffTemp, std::ios::cur);
			return is;
		}
		rp.lastBlock = rp.firstBlock;
		rp.lastData = rp.firstBlock->getData();
		rp.dataSize = 1;
		rp.sign = 0;
		return is;
	}
	else{
		is.seekg(-1, std::ios::cur);
		isOffTemp++;
	}
	try{
		rp.expandMemorySize(MINNUMSIZE);
		*(rp.firstBlock->getData()) = 0;
	}
	catch (const std::bad_alloc & e){
		is.seekg(isOffTemp, std::ios::cur);
		return is;
	}
	lastDataTemp = rp.firstBlock->getData();
	sizeTemp = 1;
	numEndFlag = false;
	while (!is.eof()){
		if (numEndFlag){
			break;
		}
		is.read(rp.buffer, BUFFERSIZE);
		currentBufferSize = is.gcount();
		isOffTemp -= currentBufferSize;
		numTemp = 0;
		if (currentBufferSize == BUFFERSIZE){
			multiplier = MAX_UDEC;
		}
		else{
			multiplier = 1;
			for (count = 0; count < currentBufferSize; count++){
				multiplier *= 10;
			}
		}
		for (count = 0; count < currentBufferSize; count++){
			if (rp.buffer[count] < '0' || rp.buffer[count] > '9'){
				if (count == 0){
					goto mark1;
				}
				multiplier = 1;
				for (int count1 = 0; count1 < count; count1++){
					multiplier *= 10;
				}
				numEndFlag = true;
			}
			numTemp = numTemp * 10 + (rp.buffer[count] - '0');
		}
		blockTemp = rp.firstBlock;
		dataTemp = blockTemp->getData();
		currentLastDataTemp = dataTemp + blockTemp->getSize();
		overflowDataTemp = numTemp;
		while (true){
			if (dataTemp >= currentLastDataTemp){
				blockTemp = blockTemp->getNext();
				dataTemp = blockTemp->getData();
				currentLastDataTemp = dataTemp + blockTemp->getSize();
			}
#ifdef w64
			__asm{
				push rax;
				push rbx;
				push rdx;
				mov rbx, qword ptr[dataTemp];
				mov rax, qword ptr[rbx];
				mul multiplier;
				add rax, qword ptr[overflowDataTemp];
				jnc mark2;
				inc rdx;
			mark2:
				mov qword ptr[overflowDataTemp], rdx;
				mov qword ptr[rbx], rax;
				pop rdx;
				pop rbx;
				pop rax;
			}
#else
			__asm{
				push eax;
				push ebx;
				push edx;
				mov ebx, dword ptr[dataTemp];
				mov eax, dword ptr[ebx];
				mul multiplier;
				add eax, dword ptr[overflowDataTemp];
				jnc mark2;
				inc edx;
			mark2:
				mov dword ptr[overflowDataTemp], edx;
				mov dword ptr[ebx], eax;
				pop edx;
				pop ebx;
				pop eax;
			}
#endif
			if (dataTemp == lastDataTemp){
				break;
			}
			dataTemp++;
		}
		if (overflowDataTemp != 0){
			dataTemp++;
			if (dataTemp >= currentLastDataTemp){
				if (blockTemp->getNext() == rp.firstBlock){
					try{
						rp.expandMemorySize(rp.memorySize);
					}
					catch (const std::bad_alloc & e){
						rp.destroy();
						is.seekg(isOffTemp, std::ios::cur);
						return is;
					}
				}
				blockTemp = blockTemp->getNext();
				dataTemp = blockTemp->getData();
				currentLastDataTemp = dataTemp + blockTemp->getSize();
			}
			*dataTemp = overflowDataTemp;
			lastDataTemp = dataTemp;
			sizeTemp++;
		}
	}
mark1:
	rp.lastBlock = blockTemp;
	rp.lastData = lastDataTemp;
	rp.dataSize = sizeTemp;
	rp.sign = signTemp;

	is.seekg(count - currentBufferSize, std::ios::cur);
	return is;
}

std::ofstream & operator <<(std::ofstream & os, ud_integer & rp){
	if (!os.is_open() || rp.memorySize == 0){
		return os;
	}
	UD_UINT * lastDataTemp, *dataTemp, *currentLastDataTemp;
	UD_UINT remainder = 0, divisor = MAX_UDEC;
	char * charTemp, *currentLastCharTemp;
	ud_integer_block * blockTemp1, *blockTemp2, *blockTemp3, *lastBlockTemp;
	ud_integer integerTemp, decimalIntegerTemp;
	if (rp.sign == 0){
		os << '0';
		return os;
	}
	integerTemp.copyFrom(rp);
	if (integerTemp.dataSize == 0){
		return os;
	}
	
	try{
		decimalIntegerTemp.expandMemorySize(MINNUMSIZE);
	}
	catch (const std::bad_alloc & e){
		return os;
	}

	lastBlockTemp = integerTemp.lastBlock;
	lastDataTemp = integerTemp.lastData;

	blockTemp2 = decimalIntegerTemp.firstBlock;
	charTemp = (char *)(blockTemp2->getData());
#ifdef w64
	currentLastCharTemp = charTemp + blockTemp2->getSize() * 8;
#else
	currentLastCharTemp = charTemp + blockTemp2->getSize() * 4;
#endif

	while (true){
		if (lastDataTemp < lastBlockTemp->getData()){
			if (lastBlockTemp == integerTemp.firstBlock){
				break;
			}
			lastBlockTemp = lastBlockTemp->getPrev();
			lastDataTemp = lastBlockTemp->getData() + lastBlockTemp->getSize() - 1;
		}
		blockTemp1 = lastBlockTemp;
		dataTemp = lastDataTemp;
		currentLastDataTemp = blockTemp1->getData();
		while (true){
			if (dataTemp < currentLastDataTemp){
				if (blockTemp1 == integerTemp.firstBlock){
					break;
				}
				blockTemp1 = blockTemp1->getPrev();
				currentLastDataTemp = blockTemp1->getData();
				dataTemp = currentLastDataTemp + blockTemp1->getSize() - 1;
			}
#ifdef w64
			__asm{
				push rax;
				push rbx;
				push rdx;
				mov rbx, qword ptr[dataTemp];
				mov rax, qword ptr[rbx];
				mov rdx, remainder;
				div divisor;
				mov qword ptr[rbx], rax;
				mov remainder, rdx;
				pop rdx;
				pop rbx;
				pop rax;
			}
#else
			__asm{
				push eax;
				push ebx;
				push edx;
				mov ebx, dword ptr[dataTemp];
				mov eax, dword ptr[ebx];
				mov edx, remainder;
				div divisor;
				mov dword ptr[ebx], eax;
				mov remainder, edx;
				pop edx;
				pop ebx;
				pop eax;
			}
#endif
			dataTemp--;
		}
		for (int count = 0; count < BUFFERSIZE; count++){
			if (charTemp >= currentLastCharTemp){
				if (blockTemp2->getNext() == decimalIntegerTemp.firstBlock){
					try{
						decimalIntegerTemp.expandMemorySize(decimalIntegerTemp.memorySize);
					}
					catch(const std::bad_alloc& e){
						return os;
					}
				}
				blockTemp2 = blockTemp2->getNext();
				charTemp = (char *)(blockTemp2->getData());
#ifdef w64
				currentLastCharTemp = charTemp + blockTemp2->getSize() * 8;
#else
				currentLastCharTemp = charTemp + blockTemp2->getSize() * 4;
#endif
			}
			*charTemp = (char)(remainder % 10) + '0';
			if (*charTemp < 0){
				int a = 1;
			}
			charTemp++;
			remainder /= 10;
		}
		if (*lastDataTemp == 0){
			lastDataTemp--;
		}
	}
	charTemp--;
	if (rp.sign == -1){
		os << '-';
	}
	while (true){
		if (charTemp < (char *)(blockTemp2->getData())){
			if (blockTemp2 == decimalIntegerTemp.firstBlock){
				break;
			}
			blockTemp2 = blockTemp2->getPrev();
#ifdef w64
			charTemp = (char *)(blockTemp2->getData()) + blockTemp2->getSize() * 8 - 1;
#else
			charTemp = (char *)(blockTemp2->getData()) + blockTemp2->getSize() * 4 - 1;
#endif
		}
		if (*charTemp != '0'){
			break;
		}
		charTemp--;
	}
	/*
	if (*charTemp == '2' && flag == true){
		std::cout << "aaa";
		std::cout << *charTemp << *(charTemp - 1);
		system("pause");
		flag1 = true;
	}
	*/
	while (true){
		if (charTemp < (char *)(blockTemp2->getData())){
			if (blockTemp2 == decimalIntegerTemp.firstBlock){
				break;
			}
			blockTemp2 = blockTemp2->getPrev();
#ifdef w64
			charTemp = (char *)(blockTemp2->getData()) + blockTemp2->getSize() * 8 - 1;
#else
			charTemp = (char *)(blockTemp2->getData()) + blockTemp2->getSize() * 4 - 1;
#endif
		}
		os << *charTemp;
		charTemp--;
	}
	
	return os;
}

void ud_integer::addPP(ud_integer & rp){
	UD_UINT * lDataTemp, *rDataTemp, *integerRetDataTemp, *lCurrentLastData, *rCurrentLastData, *integerRetCurrentLastData;
	bool overflowFlagTemp;
	ud_integer_block * lBlockTemp, *rBlockTemp, *integerRetBlockTemp;
	ud_integer integerTemp;
	
	if (this->dataSize == 0 || rp.dataSize == 0){
		this->destroy();
		return;
	}

	if (*(this->refCount) > 1){
		integerTemp = *this;
		this->destroy();
		this->copyFrom(integerTemp);
	}

	if (this->dataSize == 0){
		return;
	}

	try{
		this->expandMemorySize(this->memorySize > rp.dataSize ? 0 : rp.dataSize - this->memorySize);
	}
	catch(const std::bad_alloc & e){
		this->destroy();
		return;
	}
	
	lBlockTemp = this->firstBlock;
	lDataTemp = lBlockTemp->getData();
	if (lBlockTemp == this->lastBlock){
		lCurrentLastData = this->lastData + 1;
	}
	else{
		lCurrentLastData = lDataTemp + lBlockTemp->getSize();
	}

	rBlockTemp = rp.firstBlock;
	rDataTemp = rBlockTemp->getData();
	if (rBlockTemp == rp.lastBlock){
		rCurrentLastData = rp.lastData + 1;
	}
	else{
		rCurrentLastData = rDataTemp + rBlockTemp->getSize();
	}

	overflowFlagTemp = false;

	while (true){
		if (lDataTemp >= lCurrentLastData){
			if(lBlockTemp == this->lastBlock){
				break;
			}
			lBlockTemp = lBlockTemp->getNext();
			lDataTemp = lBlockTemp->getData();
			if (lBlockTemp == this->lastBlock){
				lCurrentLastData = this->lastData + 1;
			}
			else{
				lCurrentLastData = lDataTemp + lBlockTemp->getSize();
			}
		}
		if (rDataTemp >= rCurrentLastData){
			if(rBlockTemp == rp.lastBlock){
				break;
			}
			rBlockTemp = rBlockTemp->getNext();
			rDataTemp = rBlockTemp->getData();
			if (rBlockTemp == rp.lastBlock){
				rCurrentLastData = rp.lastData + 1;
			}
			else{
				rCurrentLastData = rDataTemp + rBlockTemp->getSize();
			}
		}
		*lDataTemp += *rDataTemp;
		__asm{
			jnc mark3;
		}
		if (overflowFlagTemp){
			(*lDataTemp)++;
		}
		overflowFlagTemp = true;
		goto mark4;
	mark3:
		if (overflowFlagTemp){
			(*lDataTemp)++;
			if (*lDataTemp == 0){
				overflowFlagTemp = true;
				goto mark4;
			}
		}
	mark5:
		overflowFlagTemp = false;
	mark4:
		lDataTemp++;
		rDataTemp++;
	}
	if (this->dataSize > rp.dataSize){
		while (true){
			if (lDataTemp >= lCurrentLastData){
				if(lBlockTemp == this->lastBlock){
					break;
				}
				lBlockTemp = lBlockTemp->getNext();
				lDataTemp = lBlockTemp->getData();
				if (lBlockTemp == this->lastBlock){
					lCurrentLastData = this->lastData + 1;
				}
				else{
					lCurrentLastData = lDataTemp + lBlockTemp->getSize();
				}
			}
			if(overflowFlagTemp){
				(*lDataTemp)++;
				if (*lDataTemp == 0){
					goto mark7;
				}
			mark6:
				overflowFlagTemp = false;
				break;
			}
			else{
				break;
			}
		mark7:
			lDataTemp++;
		}
	}
	else if (this->dataSize < rp.dataSize){
		while (true){
			if (rDataTemp >= rCurrentLastData){
				if(rBlockTemp == rp.lastBlock){
					break;
				}
				rBlockTemp = rBlockTemp->getNext();
				rDataTemp = rBlockTemp->getData();
				if (rBlockTemp == rp.lastBlock){
					rCurrentLastData = rp.lastData + 1;
				}
				else{
					rCurrentLastData = rDataTemp + rBlockTemp->getSize();
				}
			}
			if (lDataTemp >= lCurrentLastData){
				lBlockTemp = lBlockTemp->getNext();
				lDataTemp = lBlockTemp->getData();
				lCurrentLastData = lDataTemp + lBlockTemp->getSize();
			}
			if(overflowFlagTemp){
				*lDataTemp = *rDataTemp + 1;
				if (*lDataTemp == 0){
					goto mark9;
				}
			mark8:
				overflowFlagTemp = false;
			}
			else{
				*lDataTemp = *rDataTemp;
			}
		mark9:
			rDataTemp++;
			lDataTemp++;
		}
		this->lastBlock = lBlockTemp;
		this->lastData = lDataTemp - 1;
		this->dataSize = rp.dataSize;
	}
	if(overflowFlagTemp == true){
		if (this->lastData == this->firstBlock->getPrev()->getData() + this->firstBlock->getPrev()->getSize() - 1){
			try{
				this->expandMemorySize(1);
			}
			catch (const std::bad_alloc & e){
				this->destroy();
				return;
			}
			*(this->firstBlock->getPrev()->getData() + this->firstBlock->getPrev()->getSize() - 1) = 1;
			this->lastBlock = this->firstBlock->getPrev();
			this->lastData = this->lastBlock->getData();
		}
		else{
			(this->lastData)++;
			if (this->lastData >= this->lastBlock->getData() + this->lastBlock->getSize()){
				this->lastBlock = this->lastBlock->getNext();
				this->lastData = this->lastBlock->getData();
			}
		}
		(this->dataSize)++;
	}

	this->sign = 1;

	return;
}

void ud_integer::subPP(ud_integer & rp){
	UD_UINT * lDataTemp, *rDataTemp, *integerRetDataTemp, *lCurrentLastData, *rCurrentLastData, *integerRetCurrentLastData;
	bool overflowFlagTemp;
	ud_integer_block * lBlockTemp, *rBlockTemp, *integerRetBlockTemp;
	ud_integer integerTemp;

	if (this->dataSize == 0 || rp.dataSize == 0){
		this->destroy();
		return;
	}

	if (*(this->refCount) > 1){
		integerTemp = *this;
		this->destroy();
		this->copyFrom(integerTemp);
	}

	if (this->dataSize == 0){
		return;
	}

	try{
		this->expandMemorySize(this->memorySize > rp.dataSize ? 0 : rp.dataSize - this->memorySize);
	}
	catch (const std::bad_alloc & e){
		this->destroy();
		return;
	}

	lBlockTemp = this->firstBlock;
	lDataTemp = lBlockTemp->getData();
	if (lBlockTemp == this->lastBlock){
		lCurrentLastData = this->lastData + 1;
	}
	else{
		lCurrentLastData = lDataTemp + lBlockTemp->getSize();
	}

	rBlockTemp = rp.firstBlock;
	rDataTemp = rBlockTemp->getData();
	if (rBlockTemp == rp.lastBlock){
		rCurrentLastData = rp.lastData + 1;
	}
	else{
		rCurrentLastData = rDataTemp + rBlockTemp->getSize();
	}

	overflowFlagTemp = false;

	while (true){
		if (lDataTemp >= lCurrentLastData){
			if (lBlockTemp == this->lastBlock){
				break;
			}
			lBlockTemp = lBlockTemp->getNext();
			lDataTemp = lBlockTemp->getData();
			if (lBlockTemp == this->lastBlock){
				lCurrentLastData = this->lastData + 1;
			}
			else{
				lCurrentLastData = lDataTemp + lBlockTemp->getSize();
			}
		}
		if (rDataTemp >= rCurrentLastData){
			if (rBlockTemp == rp.lastBlock){
				break;
			}
			rBlockTemp = rBlockTemp->getNext();
			rDataTemp = rBlockTemp->getData();
			if (rBlockTemp == rp.lastBlock){
				rCurrentLastData = rp.lastData + 1;
			}
			else{
				rCurrentLastData = rDataTemp + rBlockTemp->getSize();
			}
		}
		
		*lDataTemp -= *rDataTemp;
		__asm{
			jnc mark10;
		}
		if (overflowFlagTemp){
			(*lDataTemp)--;
		}
		overflowFlagTemp = true;
		goto mark11;
	mark10:
		if (overflowFlagTemp){
			(*lDataTemp)--;
			if (*lDataTemp == MAX_UD_UINT){
				overflowFlagTemp = true;
				goto mark11;
			}
		}
	mark12:
		overflowFlagTemp = false;
	mark11:
		lDataTemp++;
		rDataTemp++;
	}
	if (this->dataSize > rp.dataSize){
		while (true){
			if (lDataTemp >= lCurrentLastData){
				if (lBlockTemp == this->lastBlock){
					break;
				}
				lBlockTemp = lBlockTemp->getNext();
				lDataTemp = lBlockTemp->getData();
				if (lBlockTemp == this->lastBlock){
					lCurrentLastData = this->lastData + 1;
				}
				else{
					lCurrentLastData = lDataTemp + lBlockTemp->getSize();
				}
			}
			if (overflowFlagTemp){
				(*lDataTemp)--;
				if (*lDataTemp == MAX_UD_UINT){
					overflowFlagTemp = true;
					goto mark14;
				}
			mark13:
				overflowFlagTemp = false;
				break;
			}
			else{
				break;
			}
		mark14:
			lDataTemp++;
		}
	}
	else if (this->dataSize < rp.dataSize){
		while (true){
			if (rDataTemp >= rCurrentLastData){
				if (rBlockTemp == rp.lastBlock){
					break;
				}
				rBlockTemp = rBlockTemp->getNext();
				rDataTemp = rBlockTemp->getData();
				if (rBlockTemp == rp.lastBlock){
					rCurrentLastData = rp.lastData + 1;
				}
				else{
					rCurrentLastData = rDataTemp + rBlockTemp->getSize();
				}
			}
			if (lDataTemp >= lCurrentLastData){
				lBlockTemp = lBlockTemp->getNext();
				lDataTemp = lBlockTemp->getData();
				lCurrentLastData = lDataTemp + lBlockTemp->getSize();
			}
			*lDataTemp = ~*rDataTemp;
			if (!overflowFlagTemp){
				(*lDataTemp)++;
				if (*lDataTemp == 0){
					overflowFlagTemp = false;
					goto mark16;
				}
			mark15:
				overflowFlagTemp = true;
			}
		mark16:
			rDataTemp++;
			lDataTemp++;
		}
		this->lastBlock = lBlockTemp;
		this->lastData = lDataTemp - 1;
		this->dataSize = rp.dataSize;
	}

	if (!overflowFlagTemp){
		this->sign = 1;
	}
	else{
		this->sign = -1;
	}

	if (overflowFlagTemp == true){
		lBlockTemp = this->firstBlock;
		lDataTemp = lBlockTemp->getData();
		lCurrentLastData = lDataTemp + lBlockTemp->getSize();
		while (true){
			if (lDataTemp >= lCurrentLastData){
				if (lBlockTemp == this->lastBlock){
					break;
				}
				lBlockTemp = lBlockTemp->getNext();
				lDataTemp = lBlockTemp->getData();
				if (lBlockTemp == this->lastBlock){
					lCurrentLastData = this->lastData + 1;
				}
				else{
					lCurrentLastData = lDataTemp + lBlockTemp->getSize();
				}
			}
			*lDataTemp = ~*lDataTemp;
			if (overflowFlagTemp){
				(*lDataTemp)++;
				if (*lDataTemp == 0){
					overflowFlagTemp = true;
					goto mark18;
				}
			mark17:
				overflowFlagTemp = false;
			}
		mark18:
			lDataTemp++;
		}
	}

	this->minimizeNumberSize();

	return;
}

ud_integer operator -(ud_integer & rp){
	ud_integer integerRet;
	if (rp.dataSize == 0){
		return integerRet;
	}
	else{
		(*(rp.refCount))++;
		integerRet.firstBlock = rp.firstBlock;
		integerRet.lastBlock = rp.lastBlock;
		integerRet.lastData = rp.lastData;
		integerRet.memorySize = rp.memorySize;
		integerRet.sign = -rp.sign;
		integerRet.dataSize = rp.dataSize;
		integerRet.refCount = rp.refCount;
	}
	return integerRet;
}

ud_integer & ud_integer::operator +=(ud_integer & rp){
	if (this->dataSize == 0 || rp.dataSize == 0){
		this->destroy();
		return *this;
	}
	if (this->sign == 0){
		this->copyFrom(rp);
		return *this;
	}
	if (this->sign == 1 && rp.sign == 1){
		this->addPP(rp);
		return *this;
	}
	if (this->sign == 1 || rp.sign == -1){
		this->subPP(rp);
		return *this;
	}
	if (this->sign == -1 || rp.sign == 1){
		this->subPP(rp);
		this->sign = -this->sign;
		return *this;
	}
	else{
		this->addPP(rp);
		this->sign = -this->sign;
		return *this;
	}
}

ud_integer & ud_integer::operator -=(ud_integer & rp){
	if (this->dataSize == 0 || rp.dataSize == 0){
		this->destroy();
		return *this;
	}
	if (this->sign == 0){
		this->copyFrom(-rp);
		return *this;
	}
	if (this->sign == 1 && rp.sign == -1){
		this->addPP(rp);
		return *this;
	}
	if (this->sign == 1 || rp.sign == 1){
		this->subPP(rp);
		return *this;
	}
	if (this->sign == -1 || rp.sign == -1){
		this->subPP(rp);
		this->sign = -this->sign;
		return *this;
	}
	else{
		this->addPP(rp);
		this->sign = -this->sign;
		return *this;
	}
}

ud_integer operator *(ud_integer & lp, ud_integer & rp){
	ud_integer integerRet;
	multiply(lp, rp, integerRet);
	/*
	std::ofstream ftest;
	ftest.open("ptest.txt");
	ftest << integerRet;
	*/
	return integerRet;
}

ud_integer operator /(ud_integer & lp, ud_integer & rp){
	ud_integer multiplierTemp, dividendTemp, divisorTemp, quotientTemp, quotientTemp1, productTemp;
	UD_UINT divisorFPTemp, remainderTemp, *dataTemp1, *currentLastDataTemp1, *dataTemp2, *currentLastDataTemp2, *lastDataTemp1;
	int count, signTemp1, signTemp2;
	ud_integer_block * blockTemp1, *blockTemp2, *lastBlockTemp1;

	std::ofstream ftest;

	if (lp.dataSize == 0 || rp.dataSize == 0 || rp.sign == 0){
		return quotientTemp;
	}
	if (lp.sign == 0){
		quotientTemp.copyFrom(lp);
		return quotientTemp;
	}
	try{
		multiplierTemp.expandMemorySize(1);
	}
	catch (const std::bad_alloc & e){
		return quotientTemp;
	}
	*(multiplierTemp.firstBlock->getData()) = 1;
	multiplierTemp.dataSize = 1;
	multiplierTemp.lastBlock = multiplierTemp.firstBlock;
	multiplierTemp.lastData = multiplierTemp.firstBlock->getData();
	multiplierTemp.sign = 1;
	count = (int)(log((double)(*(rp.lastData))) / log(2.0));
#ifdef w64
	count = 63 - count;
#else
	count = 31 - count;
#endif
	for (int count1 = 0; count1 < count; count1++){
		*(multiplierTemp.firstBlock->getData()) *= 2;
	}

	dividendTemp = lp * multiplierTemp;
	divisorTemp = rp * multiplierTemp;
	divisorFPTemp = *(divisorTemp.lastData);

	if (dividendTemp.dataSize == 0 || divisorTemp.dataSize == 0){
		return quotientTemp;
	}

	try{
		quotientTemp1.expandMemorySize(dividendTemp.dataSize - divisorTemp.dataSize + 1);
	}
	catch (const std::bad_alloc & e){
		return quotientTemp;
	}
	
	int countTemp = 0;
	
	while (true){

		quotientTemp1.dataSize = *(dividendTemp.lastData) >= divisorFPTemp ? (dividendTemp.dataSize - divisorTemp.dataSize + 1) : (dividendTemp.dataSize - divisorTemp.dataSize);

		if (quotientTemp1.dataSize <= 0){
			break;
		}
		blockTemp1 = dividendTemp.lastBlock;
		dataTemp1 = dividendTemp.lastData;
		currentLastDataTemp1 = blockTemp1->getData();

		blockTemp2 = quotientTemp1.firstBlock;
		dataTemp2 = blockTemp2->getData();
		currentLastDataTemp2 = dataTemp2 + blockTemp2->getSize();

		for (count = divisorTemp.dataSize; count < dividendTemp.dataSize; count++){
			dataTemp2++;
			if (dataTemp2 >= currentLastDataTemp2){
				blockTemp2 = blockTemp2->getNext();
				dataTemp2 = blockTemp2->getData();
				currentLastDataTemp2 = dataTemp2 + blockTemp2->getSize();
			}
		}
		currentLastDataTemp2 = blockTemp2->getData();
		quotientTemp1.lastBlock = blockTemp2;
		quotientTemp1.lastData = dataTemp2;
		quotientTemp1.sign = dividendTemp.sign * divisorTemp.sign;
		remainderTemp = 0;
		while (true){
			if (dataTemp1 < currentLastDataTemp1){
				blockTemp1 = blockTemp1->getPrev();
				currentLastDataTemp1 = blockTemp1->getData();
				dataTemp1 = currentLastDataTemp1 + blockTemp1->getSize() - 1;
			}
			if (dataTemp2 < currentLastDataTemp2){
				if (blockTemp2 == quotientTemp1.firstBlock){
					break;
				}
				blockTemp2 = blockTemp2->getPrev();
				currentLastDataTemp2 = blockTemp2->getData();
				dataTemp2 = dataTemp2 + blockTemp2->getSize() - 1;
			}
#ifdef w64
			__asm{
				push rax;
				push rbx;
				push rcx;
				push rdx;
				mov rbx, qword ptr[dataTemp1];
				mov rcx, qword ptr[dataTemp2];
				mov rax, qword ptr[rbx];
				mov rdx, remainderTemp;
				div divisorFPTemp;
				mov qword ptr[rcx], rax;
				mov remainderTemp, rdx;
				pop rdx;
				pop rcx;
				pop rbx;
				pop rax;
			}
#else
			__asm{
				push eax;
				push ebx;
				push ecx;
				push edx;
				mov ebx, dword ptr[dataTemp1];
				mov ecx, dword ptr[dataTemp2];
				mov eax, dword ptr[ebx];
				mov edx, remainderTemp;
				div divisorFPTemp;
				mov dword ptr[ecx], eax;
				mov remainderTemp, edx;
				pop edx;
				pop ecx;
				pop ebx;
				pop eax;
			}
#endif
			dataTemp1--;
			dataTemp2--;
		}
		if (*(quotientTemp1.lastData) == 0){
			(quotientTemp1.lastData)--;
			if (quotientTemp1.lastData < quotientTemp1.lastBlock->getData()){
				quotientTemp1.lastBlock = quotientTemp1.lastBlock->getPrev();
				quotientTemp1.lastData = quotientTemp1.lastBlock->getData() + quotientTemp1.lastBlock->getSize() - 1;
			}
		}
		/*
		countTemp++;

		ftest.open("quotientTemp+.txt");
		flag == true;
		std::cout << quotientTemp1.dataSize << std::endl;
		ftest << quotientTemp + quotientTemp1;
		flag == false;
		ftest.close();
		*/
		/*
		std::cout << countTemp << '\t' << *(dividendTemp.lastData) << std::endl;
		std::cout << quotientTemp1.dataSize << std::endl;
		*/
		//system("pause");

		if (quotientTemp.dataSize == 0){
			quotientTemp.copyFrom(quotientTemp1);
		}
		else{
			/*
			if (flag1){

				ftest.open("quotientTemp.txt");
				ftest << quotientTemp;
				ftest.close();

				std::cout << "bbb";
				ftest.open("quotientTemp1.txt");
				ftest << quotientTemp1;
				ftest.close();
				flag1 = false;
				system("pause");
			}
			*/
			quotientTemp += quotientTemp1;
		}
		/*
		countTemp++;
		if (countTemp > 700){
			ftest.close();
			return quotientTemp;
		}
		*/
		
		multiply(divisorTemp, quotientTemp1, productTemp);
		/*
		std::cout << quotientTemp1.dataSize;
		ftest.open("dividendTemp.txt");
		ftest << dividendTemp;
		ftest.close();
		ftest.open("quotientTemp1.txt");
		ftest << quotientTemp1;
		ftest.close();
		ftest.open("productTemp.txt");
		ftest << productTemp;
		ftest.close();

		system("pause");
		*/
		dividendTemp -= productTemp;
	}
	/*
	ftest.open("dividendTemp.txt");
	ftest << dividendTemp;
	ftest.close();
	ftest.open("quotientTemp1.txt");
	ftest << quotientTemp1;
	ftest.close();
	ftest.open("productTemp.txt");
	ftest << productTemp;
	ftest.close();
	*/

	*(multiplierTemp.firstBlock->getData()) = 1;
	count = (int)(log((double)(*(rp.lastData))) / log(2.0));
	for (int count1 = 0; count1 < count; count1++){
		*(multiplierTemp.firstBlock->getData()) *= 2;
	}

	dividendTemp = dividendTemp * multiplierTemp;

	blockTemp1 = dividendTemp.firstBlock;
	dataTemp2 = dataTemp1 = blockTemp1->getData();
	if (blockTemp1 == dividendTemp.lastBlock){
		currentLastDataTemp1 = dividendTemp.lastData + 1;
	}
	else{
		currentLastDataTemp1 = blockTemp1->getData() + blockTemp1->getSize();
	}
	currentLastDataTemp1 = blockTemp1->getData() + blockTemp1->getSize();

	while (true){
		dataTemp1++;
		if (dataTemp1 >= currentLastDataTemp1){
			if (blockTemp1 == dividendTemp.lastBlock){
				break;
			}
			blockTemp1 = blockTemp1->getNext();
			dataTemp1 = blockTemp1->getData();
			if (blockTemp1 == dividendTemp.lastBlock){
				currentLastDataTemp1 = dividendTemp.lastData + 1;
			}
			else{
				currentLastDataTemp1 = blockTemp1->getData() + blockTemp1->getSize();
			}
		}
		*dataTemp2 = *dataTemp1;
		dataTemp2 = dataTemp1;
	}
	if (dividendTemp.lastBlock->getData() == dividendTemp.lastData){
		dividendTemp.lastBlock = dividendTemp.lastBlock->getPrev();
		dividendTemp.lastData = dividendTemp.lastBlock->getData() + dividendTemp.lastBlock->getSize() - 1;
	}
	else{
		dividendTemp.lastData--;
	}
	dividendTemp.dataSize--;
	if (dividendTemp.sign == -1){
		*(multiplierTemp.firstBlock->getData()) = 1;
		if (rp.sign == 1){
			dividendTemp += rp;
			quotientTemp -= multiplierTemp;
		}
		else{
			dividendTemp -= rp;
			quotientTemp += multiplierTemp;
		}
	}
	/*
	if (dividendTemp.sign == 1){
		if (signTemp1 == -1){
			*(multiplierTemp.firstBlock->getData()) = 1;
			dividendTemp = rp - dividendTemp;
			if (signTemp2 == 1){
				quotientTemp = -quotientTemp;
			}
			quotientTemp -= multiplierTemp;
		}
		else{
			if (signTemp2 == -1){
				quotientTemp = -quotientTemp;
			}
		}
	}
	else if (dividendTemp.sign == -1){
		dividendTemp = -dividendTemp;
		if (signTemp1 == 1){
			*(multiplierTemp.firstBlock->getData()) = 1;
			dividendTemp = rp - dividendTemp;
			if (signTemp2 == -1){
				quotientTemp = -quotientTemp;
			}
			quotientTemp -= multiplierTemp;
		}
		else{
			if (signTemp2 == 1){
				quotientTemp = -quotientTemp;
			}
		}
	}
	*/
	return quotientTemp;
}

void multiply(ud_integer & lp, ud_integer & rp, ud_integer & integerRet){
	UD_UINT * lDataTemp, *rDataTemp, *integerRetDataTemp, *integerRetStartPosDataTemp, *lCurrentLastData, *rCurrentLastData, *integerRetCurrentLastData, *integerRetStartPosCurrentLastData;
	UD_UINT overflowDataTemp, multiplier;
	ud_integer_block * lBlockTemp, *rBlockTemp, *integerRetBlockTemp, *integerRetStartPosBlockTemp;

	if (lp.dataSize == 0 || rp.dataSize == 0){
		integerRet.destroy();
		return;
	}
	if (integerRet.refCount != NULL && *(integerRet.refCount) > 1){
		integerRet.destroy();
	}
	if (lp.sign == 0){
		integerRet.copyFrom(lp);
		return;
	}
	if (rp.sign == 0){
		integerRet.copyFrom(rp);
		return;
	}
	try{
		integerRet.expandMemorySize(integerRet.memorySize > (lp.dataSize + rp.dataSize) ? 0 : (lp.dataSize + rp.dataSize - integerRet.memorySize));
	}
	catch (const std::bad_alloc & e){
		integerRet.destroy();
		return;
	}

	integerRetBlockTemp = integerRet.firstBlock;
	integerRetDataTemp = integerRetBlockTemp->getData();
	integerRetCurrentLastData = integerRetDataTemp + integerRetBlockTemp->getSize();
	while (true){
		if (integerRetDataTemp >= integerRetCurrentLastData){
			integerRetBlockTemp = integerRetBlockTemp->getNext();
			integerRetDataTemp = integerRetBlockTemp->getData();
			integerRetCurrentLastData = integerRetDataTemp + integerRetBlockTemp->getSize();
			if (integerRetBlockTemp == integerRet.firstBlock){
				break;
			}
		}
		*integerRetDataTemp = 0;
		integerRetDataTemp++;
	}

	rBlockTemp = rp.firstBlock;
	rDataTemp = rBlockTemp->getData();
	if (rBlockTemp == rp.lastBlock){
		rCurrentLastData = rp.lastData + 1;
	}
	else{
		rCurrentLastData = rDataTemp + rBlockTemp->getSize();
	}

	integerRetStartPosBlockTemp = integerRet.firstBlock;
	integerRetStartPosDataTemp = integerRetStartPosBlockTemp->getData();
	integerRetStartPosCurrentLastData = integerRetStartPosDataTemp + integerRetStartPosBlockTemp->getSize();

	while (true){
		if (rDataTemp >= rCurrentLastData){
			if (rBlockTemp == rp.lastBlock){
				break;
			}
			rBlockTemp = rBlockTemp->getNext();
			rDataTemp = rBlockTemp->getData();
			if (rBlockTemp == rp.lastBlock){
				rCurrentLastData = rp.lastData + 1;
			}
			else{
				rCurrentLastData = rDataTemp + rBlockTemp->getSize();
			}
		}
		if (integerRetStartPosDataTemp >= integerRetStartPosCurrentLastData){
			integerRetStartPosBlockTemp = integerRetStartPosBlockTemp->getNext();
			integerRetStartPosDataTemp = integerRetStartPosBlockTemp->getData();
			integerRetStartPosCurrentLastData = integerRetStartPosDataTemp + integerRetStartPosBlockTemp->getSize();
		}
		integerRetBlockTemp = integerRetStartPosBlockTemp;
		integerRetDataTemp = integerRetStartPosDataTemp;
		integerRetCurrentLastData = integerRetStartPosCurrentLastData;

		lBlockTemp = lp.firstBlock;
		lDataTemp = lBlockTemp->getData();
		if (lBlockTemp == lp.lastBlock){
			lCurrentLastData = lp.lastData + 1;
		}
		else{
			lCurrentLastData = lDataTemp + lBlockTemp->getSize();
		}

		overflowDataTemp = 0;
		multiplier = *rDataTemp;
		while (true){
			if (lDataTemp >= lCurrentLastData){
				if (lBlockTemp == lp.lastBlock){
					break;
				}
				lBlockTemp = lBlockTemp->getNext();
				lDataTemp = lBlockTemp->getData();
				if (lBlockTemp == lp.lastBlock){
					lCurrentLastData = lp.lastData + 1;
				}
				else{
					lCurrentLastData = lDataTemp + lBlockTemp->getSize();
				}
			}
			if (integerRetDataTemp >= integerRetCurrentLastData){
				integerRetBlockTemp = integerRetBlockTemp->getNext();
				integerRetDataTemp = integerRetBlockTemp->getData();
				integerRetCurrentLastData = integerRetDataTemp + integerRetBlockTemp->getSize();
			}
			/*
			if (lDataTemp == lp.lastData && rDataTemp == rp.lastData){
				int a;
				a = a + 1;
			}
			*/
#ifdef w64
			__asm{
				push rax;
				push rbx;
				push rcx;
				push rdx;
				mov rbx, qword ptr[lDataTemp];
				mov rcx, qword ptr[integerRetDataTemp];
				mov rax, qword ptr[rbx];
				mul multiplier;
				add rax, qword ptr[overflowDataTemp];
				jnc mark19;
				inc rdx;
			mark19:
				add rax, qword ptr[rcx];
				jnc mark20;
				inc rdx;
			mark20:
				mov qword ptr[overflowDataTemp], rdx;
				mov qword ptr[rcx], rax;
				pop rdx;
				pop rcx;
				pop rbx;
				pop rax;
			}
#else
			__asm{
				push eax;
				push ebx;
				push ecx;
				push edx;
				mov ebx, dword ptr[lDataTemp];
				mov ecx, dword ptr[integerRetDataTemp];
				mov eax, dword ptr[ebx];
				mul multiplier;
				add eax, dword ptr[overflowDataTemp];
				jnc mark19;
				inc edx;
			mark19:
				add eax, dword ptr[ecx];
				jnc mark20;
				inc edx;
			mark20:
				mov dword ptr[overflowDataTemp], edx;
				mov dword ptr[ecx], eax;
				pop edx;
				pop ecx;
				pop ebx;
				pop eax;
			}
#endif
			lDataTemp++;
			integerRetDataTemp++;
		}
		if (overflowDataTemp != 0){
			while (true){
				if (integerRetDataTemp >= integerRetCurrentLastData){
					integerRetBlockTemp = integerRetBlockTemp->getNext();
					integerRetDataTemp = integerRetBlockTemp->getData();
					integerRetCurrentLastData = integerRetDataTemp + integerRetBlockTemp->getSize();
				}
				(*integerRetDataTemp) += overflowDataTemp;
				__asm{
					jnc mark21;
				}
				overflowDataTemp = 1;
				goto mark22;
			mark21:
				integerRetDataTemp++;
				break;
			mark22:
				integerRetDataTemp++;
			}
		}
		rDataTemp++;
		integerRetStartPosDataTemp++;
	}
	integerRet.dataSize = lp.dataSize + rp.dataSize - 1;
	if (overflowDataTemp != 0){
		integerRet.dataSize++;
	}
	integerRet.lastBlock = integerRetBlockTemp;
	integerRet.lastData = integerRetDataTemp - 1;
	integerRet.sign = lp.sign * rp.sign;
	/*
	std::ofstream ftest;
	ftest.open("ptest.txt");
	ftest << lp << std::endl;
	ftest << rp << std::endl;
	ftest << integerRet << std::endl;
	system("pause");
	*/
	return;
}

ud_integer operator +(ud_integer & lp, ud_integer & rp){
	ud_integer integerRet;
	integerRet.copyFrom(lp);
	integerRet += rp;
	return integerRet;
}

ud_integer operator -(ud_integer & lp, ud_integer & rp){
	return lp + (-rp);
}

void ud_integer::copyFrom(ud_integer & rp){
	ud_integer integerRet;
	ud_integer_block * blockTemp1, *blockTemp2;
	UD_UINT * dataTemp1, *dataTemp2, *currentLastDataTemp1, *currentLastDataTemp2;
	if (rp.dataSize == 0){
		this->destroy();
		return;
	}

	if (this->refCount != NULL && *(this->refCount) > 1){
		this->destroy();
	}

	try{
		this->expandMemorySize(this->memorySize > rp.dataSize ? 0 : rp.dataSize - this->memorySize);
	}
	catch (const std::bad_alloc & e){
		this->destroy();
		return;
	}
	
	blockTemp1 = this->firstBlock;
	dataTemp1 = blockTemp1->getData();
	currentLastDataTemp1 = dataTemp1 + blockTemp1->getSize();
	
	blockTemp2 = rp.firstBlock;
	dataTemp2 = blockTemp2->getData();
	if (blockTemp2 == rp.lastBlock){
		currentLastDataTemp2 = rp.lastData + 1;
	}
	else{
		currentLastDataTemp2 = dataTemp2 + blockTemp2->getSize();
	}
	
	while (true){
		if (dataTemp2 >= currentLastDataTemp2){
			if (blockTemp2 == rp.lastBlock){
				break;
			}
			blockTemp2 = blockTemp2->getNext();
			dataTemp2 = blockTemp2->getData();
			if (blockTemp2 == rp.lastBlock){
				currentLastDataTemp2 = rp.lastData + 1;
			}
			else{
				currentLastDataTemp2 = dataTemp2 + blockTemp2->getSize();
			}
		}
		if (dataTemp1 >= currentLastDataTemp1){
			blockTemp1 = blockTemp1->getNext();
			dataTemp1 = blockTemp1->getData();
			currentLastDataTemp1 = dataTemp1 + blockTemp1->getSize();
		}
		*dataTemp1 = *dataTemp2;
		dataTemp1++;
		dataTemp2++;
	}
	
	this->lastBlock = blockTemp1;
	this->lastData = dataTemp1 - 1;
	this->dataSize = rp.dataSize;
	this->sign = rp.sign;
	
	return;
}

ud_integer & ud_integer::operator =(ud_integer & rp){
	if (rp.dataSize == NULL){
		this->destroy();
		this->refCount = NULL;
	}
	else{
		(*(rp.refCount))++;
		this->destroy();
		this->firstBlock = rp.firstBlock;
		this->lastBlock = rp.lastBlock;
		this->lastData = rp.lastData;
		this->memorySize = rp.memorySize;
		this->sign = rp.sign;
		this->dataSize = rp.dataSize;
		this->refCount = rp.refCount;
	}
	return *this;
}

int main(){
	DWORD t1, t2;

	std::ifstream ftest1;
	std::ofstream ftest2;
	ud_integer test1, test2, test3;
	std::string integerTemp;
	char operatorTemp;
	std::getline(std::cin, integerTemp);
	operatorTemp = integerTemp[0];
	std::getline(std::cin, integerTemp);
	ftest2.open("integerTestTemp1.txt");
	ftest2 << integerTemp;
	ftest2.close();
	ftest1.open("integerTestTemp1.txt");
	ftest1 >> test1;
	ftest1.close();
	/*
	ftest2.open("integerTestTemp1.txt");
	ftest2 << test1;
	ftest2.close();
	*/
	std::getline(std::cin, integerTemp);
	ftest2.open("integerTestTemp1.txt");
	ftest2 << integerTemp;
	ftest2.close();
	ftest1.open("integerTestTemp1.txt");
	ftest1 >> test2;
	ftest1.close();
	/*
	ftest2.open("integerTestTemp1.txt");
	ftest2 << test2;
	ftest2.close();
	*/
	ftest2.open("integerTestTemp1.txt");
	t1 = timeGetTime();
	switch (operatorTemp)
	{
	case '+':{
				 ftest2 << test1 + test2;
				 break;
	}
	case '-':{
				 ftest2 << test1 - test2;
				 break;
	}
	case '*':{
				 ftest2 << test1 * test2;
				 break;
	}
	case '/':{
				 test3 = test1 / test2;
				 ftest2 << test3 << std::endl;
				 ftest2 << test1 - test2 * test3;
				 break;
	}
	default:{
				ftest2 << ' ';
				break;
	}
	}
	t2 = timeGetTime();
	ftest2.close();
	ftest1.open("integerTestTemp1.txt");
	getline(ftest1,integerTemp);
	std::cout << integerTemp << std::endl;
	if (operatorTemp == '/'){
		getline(ftest1, integerTemp);
		std::cout << integerTemp << std::endl;
	}
	ftest1.close();
	std::cout << t2 - t1 << std::endl;
	system("pause");
	return 0;
}