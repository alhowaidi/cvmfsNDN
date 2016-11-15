/**
 * This file is part of the CernVM File System.
 */

#include "tiered_cache.h"
#include "consumer/ndncatchunks.hpp"
#include <string>
#include <vector>
#include "hash.h"
#include "cache_posix.h"
#include "mountpoint.h"
#include "compression.h"
#include <stdio.h>
namespace cache {

#define BUFFER_SIZE (64*1024)

int
TieredCacheManager::Open(const shash::Any &id)
{
	std::string fileName =  id.MakePathWithoutSuffix();
	//printf("file name is %s \n",fileName.c_str());


	// /var/lib/cvmfs/secondary
	//FileSystem *fs;
	std::string pathName = "/var/lib/cvmfs/secondary/cms.cern.ch/";
	printf("cache path is %s \n", pathName.c_str());

	int fd = upper_->Open(id);
	if (fd >= 0) {
		printf(" upper hit .. \n");
		return fd;
	}

	int fd2 = lower_->Open(id);
	if (fd2 < 0)
	{
		// call ndn to download the filei
	//	printf("lower miss start NDN ... \n");
		ndn::chunks::ndnChunks nchunks;
		//std::string ndnName = fileName;//"common.hpp";

		printf("lower miss .. \n");

		nchunks.startChunk(fileName,pathName);
		fd2 = lower_->Open(id);
		
		if(fd2 < 0)
		{
			printf("still lower miss .. \n");
			return fd;
		}else
		{
			//FILE * inFile;
			//FILE * outFile;
		//	std::string inName = pathName+fileName+"COM";
		//	std::string outName = pathName+fileName;
			//inFile = fopen(inName.c_str(), "r");
			//outFile = fopen(inName.c_str(),"w");
			//zlib::DecompressFile2File(inFile,outFile);
		//	zlib::DecompressPath2Path(inName,outName);
		  //      printf(" .. done decompressing .. \n");
			//fclose(inFile);
			//fclose(outFile);
		}

		//return fd;
	}  // NOTE: use error code from upper.

	printf("lower cache hit .. \n");

	// Lower cache hit; upper cache miss.  Copy object into the
	// upper cache.
	int64_t size = lower_->GetSize(fd2);
	if (size < 0) {
		lower_->Close(fd2);
		return fd;
	}

	void *txn = alloca(upper_->SizeOfTxn());
	if (upper_->StartTxn(id, size, txn) < 0) {
		lower_->Close(fd2);
		return fd;
	}

	std::vector<char> m_buffer; m_buffer.reserve(BUFFER_SIZE);
	int64_t remaining = size;
	uint64_t offset = 0;
	while (remaining) {
		int nbytes = remaining > BUFFER_SIZE ? BUFFER_SIZE : remaining;
		int64_t result = lower_->Pread(fd2, &m_buffer[0], nbytes, offset);
		// The file we are reading is supposed to be exactly `size` bytes.
		if ((result < 0) || (result != nbytes)) {
			lower_->Close(fd2);
			upper_->AbortTxn(txn);
			return fd;
		}
		result = upper_->Write(&m_buffer[0], nbytes, txn);
		if (result < 0) {
			lower_->Close(fd2);
			upper_->AbortTxn(txn);
			return fd;
		}
		offset += nbytes;
		remaining -= nbytes;
	}
	lower_->Close(fd2);
	if (upper_->CommitTxn(txn) < 0) {
		return fd;
	}
	return upper_->Open(id);
}


int
TieredCacheManager::StartTxn(const shash::Any &id, uint64_t size, void *txn) {
	int upper_result = upper_->StartTxn(id, size, txn);
	if (upper_result < 0) {
		return upper_result;
	}

	void *txn2 = static_cast<char*>(txn) + upper_->SizeOfTxn();
	int lower_result = lower_->StartTxn(id, size, txn2);
	if (lower_result < 0) {
		upper_->AbortTxn(txn);
	}
	return lower_result;
}


void
TieredCacheManager::CtrlTxn(const std::string &description,
		const ObjectType type,
		const int flags,
		void *txn) {
	upper_->CtrlTxn(description, type, flags, txn);
	void *txn2 = static_cast<char*>(txn) + upper_->SizeOfTxn();
	lower_->CtrlTxn(description, type, flags, txn2);
}


int64_t
TieredCacheManager::Write(const void *buf, uint64_t size, void *txn) {
	int upper_result = upper_->Write(buf, size, txn);
	if (upper_result < 0) {return upper_result;}

	void *txn2 = static_cast<char*>(txn) + upper_->SizeOfTxn();
	return lower_->Write(buf, size, txn2);
}


int TieredCacheManager::Reset(void *txn) {
	int upper_result = upper_->Reset(txn);

	void *txn2 = static_cast<char*>(txn) + upper_->SizeOfTxn();
	int lower_result = lower_->Reset(txn2);

	return (upper_result < 0) ? upper_result : lower_result;
}


int
TieredCacheManager::AbortTxn(void *txn) {
	int upper_result = upper_->AbortTxn(txn);

	void *txn2 = static_cast<char*>(txn) + upper_->SizeOfTxn();
	int lower_result = lower_->AbortTxn(txn2);

	return (upper_result < 0) ? upper_result : lower_result;
}


int
TieredCacheManager::CommitTxn(void *txn) {
	int upper_result = upper_->CommitTxn(txn);

	void *txn2 = static_cast<char*>(txn) + upper_->SizeOfTxn();
	int lower_result = lower_->CommitTxn(txn2);

	return (upper_result < 0) ? upper_result : lower_result;
}

}  // namespace cache
