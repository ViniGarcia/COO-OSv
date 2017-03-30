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

namespace httpserver {

namespace api {

namespace click_plugin {

using namespace std;
using namespace json;
using namespace click_plugin_json;

static int i;

int get_value(){
    return i;
}

static std::string exec_click() {
    const std::string& cmnd_line = "/click --dpdk --no-shconf -c 0x01 -n 1 --log-level 8 -m 64 -- --allow-reconfigure -p 8001 func.click";
    bool ok;
    bool new_program = true;
    auto new_commands = osv::parse_command_line(cmnd_line, ok);
    if (!ok) {
        throw bad_param_exception("Bad formatted command");
    }
    std::string app_ids;
    for (auto cmnd: new_commands) {
        std::vector<std::string> c(cmnd.begin(), std::prev(cmnd.end()));
        auto click_main = osv::application::run(c[0], c, new_program);
        pid_t pid = click_main->get_main_thread_id();
        assert(pid != 0);
        app_ids += std::to_string(pid) + " ";
    }
    if (app_ids.size()) {
        app_ids.pop_back(); // remove trailing space
    }
    return app_ids;
}

extern "C" void init(void* arg)
{

    click_plugin_json_init_path("Click Modular Router API");

    click_version.set_handler([](const_req req){
        return "Version 0.0";
    });

    //Retorna true se click estiver rodando

    click_is_running.set_handler([](const_req req){
    	//Get info from handler?
    	int i;
        i = get_value();
        if(i == 1) return true;
        return false;
    });

    //Inicia uma funcao
    //Retorna true se sucesso?
    click_start.set_handler([](const_req req){
    	//change_value(1);
        return exec_click();
    });

    //Para a execução de uma funcao
    //Retorna true se sucesso?
    click_stop.set_handler([](const_req req){
        //change_value(0);
    	return true;
    });

    //Deve receber o texto alterado e salvar no arquivo
    //retorna true se sucesso?
    click_edit_config.set_handler([](const_req req){
    	string function = req.get_query_param("function");
    	return function;

    });

}
}
}
}
