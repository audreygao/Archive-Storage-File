//
//  main.cpp
//  RGAssignment2
//
//  Created by rick gessner on 1/24/21.
//

#include <iostream>
#include "Testing.hpp"
#include <cstdlib>
#include <ctime>  

std::string getLocalFolder() {
  return std::string("/tmp"); //PATH TO LOCAL ARCHIVE FOLDER
}
  
int main(int argc, const char * argv[]) {
    
  const char* theStatus[]={"FAIL","PASS"};
    
  // insert code here...
  if(argc>1) {
    
    std::string theFolder;
    theFolder=(3==argc) ? argv[2] : getLocalFolder();
    
    ECE141::Testing theTests(theFolder);
                           
    srand(time(0));  
      
    std::string temp(argv[1]);
    if("compile"==temp) {
      std::cout << "Compile test " << theStatus[theTests.doCompileTest()] << ".\n";
    }
    else if("add"==temp) {
      std::cout << "Add test " << theStatus[theTests.doAddTest()] << ".\n";
    }
    else if ("dump"==temp) {
      std::cout << "Dump test "
        << theStatus[theTests.doDumpTest()] << ".\n";
    }
    else if ("extract"==temp) {
      std::cout << "Extract test " << theStatus[theTests.doExtractTest()] << ".\n";
    }
    else if ("list"==temp) {
      std::cout << "List test " << theStatus[theTests.doListTest()] << ".\n";
    }             
    else if ("remove"==temp) {
      std::cout << "Remove test "    
        << theStatus[theTests.doRemoveTest()] << ".\n";
    }
    else if ("compact"==temp) {
      std::cout << "Compact test "
        << theStatus[theTests.doCompactTest()] << ".\n";
    }    
    else if ("stress"==temp) {
      std::cout << "Stress test "
        << theStatus[theTests.doStressTest()] << ".\n";
    }
  } 
  
  return 0;
}
