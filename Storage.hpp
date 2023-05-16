#ifndef Storage_hpp
#define Storage_hpp

#include <cstdint>
#include <iostream>
#include <fstream>

namespace ECE141 {
	struct Header {
      	size_t id; //4 bytes
        bool available; //1 byte
    	char name[30]; //30 bytes
        uint32_t time; //32bit = 4 bytes
        size_t size; //4 bytes
        size_t numOfBlocks; //4 bytes
     	size_t partnum; //4 bytes... 
        size_t nextInd; //4 bytes
	};

	const size_t theBlockSize = 1024;
	const size_t theHeaderSize = sizeof(Header);
	const size_t thePayloadSize = theBlockSize-theHeaderSize;

	struct ArchiveBlock {
     	Header  meta;
      	uint8_t data[thePayloadSize];
	};
  
  //does block level IO
  class Storage {
      public:

        void writeBlock(std::fstream &fs, ArchiveBlock ab, size_t index);
        void readHeader(std::fstream &fs, Header & h, size_t index);
        void readBlock(std::fstream &fs, ArchiveBlock & ab, size_t index);
        void emptyBlock(std::fstream &fs, size_t index);
        size_t findEmptyBlock(std::fstream &fs, size_t index);
        size_t getSumBlocks();
        void setSumBlocks(size_t sum);

        size_t sumBlocks = 0;

  };
}

#endif
