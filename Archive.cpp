//
//  Archive.cpp
//  RGAssignment2
//
//  Created by rick gessner on 1/24/21.
//

#include "Archive.hpp"
#include <time.h>
#include <cstring>

#include <sstream>
namespace ECE141 {
    Archive::Archive(const std::string &aFullPath, AccessMode aMode) {
        archiveName = aFullPath;
        stream.clear();
        stream.open(aFullPath.c_str(), (std::ios_base::openmode)aMode); //open a file depending on the mode
    }

    Archive::~Archive() {
        stream.flush();
        stream.close();
    }

    // Archive Factory
    Archive* Archive::createArchive(const std::string &anArchiveName){
        std::string aName = anArchiveName;

        //check if it contains .arc, if not, add .arc extension
        if(aName.find(extension) == std::string::npos) {
            aName += extension;
        }

        Archive * temp = new Archive(aName, AccessMode::AsNew);
        if(temp->stream) {
            return temp;
        } else {
            return nullptr;
        }
    }
    Archive* Archive::openArchive(const std::string &anArchiveName){
        std::string aName = anArchiveName;

        //doesn't contain .arc extension: file is not archive file
        if(aName.find(extension) == std::string::npos) {
            return nullptr;
        }

        Archive * temp = new Archive(aName, AccessMode::AsExisting);
        if(temp->stream) {
            return temp;
        } else {
            return nullptr;
        }
    }

    //add observer to observer list
    Archive&  Archive::addObserver(ArchiveObserver &anObserver){
        observerList.push_back(&anObserver);
        return *this;
    }
    
    bool Archive::add(const std::string &aFilename){

        std::string fileName = aFilename;

        //extract filename without full path
        size_t found = aFilename.rfind("/");
        if (found!=std::string::npos) {
            fileName = aFilename.substr(found+1);
        }

        //fileName already existed in the Archive
        size_t s=0;
        if(findFile(fileName, s)) {
            notifyObserver(ActionType::added, aFilename, false);
            return false;
        }

        std::fstream input(aFilename, std::ios::in | std::ios::binary); //open the input file
        time_t t = time(NULL); //get the time

        //get the size
        input.seekg(0, input.end);
        size_t size = input.tellg();
        input.seekg(0, input.beg);

        //calculate the number of blocks needed:
        size_t numOfBlocks = size / thePayloadSize + 1;
  
        //input file is opened successfully
        if(input) { 
            //find the index of the first empty block
            size_t index = findEmptyBlock(stream, 0);
            size_t nextIndex = 0;

            for(size_t i = 0; i < numOfBlocks; i++) {

                //Last block doesn't need to find next empty block's index
                if(i < numOfBlocks - 1) {
                    nextIndex = findEmptyBlock(stream, index+1);
                }
                   
                Header h{index + i, false};
                strcpy(h.name, fileName.c_str());
                h.size = size;
                h.time = t;
                h.numOfBlocks = numOfBlocks;
                h.partnum = i;
                h.nextInd = nextIndex;
 
                //fill in block data with header and file read from input
                ArchiveBlock ab{h}; 
                //input.read((char *)&ab.data, sizeof(ab.data));
                input.read((char *)&ab.data, thePayloadSize);
                //write the block to stream at index-th block
                writeBlock(stream, ab, index);
                index = nextIndex;
            }

            input.close();
            notifyObserver(ActionType::added, aFilename, true);
            return true;
        }

        

        //input file not opened successfully -> not added successfully
        notifyObserver(ActionType::added, aFilename, false);
        return false;
    }

    bool Archive::extract(const std::string &aFilename, const std::string &aFullPath){

        //extract filename without full path
        std::string fileName = aFilename;
        size_t found = aFilename.rfind("/");
        if (found!=std::string::npos) {
            fileName = aFilename.substr(found+1);
        }

        //file not found in the Archive
        size_t ind=0;
        if(!findFile(fileName, ind)) {
            notifyObserver(ActionType::extracted, aFilename, false);
            return false;
        }

        //get outputStream ready to be written
        std::fstream output(aFullPath, std::ios::out | std::ios::binary);

        size_t index = ind; //index of the first block of the file in the archive
        ArchiveBlock ab{};

        //read the first block
        readBlock(stream, ab, index);
        size_t numOfBlocks = ab.meta.numOfBlocks;

        //iterate through blocks that contain this file
        for(size_t i = 0; i < numOfBlocks; i++) {
            output.write((char*)ab.data, sizeof(ab.data));

            //read nextblock of the file before the current block is the ending block
            if(i<numOfBlocks - 1) {
                index = ab.meta.nextInd;
                readBlock(stream, ab, index);
            }
        }
        output.close();
        notifyObserver(ActionType::extracted, aFilename, true);
        return true;
    }


    bool Archive::remove(const std::string &aFilename){
        //extract filename without full path
        std::string fileName = aFilename;
        size_t found = aFilename.rfind("/");
        if (found!=std::string::npos) {
            fileName = aFilename.substr(found+1);
        }

        //filename doesn't exist in the Archive
        size_t ind=0;
        if(!findFile(fileName, ind)) {
            notifyObserver(ActionType::removed, aFilename, false);
            return false;
        }

        size_t index = ind; // index of first block of data;
        ArchiveBlock ab{};
        readBlock(stream, ab, index);//read the first block
        size_t numOfBlocks = ab.meta.numOfBlocks;

        //iterate through the blocks that contain the file
        for(size_t i = 0; i < numOfBlocks; i++) {
            emptyBlock(stream, index); //empty the block

            //read the next block of the file only if current block is not last block
            if(i < numOfBlocks - 1) {
                index = ab.meta.nextInd;
                readBlock(stream, ab, index);
            }
        }
        notifyObserver(ActionType::removed, aFilename, true);
        return true;
    }
    
    size_t Archive::list(std::ostream &aStream){
        size_t count = 0;
         for(size_t i = 0; i < getSumBlocks(); i++) {
            Header h{};
            readHeader(stream, h, i);
            //ArchiveBlock ab{};
            //readBlock(stream, ab, i);
            //if(!ab.meta.available && ab.meta.partnum == 0) {
            if(!h.available && h.partnum == 0) { //block is used and is the first block of the file

                //convert the uint32_t time stored in header to actual time
                //time_t rawtime(ab.meta.time);
                time_t rawtime(h.time);
                struct tm * timeinfo;
                time(&rawtime);
                timeinfo = localtime (&rawtime);
                aStream << count+1 << "    " << h.name << "  " << h.size << "  " << asctime(timeinfo) << "\n";
                //aStream << count+1 << "    " << ab.meta.name << "  " << ab.meta.size << "  " << asctime(timeinfo) << "\n";
                count += 1;
            }
        }

        notifyObserver(ActionType::listed, "", true);
        return count;
    }

    size_t Archive::debugDump(std::ostream &aStream){
        //iterate over all the blocks
        for(size_t i = 0; i < getSumBlocks(); i++) {
            Header h;
            readHeader(stream, h, i);
            std::string str = h.available? "empty" : "used";
            aStream << (i+1) << ".  " << str << "   " << h.name << "\n";
        }
        notifyObserver(ActionType::dumped, "", true);
        return getSumBlocks();
    }

    size_t Archive::compact() {

        //if there's no empty block in Archive
        size_t currSumBlock = getSumBlocks();
        if(currSumBlock < findEmptyBlock(stream, 0)) {
            //recover the original sum of blocks
            //sumBlocks = currSumBlock;
            setSumBlocks(currSumBlock);
            notifyObserver(ActionType::compact, "", true);
            return getSumBlocks();
        }

        //open a temporary file 
        std::fstream tmp;
        tmp.open("tempFile.arc", std::ios::in | std::ios::out | std::ios::trunc | std::ios::binary);

        size_t count = 0; //new file number of blocks tracking
        size_t tmpInd = 0; //header block id tracking

        //iterate over all the blocks of the current arc file
        for(size_t i = 0; i < getSumBlocks(); i++) {

            //read the current block info from the file
            ArchiveBlock ab{};
            readBlock(stream, ab, i);

            //if the block is used and is the first block of the contained file
            if(!ab.meta.available && ab.meta.partnum == 0) {
                count++;
                size_t num = ab.meta.numOfBlocks;
                size_t next = ab.meta.nextInd;

                //update the header info: new id and next block index of this first block
                ab.meta.id = tmpInd;
                ab.meta.nextInd = tmpInd+1;

                //write the updated block into the temporary filestream
                writeBlock(tmp, ab, ab.meta.id);
                tmpInd++;

                //iterate through the blocks that contain this current file
                for(size_t j = 1; j < num; j++) {
                    readBlock(stream, ab, next);
                    next = ab.meta.nextInd; //record the old next index

                    //update the header info and write the updated block into the temporary file
                    ab.meta.id = tmpInd;
                    ab.meta.nextInd = ab.meta.id + 1;
                    writeBlock(tmp, ab, ab.meta.id);

                    count++;
                    tmpInd++;
                }
            }
        }
        tmp.close();
        //sumBlocks = count; //update the new block count
        setSumBlocks(count);//update the new block count

        //remove the old arc file and rename the temp.arc file with the original name and open it with stream
        stream.close();
        std::remove(archiveName.c_str());        
        std::rename("tempFile.arc", archiveName.c_str());
        stream.open(archiveName.c_str(), std::ios::in | std::ios::out | std::ios::binary);
        
        notifyObserver(ActionType::compact, "", true);
        return count;
    }

    //helper fundtion to find file with fileName it the archive and if found update index with index of the first block
    bool Archive::findFile(std::string fileName, size_t & index) {
        for(size_t i = 0; i < getSumBlocks(); i++) {
            Header h;
            readHeader(stream, h, i);
            if(!h.available && fileName == h.name && h.partnum == 0) {
                index = i;
                return true;
            }
        }
        return false;
    }

    void Archive::notifyObserver(ActionType anAction, const std::string &aName, bool status) {
        //iterate through the list of observers and notify them
        for(auto * observer: observerList) {
            (*observer)(anAction, aName, status);
        }
    }

}