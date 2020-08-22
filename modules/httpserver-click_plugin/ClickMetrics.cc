#include "ClickMetrics.hh"

int threadID;
bool clickToken;

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
ClickMetrics::ClickMetrics(int id, bool click){
  using namespace std::chrono;

  this->threadID = id;
  this->clickToken = click;

  if (click){
    this->clickSocket = new ControlSocket("localhost", 8001);
    this->findInputAndOutputs();
  }

  this->prevCPUTime = duration_cast<milliseconds> (osv::clock::wall::now().time_since_epoch()).count();
  this->prevCPUTxTime = duration_cast<milliseconds> (osv::clock::wall::now().time_since_epoch()).count();
  this->prevCPURxTime = duration_cast<milliseconds> (osv::clock::wall::now().time_since_epoch()).count();
}

//Close socket (prevents OSv hanging on shutdown)
void ClickMetrics::finish(){
  if (this->clickToken) this->clickSocket->CloseSocket();
}

//Iterate over all elements in the NF, and save the ids for Inputs and Outputs
void ClickMetrics::findInputAndOutputs(){
    if (!this->clickToken) return;
    std::vector<std::string> response;

    this->clickSocket->ReadManagementData("list");
    response = clickSocket->returnSplitData();
    if(response[0].find("200") != std::string::npos){
        int total_elements = std::stoi(response[2]);
        for(int i = 1 ; i <= total_elements ; i++){
            this->clickSocket->ReadManagementData(std::to_string(i)+".in_bytes");
            if(this->clickSocket->accessSuccess){
                this->rx_ids.push_back(i);
            }
        }

        for(int i = 1 ; i <= total_elements ; i++){
            this->clickSocket->ReadManagementData(std::to_string(i)+".out_bytes");
            if(this->clickSocket->accessSuccess){
                this->tx_ids.push_back(i);
            }
        }
    }
}

static httpserver::json::Interface get_interface(const std::string& name, ifnet* ifp, long time)
{
    httpserver::json::Interface_config ifc;
    httpserver::json::Interface_data ifd;
    httpserver::json::Interface f;
    osv::network::interface intf(name);


    if_data cur_data = { 0 };
    if (!set_interface_info(ifp, cur_data, intf)) {
        return f;
    }

    ifc = intf;
    f.config = ifc;
    ifd = cur_data;
    f.data = ifd;
    f.time = time;
    return f;
}

void ifconfig(std::vector<httpserver::json::Interface> ifaces){
    auto time = std::chrono::duration_cast<std::chrono::microseconds>
                    (osv::clock::uptime::now().time_since_epoch()).count();

    for (unsigned int i = 0; i <= osv::network::number_of_interfaces(); i++) {
        auto* ifp = osv::network::get_interface_by_index(i);

        if (ifp != nullptr) {
            ifaces.push_back(get_interface(osv::network::get_interface_name(ifp), ifp, time));
        }
    }
}

void ClickMetrics::genericInOutBytes(long *aggregate){
    std::vector<httpserver::json::Interface> ifaces;

    aggregate[0] = 0;
    aggregate[1] = 0;

    ifconfig(ifaces);
    for (auto &i : ifaces){
	aggregate[0] = aggregate[0] + i.data().ifi_ibytes();
	aggregate[1] = aggregate[1] + i.data().ifi_obytes();
    }
}

void ClickMetrics::clickInOutBytes(long *aggregate){
    std::vector<std::string> returnValue;	
    
    for(int i = 0; i < rx_ids.size(); i++){
        this->clickSocket->ReadManagementData(std::to_string(rx_ids[i])+".in_bytes");
        if(this->clickSocket->accessSuccess){
            returnValue = clickSocket->returnSplitData();
            aggregate[0] = aggregate[0] + std::stoll(returnValue[2]);
        }
    }
    for(int i = 0; i < tx_ids.size(); i++){
        this->clickSocket->ReadManagementData(std::to_string(tx_ids[i])+".out_bytes");
        if(this->clickSocket->accessSuccess){
            returnValue = this->clickSocket->returnSplitData();
            aggregate[1] += std::stoll(returnValue[2]);
        }
    }
}

/*
* Get the mean bit rate from all inputs
* Click can only return the total input and output bytes for each interface, so we
* subtract this value from the previous value. Same thing with CPU Time. After we divide
* the bytes by the ms between each request, and finally divide by a default Link Speed(1Gbit)
*/
long ClickMetrics::getNetRX(long rxValue){
    using namespace std::chrono;

    long curCPURxTime = duration_cast<milliseconds> (osv::clock::wall::now().time_since_epoch()).count();
    long cpuResult = curCPURxTime - this->prevCPURxTime;
    this->prevCPURxTime = curCPURxTime;
  
    long rxResult = rxValue - this->prevRXValue;
    this->prevRXValue = rxValue;
    
    if (cpuResult > 0){
        long finalResult = ((rxResult / cpuResult) / (double) 10000)*100;
        if(finalResult > 100) return 100;
        else return finalResult;
    }
    return 0;
}

/*
* Get the mean bit rate from all outputs
* Click can only return the total input and output bytes for each interface, so we
* subtract this value from the previous value. Same thing with CPU Time. After we divide
* the bytes by the ms between each request, and finally divide by a default Link Speed(1Gbit)
*/
long ClickMetrics::getNetTX(long txValue){
    using namespace std::chrono;

    long curCPUTxTime = duration_cast<milliseconds> (osv::clock::wall::now().time_since_epoch()).count();
    long cpuResult = curCPUTxTime - this->prevCPUTxTime;
    this->prevCPUTxTime = curCPUTxTime;

    long txResult = txValue - this->prevTXValue;
    this->prevTXValue = txValue;
    
    if (cpuResult > 0){
        long finalResult = ((txResult / cpuResult) / (double) 10000)*100;
        if(finalResult>100) return 100;
        else return finalResult;
    }
    return 0;
}

/*
* Get CPUTime used by Click thread. Based on OSv /os/threads function.
*/
int ClickMetrics::getCPU(){
  using namespace std::chrono;

  long clickTime, cpuResult, clickResult;
  long value_now = duration_cast<milliseconds> (osv::clock::wall::now().time_since_epoch()).count();
  sched::with_thread_by_id(this->threadID, [&](sched::thread *t) {clickTime = duration_cast<milliseconds>(t->thread_clock()).count();});

  cpuResult = value_now - this->prevCPUTime;
  this->prevCPUTime = value_now;
  clickResult = clickTime - this->prevClickTime;
  this->prevClickTime = clickTime;
  return (((double) clickResult / cpuResult) * 100);
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

/*
*Cant get memory for a single thread, since in osv everything is in the same address space.
*Thus, it gets the total memory size.
*/
int ClickMetrics::getMemory(){
	struct sysinfo info;

	sysinfo(&info);
	return (100 - (((double) info.freeram / (double) info.totalram) * 100));
};
