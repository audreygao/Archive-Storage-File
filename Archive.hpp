//
//  Archive.hpp
//  RGAssignment2
//
//  Created by rick gessner on 1/24/21.
//

#ifndef Archive_hpp
#define Archive_hpp

#include <stdio.h>
#include <iostream>
#include <fstream>
#include "Storage.hpp"
#include <vector>

namespace ECE141 {

  const std::string extension = ".arc";

  const int OpenNew = std::ios::in | std::ios::out | std::ios::trunc | std::ios::binary;
  const int OpenExisting = std::ios::in | std::ios::out | std::ios::app | std::ios::binary;
  
  enum class ActionType {added, extracted, removed, listed, dumped, compact};
  enum class AccessMode {AsNew=OpenNew, AsExisting=OpenExisting};

  struct ArchiveObserver {
    virtual void operator()(ActionType anAction,
                    const std::string &aName, bool status)=0;
  };
  
  //---------------------------------------------------

  class Archive : public Storage {
  protected:
              //protected on purpose!
              Archive(const std::string &aName, AccessMode);

  public:

              ~Archive();  //

    static    Archive* createArchive(const std::string &anArchiveName);
    static    Archive* openArchive(const std::string &anArchiveName);

    Archive&  addObserver(ArchiveObserver &anObserver);
    
    bool      add(const std::string &aFilename);
    bool      extract(const std::string &aFilename, const std::string &aFullPath);
    bool      remove(const std::string &aFilename);
    
    size_t    list(std::ostream &aStream);
    size_t    debugDump(std::ostream &aStream);

    size_t    compact();

    bool      findFile(std::string fileName, size_t & index);
    void      notifyObserver(ActionType anAction, const std::string &aName, bool status);

    std::string   archiveName;
    std::fstream stream;
    std::vector<ArchiveObserver*> observerList;
      
      
  };

}

#endif /* Archive_hpp */
