//
//  Testing.hpp
//  RGAssignment2
//
//  Created by rick gessner on 1/24/21.
//

#ifndef Testing_h
#define Testing_h
#include "Archive.hpp"
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <cstring>
#include <functional>
#include <iterator>
#include <algorithm>

namespace ECE141 {
  
  struct Testing : public ArchiveObserver {
    
    //-------------------------------------------
    std::string folder;
    std::vector<std::string> stressList;

    std::string getRandomWord() {
      static std::vector<std::string> theWords = {
        std::string("class"),   std::string("happy"),
        std::string("coding"),  std::string("pattern"),
        std::string("design"),  std::string("method"),
        std::string("dyad"),    std::string("story"),
        std::string("monad"),   std::string("data"),
        std::string("compile"), std::string("debug"),
      };
      return theWords[rand() % theWords.size()];
    }
    
    void makeFile(const std::string &aFullPath, size_t aMaxSize) {
      const char* thePrefix="";
      std::ofstream theFile(aFullPath.c_str(),std::ios::trunc | std::ios::out);
      size_t theSize=0;
      size_t theCount=0;
      while(theSize<aMaxSize) {
        std::string theWord=getRandomWord();
        theFile << thePrefix;
        if(0==theCount++ % 10) theFile << "\n";
        theFile << theWord;
        thePrefix=", ";
        theSize+=theWord.size()+2;
      }
      theFile << std::endl;
      theFile.close();
    }

    void buildTestFiles() {
      makeFile(folder+"/smallA.txt",890);
      makeFile(folder+"/smallB.txt",890);
      makeFile(folder+"/mediumA.txt",1780);
      makeFile(folder+"/mediumB.txt",1780);
      makeFile(folder+"/largeA.txt",2640);
      makeFile(folder+"/largeB.txt",2640);
    }
    
    Testing(const std::string &aFolder) : folder(aFolder) {
      buildTestFiles();
    }
    
    ~Testing() {
      std::cout << "Test Version 2.7\n";
    }
        
    bool doCompileTest() {
      //just return true; if things compile at all, we're good.
      return true;
    }
    
    size_t addTestFile(Archive &anArchive,
                       const std::string &aName, char aChar='A') {
      std::string theFullPath(folder+'/'+aName+aChar+".txt");
      anArchive.add(theFullPath);
      return 1;
    }
    
    size_t addTestFiles(Archive &anArchive, char aChar='A') {
      addTestFile(anArchive, "small", aChar);
      addTestFile(anArchive, "medium", aChar);
      addTestFile(anArchive, "large", aChar);
      return 3;
    }
      
    //-------------------------------------------
    
    size_t getFileSize(const std::string &aFilePath) {
      std::ifstream theStream(aFilePath, std::ios::binary);
      const auto theBegin = theStream.tellg();
      theStream.seekg (0, std::ios::end);
      const auto theEnd = theStream.tellg();
      return theEnd-theBegin;
    }

    //-------------------------------------------
    
    bool hasMinSize(const std::string &aFilePath, size_t aMinSize) {
      size_t theSize= getFileSize(aFilePath);
      return theSize-aMinSize<=1024; //ok to have one extra block
    }
    
    //-------------------------------------------

    bool doAddTest() {
      size_t theCount=0;
      size_t theAddCount=0;
      Archive *theArchive = Archive::createArchive(folder+"/addtest");
      if(theArchive) {
        theArchive->addObserver(*this);
        theAddCount=addTestFiles(*theArchive);
        std::stringstream theStream;
        theArchive->list(theStream);
        theCount=theArchive->list(theStream);
        delete theArchive;
      }
      
      if(theCount==theAddCount) {
        std::string theArcName(folder+"/addtest.arc");
        return hasMinSize(theArcName, 6144);
      }
      
      return false;
    }
    
    //-------------------------------------------
    
    bool verifyDump(const std::string &aString) {
      std::map<std::string, size_t> theCounts;
      //std::cout << aString << "\n"; //debug
      
      std::stringstream theInput(aString);
      char theBuffer[512];
      while(!theInput.eof()) {
        theInput.getline(theBuffer, sizeof(theBuffer));
        if(strlen(theBuffer)) {
          std::string temp(theBuffer);
          std::stringstream theLineInput(temp);
          std::string theName;
          theLineInput >> theName >> theName >> theName;
          std::string thePrefix=theName.substr(0,3);
          theCounts[thePrefix]+=1;
        }
      }
      bool theResult= (theCounts["sma"]==2) &&
                      (theCounts["med"]==4) &&
                      (theCounts["lar"]==6);
      return theResult;
    }
    
    bool doDumpTest() {
      bool   theResult=false;
      Archive *theArchive = Archive::createArchive(folder+"/dumptest");
      if(theArchive) {
        theArchive->addObserver(*this);
        addTestFiles(*theArchive,'A');
        addTestFiles(*theArchive,'B');
        std::stringstream theStream;
        if(12==theArchive->debugDump(theStream)) {
          theResult=verifyDump(theStream.str());
        }
        delete theArchive;
      }
      
      if(theResult) {
        std::string theArcName(folder+"/dumptest.arc");
        theResult=hasMinSize(theArcName, 1024*12);
      }
      
      return theResult;
    }

    //-------------------------------------------
    
    bool filesMatch(const std::string &aFilename, const std::string &aFullPath) {
      std::string theFilePath(folder+"/");
      theFilePath+=aFilename;
      std::ifstream theFile1(theFilePath);
      std::ifstream theFile2(aFullPath);

      if(theFile1 && theFile2){
        std::string theLine1;
        std::string theLine2;
        while(std::getline(theFile1,theLine1)) {
          std::getline(theFile2,theLine2);
          if(theLine1!=theLine2) return false;
        }
        return true;
      }
      return false;
    }
    
    //-------------------------------------------
    
    const char* pickRandomFile() {
      static const char* theFiles[]={
        "smallA.txt","mediumA.txt","largeA.txt"};
      size_t theCount=sizeof(theFiles)/sizeof(char*);
      size_t temp=(rand() % theCount);
      return theFiles[temp];
    }
    
    bool doExtractTest() {
      bool theResult=false;
      Archive *theArchive = Archive::createArchive(folder+"/test.arc");
      if(theArchive) {
        theArchive->addObserver(*this);
        addTestFiles(*theArchive);
        std::stringstream theStream;
        std::string theFileName=pickRandomFile();
        std::string temp(folder+"/out.txt");
        theArchive->extract(theFileName, temp);
        theResult=filesMatch(theFileName,temp);
        delete theArchive;
      }
    
      return theResult;
    }

    //-------------------------------------------
    
    bool verifyAddList(const std::string &aString) {
      std::map<std::string, size_t> theCounts;
      //std::cout << aString << "\n"; //debug
      
      std::stringstream theInput(aString);
      char theBuffer[512];
      while(!theInput.eof()) {
        theInput.getline(theBuffer, sizeof(theBuffer));
        if(strlen(theBuffer)) {
          std::string temp(theBuffer);
          std::stringstream theLineInput(temp);
          std::string theName;
          theLineInput >> theName >> theName;
          std::string thePrefix=theName.substr(0,3);
          theCounts[thePrefix]+=1;
        }
      }
      bool theResult= (theCounts["sma"]==2) &&
                      (theCounts["med"]==2) &&
                      (theCounts["lar"]==2);
      return theResult;
    }
    
    bool doListTest() {
      bool theResult=false;
      Archive *theArchive = Archive::createArchive(folder+"/test");
      if(theArchive) {
        theArchive->addObserver(*this);
        addTestFiles(*theArchive,'A');
        addTestFiles(*theArchive,'B');
        std::stringstream theStream;
        theArchive->list(theStream);
        std::string theOutput=theStream.str();
        theResult=verifyAddList(theOutput);
        delete theArchive;
      }
      return theResult;
    }

    //-------------------------------------------
    
    bool verifyRemove(const std::string &aName, std::string &anOutput) {
      //std::cout << anOutput << "\n";
      std::stringstream theInput(anOutput);
      std::string theName;
      char theBuffer[512];
      
      while(!theInput.eof()) {
        theInput.getline(theBuffer, sizeof(theBuffer));
        if(strlen(theBuffer)) {
          std::string temp(theBuffer);
          std::stringstream theLineInput(temp);
          theLineInput >> theName >> theName;
          if(theName==aName) return false;
        }
      }
      
      return true;
    }
    
    bool doRemoveTest() {
      bool theResult=false;
      Archive *theArchive = Archive::createArchive(folder+"/remtest");
      if(theArchive) {
        theArchive->addObserver(*this);
        addTestFiles(*theArchive);
        std::stringstream theStream;
        std::string theFileName=pickRandomFile();
        theArchive->remove(theFileName);
        theArchive->list(theStream);
        std::string theOutput=theStream.str();
        theResult=verifyRemove(theFileName, theOutput);
        delete theArchive;
      }
      
      return theResult;
    }
    
    //--------------------------------------
    
    bool doCompactTest() {
      bool theResult=false;
      Archive *theArchive = Archive::createArchive(folder+"/compact");
      if(theArchive) {
        theArchive->addObserver(*this);
        addTestFiles(*theArchive);
        std::string theFileName("mediumA.txt");
        theArchive->remove(theFileName);
        theArchive->compact();
        
        std::stringstream theStream;
        if(theArchive->debugDump(theStream)<=5) {
          theResult=true;
        }
        
        delete theArchive;
      }
      
      if(theResult) {
        std::string theArcName(folder+"/compact.arc");
        theResult=hasMinSize(theArcName, 4096);
      }
      return theResult;
    }
    
    //-------------------------------------------

    bool verifyStressList(Archive &anArchive) {
      std::stringstream theStream;
      size_t            theCount=0;
      std::string       theName;

      anArchive.list(theStream);

      std::stringstream theInput(theStream.str());
      char theBuffer[512];
      while(!theInput.eof()) {
        theInput.getline(theBuffer, sizeof(theBuffer));
        if(strlen(theBuffer)) {
          std::string temp(theBuffer);
          std::stringstream theLineInput(temp);
          theLineInput >> theName >> theName;
          
          auto theIter = std::find(
            stressList.begin(), stressList.end(), theName);
          
          if(theIter!=stressList.end()) {
            theCount++; //we found the name in our list...
          }
        }
      } //while
      
      return theCount==stressList.size(); //names match
    }
    
    //-------------------------------------------
    
    bool stressAdd(Archive &anArchive) {
      static int counter=0;
      std::stringstream temp;
      temp << "fake" << ++counter << ".txt";
      std::string theName(temp.str());
      stressList.push_back(theName);
      const int theMin=1000;
      size_t theSize=theMin + rand() % (( 2001 ) - theMin);
      std::string theFullPath(folder+'/'+theName);
      makeFile(theFullPath, theSize);
      anArchive.add(theFullPath);
      return verifyStressList(anArchive);
    }

    //-------------------------------------------
    
    bool stressRemove(Archive &anArchive) {
      if(stressList.size()) {
        size_t theIndex= rand() % stressList.size();
        std::string theName=stressList[theIndex];
        stressList.erase(stressList.begin() + theIndex);
        anArchive.remove(theName);
        return verifyStressList(anArchive);
      }
      return true;
    }

    //-------------------------------------------
    
    bool stressExtract(Archive &anArchive) {
      if(stressList.size()) {
        std::string theOutFileName(folder+"/out.txt");
        size_t theIndex= rand() % stressList.size();
        std::string theName=stressList[theIndex];
        anArchive.extract(theName,  theOutFileName);
        return filesMatch(theName, theOutFileName);
      }
      return true;
    }

    //-------------------------------------------
    //return block count...
    size_t doStressDump(Archive &anArchive, size_t &aFreeCount) {
      size_t theBlockCount=0;
      if(stressList.size()) {
        std::stringstream theOutput;
        anArchive.debugDump(theOutput);
        
        std::stringstream theInput(theOutput.str());
        char theBuffer[512];
        std::string theStatus;
        std::string theName;

        while(!theInput.eof()) {
          theInput.getline(theBuffer, sizeof(theBuffer));
          if(strlen(theBuffer)) {
            std::string temp(theBuffer);
            std::stringstream theLineInput(temp);
            theLineInput >> theName >> theStatus >> theName;
            
            auto theIter = std::find(
                stressList.begin(), stressList.end(), theName);

            if(theStatus=="empty") {
              aFreeCount++;
              theBlockCount++;
            }
            else if(theIter!=stressList.end()) {
              theBlockCount++;
            }
          }
        }
      }
      return theBlockCount;
    }
    
    //--------------------------------------------

    bool doStressTest() {
      bool theResult=true;
      
      std::string thePath(folder+"/stresstest");
      Archive *theArchive = Archive::createArchive(thePath);
      if(theArchive) {
        theArchive->addObserver(*this);
        addTestFiles(*theArchive,'B'); //bootstrap...
        stressList.push_back("smallB.txt");
        stressList.push_back("mediumB.txt");
        stressList.push_back("largeB.txt");
    
        size_t theOpCount=500;
        static ActionType theCalls[]={
          ActionType::added,
          ActionType::removed,
          ActionType::extracted
        };

        while(theResult && theOpCount--) {
          switch(theCalls[rand() % 3]) {
            case ActionType::added:
              theResult=stressAdd(*theArchive);
              break;
            case ActionType::removed:
              theResult=stressRemove(*theArchive);
              break;
            case ActionType::extracted:
              theResult=stressExtract(*theArchive);
              break;
            default:
              break;
          }
        } //while
       
        if(theResult) {
          size_t thePreFreeCount=0;
          size_t thePreCount=doStressDump(*theArchive, thePreFreeCount);
                 
          delete theArchive; //UNCOMMENT!
          
          std::string theTempName(thePath+".arc");
          size_t thePreSize= getFileSize(theTempName);

          theArchive = Archive::openArchive(thePath); //UNCOMMENT
          
          //Final test: Dump, compact, re-dump and compare...
          if(theResult && theArchive) {
            theResult=false;

            if(size_t theBlockCount=theArchive->compact()) {
              size_t thePostFreeCount=0;
              size_t thePostCount=doStressDump(*theArchive, thePostFreeCount);
              theResult=(thePostCount<=thePreCount) &&                    (thePostFreeCount<=thePreFreeCount);
            }
            
            delete theArchive; //close compacted archive
            
            if(theResult) { //compacted file should be smaller...
              size_t thePostSize= getFileSize(theTempName);
              theResult = thePostSize<=thePreSize;
            }
          } //if
        } //if
   
      }
      return theResult;
    }
      
    //-----------------------------------------------
    
    void operator()(ActionType anAction, const std::string &aName, bool status) {
      std::cerr << "observed ";
      switch(anAction) {
        case ActionType::added: std::cerr << "add "; break;
        case ActionType::extracted: std::cerr << "extract "; break;
        case ActionType::removed: std::cerr << "remove "; break;
        case ActionType::listed: std::cerr << "list "; break;
        case ActionType::dumped: std::cerr << "dump "; break;
        case ActionType::compact: std::cerr << "dump "; break;
      }
      std::cerr << aName << "\n";
    }
    
  }; //testing

}

#endif /* Testing_h */
