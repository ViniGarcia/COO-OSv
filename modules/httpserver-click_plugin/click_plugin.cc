#include "click_plugin.hh"
#include "ControlSocket.hh"
#include "VNFHeader.hh"
#include "ClickMetrics.hh"
#include "FrameworkAdapters.hh"
#include "autogen/click_plugin.json.hh"
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <osv/commands.hh>
#include <osv/app.hh>
#include <osv/sched.hh>
#include <drivers/device.hh>
#include <drivers/driver.hh>
#include <errno.h>
#include <fstream>
#include <iostream>
#include <string>
#include <streambuf>
#include <typeinfo>
#include <random>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <bsd/porting/networking.hh>

extern "C" {
void dhcp_start_except(bool wait, std::string interface);
void dhcp_release_except(std::string interface);
}

namespace httpserver {

namespace api {

namespace click_plugin {

using namespace std;
using namespace json;
using namespace click_plugin_json;

static pid_t pid = 0;
static shared_ptr<osv::application> app;
static bool running = false;
static std::string click_ver = "2.0";
static VNFHeader *header;
static ClickMetrics *clickMetrics;

static hw::device_manager *devman = hw::device_manager::instance();
static hw::driver_manager *drvman = hw::driver_manager::instance();

void start_virtio() {
    hw::hw_device_id id = hw::hw_device_id(6900, 4096);
    int num_devices = devman->get_num_devices(id);

    for(int index = 0; index < num_devices; index++){
        hw::hw_device* temp_dev = devman->get_device(id, index);
        if (!temp_dev->is_attached()){
            drvman->load_unlimited(temp_dev);
        }
    }

    bool has_if = false;
    osv::for_each_if([&has_if] (std::string if_name) {       
	if ((if_name == "lo0") || (if_name == "eth0"))
            return;
	
        has_if = true;
        if ( osv::start_if(if_name, "0.0.0.0", "255.255.255.0") != 0 ||
            osv::ifup(if_name) != 0){
            debug("Could not initialize network interface.\n");
	}
    });
    if (has_if){
        dhcp_start_except(true, "eth0");
	usleep(1000);
    }
}

void stop_virtio() {
    osv::for_each_if([] (std::string if_name) {       
	if ((if_name == "lo0") || (if_name == "eth0"))
            return;

    	osv::stop_if(if_name.c_str(), osv::if_ip(if_name.c_str()));
    });
    dhcp_release_except("eth0");
    drvman->unload_unlimited();
}

void start_click(std::vector<std::string> c) {
    if (!header->headerGet(VNF_NETWORK).compare("VirtIO")) start_virtio();

    bool new_program = true;
    app = osv::application::run(c[0], c, new_program);
    pid = app->get_main_thread_id();

    if(pid != 0){
        clickMetrics = new ClickMetrics((int)pid, false);
        running = true;
    }
}

static std::string stop_click() {
    clickMetrics->finish();
    app->request_termination();
    int th_finished = 1;
    
    sleep(1);
    sched::with_thread_by_id(pid, [&](sched::thread *t) {
        if (t && t->get_status() != sched::thread::status::terminated) {
            th_finished = 0;
        }
    });

    if(th_finished == 1){
	if (!header->headerGet(VNF_NETWORK).compare("VirtIO")) stop_virtio();
        running = false;
        return ("Success");
    }

    return ("Error while stopping");
}

void start_python2(std::vector<std::string> c) {
    if(python2_check()) return;
    python2_adapter();

    if (!header->headerGet(VNF_NETWORK).compare("VirtIO")) start_virtio();

    bool new_program = true;
    app = osv::application::run(c[0], c, new_program);
    pid = app->get_main_thread_id();

    if(pid != 0){
	clickMetrics = new ClickMetrics((int)pid, false);
        running = true;
    }
}

static std::string stop_python2() {
    int nf_stop = socket(AF_INET,SOCK_DGRAM,0);
    struct sockaddr_in nf_serv;
    nf_serv.sin_family = AF_INET;
    nf_serv.sin_port = htons(8001);
    nf_serv.sin_addr.s_addr = inet_addr("0.0.0.0");
    socklen_t nf_serv_len = sizeof(nf_serv);

    char buffer[7] = {'e', 'n', 'd', '_', 'p', 'y', '\0'};
    sendto(nf_stop, buffer, sizeof(buffer), 0, (struct sockaddr *) &nf_serv, nf_serv_len);   
    close(nf_stop);

    if (!header->headerGet(VNF_NETWORK).compare("VirtIO")) stop_virtio();

    running = false;
    return ("Success");
}

static std::string start() {
    if(running){
        return ("Framework is already running");
    }

    header = new VNFHeader("/func.click");
    if(!header->headerValidate()){
	return("Invalid VNF Header");
    }

    const std::string& cmnd_line = header->headerExecuter();
    bool ok;
    auto new_commands = osv::parse_command_line(cmnd_line, ok);
    if(!ok){
        return ("Invalid cmd line");
    }

    std::vector<std::string> c(new_commands[0].begin(), std::prev(new_commands[0].end()));
    if (!header->headerGet(VNF_FRAMEWORK).compare("Click")) start_click(c);
    else if (!header->headerGet(VNF_FRAMEWORK).compare("Python2")) start_python2(c);

    return ("Sucess: PID " + std::to_string(pid));
}

static std::string stop(){
  if(running == true){
    if (!header->headerGet(VNF_FRAMEWORK).compare("Click")) return stop_click();
    else if (!header->headerGet(VNF_FRAMEWORK).compare("Python2")) return stop_python2();
  }

  return ("Framework is not running");
}

//Return Click Metrics
static json::Metrics getMetrics(){
        using namespace std::chrono;
        httpserver::json::Metrics metrics;
        metrics.time_ms = duration_cast<milliseconds>
            (osv::clock::wall::now().time_since_epoch()).count();
        httpserver::json::Metric metric;

        metric.id = 0;
        metric.name = "CPU Usage";
        metric.value = 0;
        if(running) metric.value = clickMetrics->getCPU();
        metrics.list.push(metric);
        metric.id = 1;
        metric.name = "Disk Usage";
        if(running) metric.value = clickMetrics->getDisk();
        metrics.list.push(metric);
        metric.id = 2;
        metric.name = "Memory Usage";
        if(running) metric.value = clickMetrics->getMemory();
        metrics.list.push(metric);
        metric.id = 3;
        metric.name = "Net TX";
        metric.value = 0;
        //if(running) metric.value = clickMetrics->getNetTX();
        metrics.list.push(metric);
        metric.id = 4;
        metric.name = "Net RX";
        metric.value = 0;
        //if(running) metric.value = clickMetrics->getNetRX();
        metrics.list.push(metric);

        return metrics;
}

//Se header é validado retorna info
static json::VNF_id getVNFDesc(){
    json::VNF_id desc;
    if(running){
        if(header->headerValidate()){
            desc.id = header->headerGet(VNF_ID);
            desc.version = header->headerGet(VNF_VERSION);
            desc.name = header->headerGet(VNF_NAME);
            desc.description = header->headerGet(VNF_DESCRIPTION);
            desc.provider = header->headerGet(VNF_PROVIDER);
            desc.framework = header->headerGet(VNF_FRAMEWORK);
        }
    }else{
        desc.id = "";
        desc.version = "";
        desc.name = "";
        desc.description = "";
        desc.provider = "";
        desc.framework = "";
    }
    return desc;
}

//Read File
class click_file_reader : public handler_base {
    void handle(const std::string& path, parameters* parts,
                const http::server::request& req, http::server::reply& rep)
    override
    {
        ifstream Input("/func.click");
        if (!Input.is_open()){
            reply500(rep, "FILE NOT FOUND!!");
            return;
        }
        string InputData((std::istreambuf_iterator<char>(Input)), std::istreambuf_iterator<char>());
        rep.content.append(InputData);
        Input.close();
        set_headers(rep, "string");
    }
};

//Write File
class click_file_writer : public handler_base {
    void handle(const std::string& path, parameters* parts,
                const http::server::request& req, http::server::reply& rep)
    override
    {
        string Path = req.get_query_param("path");
        string Content = req.get_query_param("content");
        ofstream Output(Path);
        Output << Content;
        Output.close();
        rep.content.append("OK!!");
        set_headers(rep, "string");
    }
};

extern "C" void init(void* arg)
{

    click_plugin_json_init_path("Click Modular Router API");

    click_version.set_handler([](const_req req){
        return click_ver;

    });

    //Retorna true se click estiver rodando
    click_is_running.set_handler([](const_req req){
        int th_finished = 1;
        sched::with_thread_by_id(pid, [&](sched::thread *t) {
            if (t && t->get_status() != sched::thread::status::terminated) {
                th_finished = 0;
            }
        });
        if(th_finished == 1){
            running = false;
        }else{
            running = true;
        }
        return running;
    });

    //Starts click thread
    click_start.set_handler([](const_req req){
        return start();
    });

    //Stop click thread
    click_stop.set_handler([](const_req req){
        return stop();
    });

    //Return VNF Description
    click_vnf_id.set_handler([](const_req req){
        return getVNFDesc();
    });

    click_write_file.set_handler(new click_file_writer());

    click_read_file.set_handler([](const_req req){
        ifstream Input("/func.click");
        if (!Input.is_open()){
            return "Error Opening File";
        };
        string InputData((std::istreambuf_iterator<char>(Input)), std::istreambuf_iterator<char>());
        Input.close();
        return InputData.data();
    });

    //Return click log
    click_log.set_handler([](const_req req) {
        if(!running)remove("/click_errors.txt");
        ifstream Input("/click_errors.txt");
        if (!Input.is_open()){
            return "Click is not running";
        };
        string InputData((std::istreambuf_iterator<char>(Input)), std::istreambuf_iterator<char>());
        Input.close();
        return InputData.data();
    });

    //Return metrics
    click_metrics.set_handler([](const_req req) {
        return getMetrics();
    });

}
}
}
}
