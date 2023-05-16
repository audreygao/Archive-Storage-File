
#include <iostream>
#include "Storage.hpp"
#include <cstdint>
#include <fstream>

namespace ECE141 {

    //read the header portion of the file block at index index into Header h
    void Storage::readHeader(std::fstream &fs, Header & h, size_t index) {
        fs.seekg(index * theBlockSize, fs.beg);
        fs.read((char *)&h, theHeaderSize);
    }

    //write the info at block ab into the file at block index 
    void Storage::writeBlock(std::fstream &fs, ArchiveBlock ab, size_t index) {
        fs.seekp(index * theBlockSize, fs.beg);
        fs.write((char *)&ab, theBlockSize);
    }

    //read the entire block of index into archive block ab
    void Storage::readBlock(std::fstream &fs, ArchiveBlock & ab, size_t index) {
        fs.seekg(index * theBlockSize, fs.beg);
        fs.read((char *)&ab, theBlockSize);
    }

    //empty the block at index by setting the available attribute to true
    void Storage::emptyBlock(std::fstream &fs, size_t index){
        Header h{index, true};
        ArchiveBlock ab{h};
        writeBlock(fs, ab, index);
    }

    //find the index of the first empty block starting at index
    size_t Storage::findEmptyBlock(std::fstream &fs, size_t index) {

        if( sumBlocks == 0) {
            sumBlocks += 1;
            return 0;
        }

        for(size_t i = index; i < sumBlocks; i++) {
            Header h{};
            readHeader(fs, h, i);
            if(h.available) {
                return i;
            }
        }

        //no empty block, increment number of block at the end of the filestream
        size_t tmp = sumBlocks;
        sumBlocks++;        

        return tmp;
    }

    size_t Storage::getSumBlocks() {
        return sumBlocks;
    }

    void Storage::setSumBlocks(size_t sum) {
        sumBlocks = sum;
    }
}