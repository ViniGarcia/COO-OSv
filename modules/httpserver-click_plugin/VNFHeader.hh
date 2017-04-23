//Library for checking VNF Header
//Author: Vinicius F. Garcia
#ifndef VNFHEADER
#define VNFHEADER

#include <iostream>
#include <fstream>
#include <string>
#include <regex>
#include <vector>
#include <boost/algorithm/string.hpp>


#define VNF_VERSION      	 0
#define VNF_ID           	 1
#define VNF_PROVIDER     	 2
#define VNF_NAME         	 3
#define VNF_RELEASE_DATE     4 
#define VNF_RELEASE_VERSION  5
#define VNF_RELEASE_LIFESPAN 6
#define VNF_DESCRIPTION		 7

class VNFHeader {
    private:
    bool checkStatus;
    std::regex *headerStart;
    std::regex *checkSequence[8];
    std::string headerSequence[8];

    public:
    VNFHeader();
    bool headerValidate(std::string filePath);
    std::string headerGet(int component);

};

#endif
