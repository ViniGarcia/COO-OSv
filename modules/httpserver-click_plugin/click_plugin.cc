#include "click_plugin.hh"
#include "autogen/click_plugin.json.hh"
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <osv/commands.hh>
#include <osv/app.hh>
#include <osv/sched.hh>
#include <signal.h>
#include <sys/types.h>
#include <errno.h>
#include <fstream>
#include <iostream>
#include <string>
#include <streambuf>
#include <typeinfo>

namespace httpserver {

namespace api {

namespace click_plugin {

using namespace std;
using namespace json;
using namespace click_plugin_json;

//Needed for calls on different functions
static pid_t pid = 0;
static shared_ptr<osv::application> click_app;
static bool running = false;

//Adapted from api/run - Runs Click with default parameters
static std::string exec_click() {
    //Default command line for running DPDK
    const std::string& cmnd_line = "/click --dpdk --no-shconf -c 0x01 -n 1 --log-level 8 -m 64 -- --allow-reconfigure -p 8001 func.click";
    //Test cmd_line (useful if we offer a way to change the default cmdline)
    bool ok;
    auto new_commands = osv::parse_command_line(cmnd_line, ok);
    if(!ok){
        return ("Invalid cmd line");
    }
    if(running){
        return ("Click is already running");
    }
    //Set click to run on a new namespace
    bool new_program = true;
    //Pass commands to string vector and runs - test if pid is valid
    std::vector<std::string> c(new_commands[0].begin(), std::prev(new_commands[0].end()));
    click_app = osv::application::run(c[0], c, new_program);
    pid = click_app->get_main_thread_id();
    if(pid != 0){
        running = true;
    }
    return ("Sucess: PID " + std::to_string(pid));
}

//Handle stop request
static std::string stop_click(){
    if(running == true){
        click_app->request_termination();
        //waits for click to finish;
        sleep(1);
        int th_finished = 1;
        sched::with_thread_by_id(pid, [&](sched::thread *t) {
            if (t && t->get_status() != sched::thread::status::terminated) {
                th_finished = 0;
            }
        });
        if(th_finished == 1){
            running = false;
            return ("Success");
        }
        return ("Error while stopping");
    }
    return ("Click is not running");
}

//Read File
class click_file_reader : public handler_base {
    void handle(const std::string& path, parameters* parts,
                const http::server::request& req, http::server::reply& rep)
    override
    {
        ifstream Input(req.get_query_param("path"));
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
        return "Version 0.0";
    });

    //Retorna true se click estiver rodando

    click_is_running.set_handler([](const_req req){
        return std::to_string(running);
    });

    //Inicia uma funcao
    //Retorna true se sucesso?
    click_start.set_handler([](const_req req){
        return exec_click();
    });

    //Para a execução de uma funcao
    //Retorna true se sucesso?
    click_stop.set_handler([](const_req req){
        return stop_click();
    });

    click_read_file.set_handler(new click_file_reader());

    click_write_file.set_handler(new click_file_writer());

}
}
}
}
