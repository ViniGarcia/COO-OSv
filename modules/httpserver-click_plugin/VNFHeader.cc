#include "VNFHeader.hh"

VNFHeader::VNFHeader(std::string inputFilePath){
    this->filePath = inputFilePath;
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
    this->checkSequence[8] = new std::regex("//VNF_FRAMEWORK:( )*([0-9a-zA-Z .,!/|()])*");
    this->checkSequence[9] = new std::regex("//VNF_NETWORK:( )*([0-9a-zA-Z .,!/|()])*");
}

bool VNFHeader::headerValidate(){
    int lineCheck;
    std::string line;
    std::vector<std::string> splitLine; 
    std::ifstream inputFile; 

    if (this->checkStatus){
	return true;
    }

    inputFile.open(this->filePath);
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

    for (lineCheck = 0; ((!inputFile.eof()) && (lineCheck < 10)); lineCheck++) {
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

    if (lineCheck == 10){
	if((this->headerSequence[8].compare("Click")) && (this->headerSequence[8].compare("Python2"))){
	    this->checkStatus = false;
	    return false;
        }

        if((this->headerSequence[9].compare("VirtIO")) && (this->headerSequence[9].compare("DPDK"))){
	    this->checkStatus = false;
	    return false;
        }
	
	if (!this->headerSequence[8].compare("Python2")){
	    if(!this->headerSequence[9].compare("DPDK")){
	        this->checkStatus = false;
		return false;
	    }
	}

        this->checkStatus = true;
	
	std::string functionData;
	while(!inputFile.eof()){
	    std::getline(inputFile, line);
            functionData.append(line);
            functionData.append("\n");
    	}
        inputFile.close();
	std::ofstream funcExe;
        funcExe.open("/func.exe", std::ofstream::trunc);
        funcExe << functionData;
        funcExe.close();
        return true;
    }
    else{
        inputFile.close();
        this->checkStatus = false;
        return false;
    }
}

std::string VNFHeader::headerGet(int component){

    if((this->checkStatus) && (component >= 0) && (component <= 9)){
        return this->headerSequence[component];
    }
    return ""; 
}

std::string VNFHeader::headerExecuter(){
    if(!this->checkStatus) return "";
    if(!this->headerSequence[8].compare("Click")){
	if(!this->headerSequence[9].compare("DPDK")){ 
	    return "/click --dpdk --no-shconf -c 0x01 -n 1 --log-level 8 -m 64 -- --allow-reconfigure -p 8001 func.exe";
	}
	return "/click --allow-reconfigure -p 8001 func.exe";
    }
    if(!this->headerSequence[8].compare("Python2")) return "/python func.exe";
    return "";
}
