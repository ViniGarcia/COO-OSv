//API em c++ para conectar a um ControlSocket do Click
//Author: Vinicius F. Garcia

#ifndef ClassControlSocket
#define ClassControlSocket

#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 


class ControlSocket{
 public:
 	bool accessSuccess;
 	std::string returnData;
 private:
 	int controlSocket;
 	char dataBuffer[512];

private:
	void GetMessageSize(int *metaData);
	void UpdateMessageSize(int *metaData);

 public:
 	ControlSocket(char *HostIP, int Port);
 	void ReadManagementData(std::string query);
 	void WriteManagementData(std::string query);
 	void FreeData(std::string query);
 	void CloseSocket();
 	std::vector<std::string> returnSplitData();
};


#endif