#include "ControlSocket.hh"
#include <boost/algorithm/string.hpp>

void ControlSocket::GetMessageSize(int *metaData){
	std::istringstream iterator(this->dataBuffer);
	std::string line;

	while (std::getline(iterator, line)){
		if (!strncmp(line.c_str(), "DATA", 4)){
			metaData[0] = atoi(&line[5]);
			break;
		}
	}
	metaData[1] = 0;
	while (std::getline(iterator, line)){
		metaData[1] += line.length() + 1;
	}
}

void ControlSocket::UpdateMessageSize(int *metaData){
	std::istringstream iterator(this->dataBuffer);
	std::string line;

	while (std::getline(iterator, line)){
		metaData[1] += line.length() + 1;
	}
}

ControlSocket::ControlSocket(char *HostIP, int Port){
	struct sockaddr_in serverAdress;
    struct hostent *server;

    this->controlSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (this->controlSocket < 0){
    	this->accessSuccess = false;
    	return;
    }

    server = gethostbyname(HostIP);
    if (server == NULL) {
        this->accessSuccess = false;
    	return;
    }

    bzero((char*) &serverAdress, sizeof(serverAdress));
    serverAdress.sin_family = AF_INET;
    bcopy((char*) server->h_addr, (char*) &serverAdress.sin_addr.s_addr, server->h_length);
    serverAdress.sin_port = htons(Port);
    if (connect(this->controlSocket, (struct sockaddr*) &serverAdress, sizeof(serverAdress)) < 0){
        this->accessSuccess = false;
    	return;
    }

    bzero(this->dataBuffer, 512);
    if (read(this->controlSocket, this->dataBuffer, 511) < 0){
    	this->accessSuccess = false;
    	return;
    }
    
    this->accessSuccess = true;
}

void ControlSocket::ReadManagementData(std::string query){
	int blockMetaData[2];

    if (write(this->controlSocket, ("READ " + query + "\n").c_str(), query.length() + 6) < 0){
    	this->accessSuccess = false;
    	return;
    }	

    bzero(this->dataBuffer, 512);
    if (read(this->controlSocket, this->dataBuffer, 511) < 0){
    	this->accessSuccess = false;
    	return;
    }
    if (!strncmp(this->dataBuffer, "200", 3)){
    	this->returnData = this->dataBuffer;
    	this->GetMessageSize(blockMetaData);
    	while(blockMetaData[0] > blockMetaData[1]){
    		read(this->controlSocket, this->dataBuffer, 511);
    		this->returnData.append(this->dataBuffer);
    		this->UpdateMessageSize(blockMetaData);
    	}
    }
    else{
    	this->accessSuccess = false;
    	return;
    }

    this->accessSuccess = true;
}

void ControlSocket::WriteManagementData(std::string query){
	int blockMetaData[2];

    if (write(this->controlSocket, ("WRITE " + query + "\n").c_str(), query.length() + 7) < 0){
    	this->accessSuccess = false;
    	return;
    }	

    bzero(this->dataBuffer, 512);
    if (read(this->controlSocket, this->dataBuffer, 511) < 0){
    	this->accessSuccess = false;
    	return;
    }
    if (!strncmp(this->dataBuffer, "200", 3)){
    	this->returnData = this->dataBuffer;
    }
    else{
    	this->accessSuccess = false;
    	return;
    }

    this->accessSuccess = true;
}

void ControlSocket::FreeData(std::string query){
	int blockMetaData[2] = {0,0};

    if (write(this->controlSocket, (query + "\n").c_str(), query.length() + 1) < 0){
    	this->accessSuccess = false;
    	return;
    }	

    bzero(this->dataBuffer, 512);
    if (read(this->controlSocket, this->dataBuffer, 511) < 0){
    	this->accessSuccess = false;
    	return;
    }
    if (!strncmp(this->dataBuffer, "200", 3)){
    	this->returnData = this->dataBuffer;
    	this->GetMessageSize(blockMetaData);
    	while(blockMetaData[0] > blockMetaData[1]){
    		read(this->controlSocket, this->dataBuffer, 511);
    		this->returnData.append(this->dataBuffer);
    		this->UpdateMessageSize(blockMetaData);
    	}
    }
    else{
    	this->accessSuccess = false;
    	return;
    }

    this->accessSuccess = true;
}

void ControlSocket::CloseSocket(){
	write(this->controlSocket, "QUIT", 4);
	close(this->controlSocket);
}

std::vector<std::string> ControlSocket::returnSplitData(){
    std::vector<std::string> splitData;
    split(splitData, this->returnData, boost::algorithm::is_any_of("\b\n"), boost::algorithm::token_compress_on);
    return splitData;
}


/*
int main (int argc, char *argv[]){
	std::string query;

	if (argc != 3){
		std::cout << "\n==============================================================\nUSAGE: " << argv[0] << " CONTROLSOCKET_IP CONTROLSOCKET_PORT\n==============================================================\n\n";
		return -1;
	}


	ControlSocket *socket = new ControlSocket(argv[1], atoi(argv[2]));
	for(std::cout << "\nOPTION: ", std::getline(std::cin, query); query.compare("QUIT"); std::cout << "\nOPTION: ", std::getline(std::cin, query)){
		if (!strcmp("READ", query.c_str())){
			for(std::cout << "\nREAD: ", std::getline(std::cin, query); query.compare("BACK"); std::cout << "\nREAD: ", std::getline(std::cin, query)){
				socket->ReadManagementData(query);
				std::cout << socket->returnData;
			}
		}
		if (!strcmp("WRITE", query.c_str())){
			for(std::cout << "\nWRITE: ", std::getline(std::cin, query); query.compare("BACK"); std::cout << "\nWRITE: ", std::getline(std::cin, query)){
				socket->WriteManagementData(query);
				std::cout << socket->returnData;
			}
		}
		if (!strcmp("FREE", query.c_str())){
			for(std::cout << "\nFREE: ", std::getline(std::cin, query); query.compare("BACK"); std::cout << "\nFREE: ", std::getline(std::cin, query)){
				socket->FreeData(query);
				std::cout << socket->returnData;
			}
		}
	}
	socket->CloseSocket();
}
*/