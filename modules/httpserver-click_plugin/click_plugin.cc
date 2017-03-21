#include "click_plugin.hh"
#include "autogen/click_plugin.json.hh"

namespace httpserver {

namespace api {

namespace click_plugin {

using namespace std;
using namespace json;
using namespace click_plugin_json;

static int i;

void change_value(int j){
    i = j;
}

int get_value(){
    return i;
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
    	//ver em httpserver/api/app.cc
    	change_value(1);
        return true;
    });

    //Para a execução de uma funcao
    //Retorna true se sucesso?
    click_stop.set_handler([](const_req req){
        change_value(0);
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
