#include "ClickMetrics.hh"

int threadID;
//Needed for calculing Usage
long prevCPUTime = 0;
long prevCPUTxTime = 0;
long prevCPURxTime = 0;
long prevClickTime = 0;
long prevTXValue = 0;
long prevRXValue = 0;
//Socket which gets data from Click
ControlSocket* clickSocket;
std::vector<std::string> returnData;
//Lists with id from inputs and outputs
std::vector<int> rx_ids;
std::vector<int> tx_ids;

//Initialize timers and connection with Click
//This needs to be called only after Click is running
ClickMetrics::ClickMetrics(int id){
	using namespace std::chrono;
	threadID = id;
	clickSocket = new ControlSocket("localhost", 8001);
	//if(clickSocket->accessSuccess)printf("Connect OK\n");
	findInputAndOutputs();
	prevCPUTime = duration_cast<milliseconds> (osv::clock::wall::now().time_since_epoch()).count();
	prevCPUTxTime = duration_cast<milliseconds> (osv::clock::wall::now().time_since_epoch()).count();
	prevCPURxTime = duration_cast<milliseconds> (osv::clock::wall::now().time_since_epoch()).count();

}

//Close socket (prevents OSv hanging on shutdown)
void ClickMetrics::finish(){
	clickSocket->CloseSocket();
}

//Iterate over all elements in the NF, and save the ids for Inputs and Outputs
void ClickMetrics::findInputAndOutputs(){
	std::vector<std::string> response;
	clickSocket->ReadManagementData("list");
	response = clickSocket->returnSplitData();
	if(response[0].find("200") != std::string::npos){
		int total_elements = std::stoi(response[2]);
		for(int i = 1 ; i <= total_elements ; i++){
			clickSocket->ReadManagementData(std::to_string(i)+".in_bytes");
			if(clickSocket->accessSuccess){
				rx_ids.push_back(i);
			}
		};
		for(int i = 1 ; i <= total_elements ; i++){
			clickSocket->ReadManagementData(std::to_string(i)+".out_bytes");
			if(clickSocket->accessSuccess){
				tx_ids.push_back(i);
			}
		}
	}

}


/*
* Get the mean bit rate from all inputs
* Click can only return the total input and output bytes for each interface, so we
* subtract this value from the previous value. Same thing with CPU Time. After we divide
* the bytes by the ms between each request, and finally divide by a default Link Speed(1Gbit)
*/
long ClickMetrics::getNetRX(){
	using namespace std::chrono;
	std::vector<std::string> response;
	unsigned long long rxValue, rxResult;
	long cpuResult;
	long value_now = duration_cast<milliseconds> (osv::clock::wall::now().time_since_epoch()).count();
	for(int i = 0; i < rx_ids.size(); i++){
		clickSocket->ReadManagementData(std::to_string(rx_ids[i])+".in_bytes");
		if(clickSocket->accessSuccess){
			response = clickSocket->returnSplitData();
			rxValue += std::stoll(response[2]);
		}
	}
	cpuResult = ((value_now - prevCPURxTime));
	prevCPURxTime = value_now;
	rxResult = rxValue - prevRXValue;
	prevRXValue = rxValue;
	//std::cout << "RX" << rxResult << "CPU" << cpuResult << "\n";
	//Bytes/s
	return ((rxResult / cpuResult) / (double) 150000)*100;
}

/*
* Get the mean bit rate from all outputs
* Click can only return the total input and output bytes for each interface, so we
* subtract this value from the previous value. Same thing with CPU Time. After we divide
* the bytes by the ms between each request, and finally divide by a default Link Speed(1Gbit)
*/
long ClickMetrics::getNetTX(){
	using namespace std::chrono;
	std::vector<std::string> response;
	unsigned long long txValue, txResult;
	long cpuResult;
	long value_now = duration_cast<milliseconds> (osv::clock::wall::now().time_since_epoch()).count();
	for(int i = 0; i < tx_ids.size(); i++){
		clickSocket->ReadManagementData(std::to_string(tx_ids[i])+".out_bytes");
		if(clickSocket->accessSuccess){
			response = clickSocket->returnSplitData();
			txValue += std::stoll(response[2]);
		}
	}
	cpuResult = ((value_now - prevCPUTxTime));
	prevCPUTxTime = value_now;
	txResult = txValue - prevTXValue;
	prevTXValue = txValue;
	//std::cout << "TX" << (txResult / cpuResult) << "CPU" << cpuResult << "\n";
	return ((txResult / cpuResult) / (double) 150000)*100;
}

/*
* Get CPUTime used by Click thread. Based on OSv /os/threads function.
*/
int ClickMetrics::getCPU(){
	using namespace std::chrono;
	long clickTime,cpuResult,clickResult;
	long value_now = duration_cast<milliseconds> (osv::clock::wall::now().time_since_epoch()).count();
	sched::with_thread_by_id(threadID, [&](sched::thread *t) {
		clickTime = duration_cast<milliseconds>(t->thread_clock()).count();  
    });
    cpuResult = value_now - prevCPUTime;
    prevCPUTime = value_now;
    clickResult = clickTime - prevClickTime;
    prevClickTime = clickTime;
    //std::cout << "CPU_TIME" << cpuResult << "C_RESULT" << clickResult << "\n";
    return (((double) clickResult / cpuResult) * 100);
    //return 0;
}

/*
*Returns the percentage of disk used on the VM. The total size is defined on build,
*does not mean the image will be that size, since its dinamically allocated.
*/
int ClickMetrics::getDisk(){
	struct statfs st;
    for (osv::mount_desc mount : osv::current_mounts()) {
    	if (mount.type == "zfs") {
    		if (statfs(mount.path.c_str(),&st) == 0) {
    			struct statfs& temp = st;
				return (100 - (((double) temp.f_bfree / (double) temp.f_blocks) * 100));
    		}
    	}
    }
    return 0;
};

//Cant get memory for a single thread, since in osv everything is in the same address space
//Get total memory size then
int ClickMetrics::getMemory(){
	struct sysinfo info;
	sysinfo(&info);
	return (100 - (((double) info.freeram / (double) info.totalram) * 100));
};