#include "VNFHeader.hh"

VNFHeader::VNFHeader(){
    this->checkStatus = false;
    this->headerStart = new std::regex("//VNF_HEADER( )*");
    this->checkSequence[0] = new std::regex("//VNF_VERSION:( )*([0-9]+)(.[0-9]+)*");
    this->checkSequence[1] = new std::regex("//VNF_ID:( )*([0-9A-Fa-f]){8}-([0-9A-Fa-f]){4}-([0-9A-Fa-f]){4}-([0-9A-Fa-f]){4}-([0-9A-Fa-f]){12}");
    this->checkSequence[2] = new std::regex("//VNF_PROVIDER:( )*([0-9a-zA-Z ])*");
    this->checkSequence[3] = new std::regex("//VNF_NAME:( )*([0-9a-zA-Z ])*");
    this->checkSequence[4] = new std::regex("//VNF_RELEASE_DATE:( )*([0-9]{4})-([0-9]{2})-([0-9]{2})( ([0-9]{2})-([0-9]{2})(-([0-9]{2}))?)?");
    this->checkSequence[5] = new std::regex("//VNF_RELEASE_VERSION:( )*([0-9]+)(.[0-9]+)*");
    this->checkSequence[6] = new std::regex("//VNF_RELEASE_LIFESPAN:( )*([0-9]{4})-([0-9]{2})-([0-9]{2})( ([0-9]{2})-([0-9]{2})(-([0-9]{2}))?)?");
    this->checkSequence[7] = new std::regex("//VNF_DESCRIPTION:( )*([0-9a-zA-Z .,!/|()])*");
}

bool VNFHeader::headerValidate(std::string filePath){
    int lineCheck;
    std::string line;
    std::vector<std::string> splitLine; 
    std::ifstream inputFile;

    inputFile.open(filePath);
    if (!inputFile.is_open()){
        this->checkStatus = false;
        return false;
    }

    std::getline(inputFile, line);
    if (!std::regex_match(line, *this->headerStart)){
        inputFile.close();
        this->checkStatus = false;
        return false;
    }

    for (lineCheck = 0; ((!inputFile.eof()) && (lineCheck < 8)); lineCheck++) {
        std::getline(inputFile, line);
        if (!std::regex_match(line, *this->checkSequence[lineCheck])){
            inputFile.close();
            this->checkStatus = false;
            return false;
        }
        boost::split(splitLine, line, boost::is_any_of(":"));
        this->headerSequence[lineCheck] = splitLine[1];
        boost::trim(this->headerSequence[lineCheck]);
    }

    inputFile.close();
    if (lineCheck == 8){
        this->checkStatus = true;
        return true;
    }
    else{
        this->checkStatus = false;
        return false;
    }
}

std::string VNFHeader::headerGet(int component){

    if((this->checkStatus) && (component >= 0) && (component <= 7)){
        return this->headerSequence[component];
    }
    return ""; 
}

/*
int main(){
    VNFHeader *header = new VNFHeader();
    if(header->headerValidate("func.click")){
        std::cout << "Ok\n";
        std::cout << header->headerGet(VNF_DESCRIPTION) + "\n";
    }
    else{
        std::cout << "No\n";
        std::cout << header->headerGet(VNF_DESCRIPTION) + "\n";
    }

    return 0;
}
*/