#include "click_plugin.hh"
#include "autogen/click_plugin.json.hh"

namespace httpserver {

namespace api {

namespace click_plugin {

using namespace std;
using namespace json;
using namespace click_plugin_json;

extern "C" void init(void* arg)
{

    click_plugin_json_init_path("Click API");

    click_info.set_handler([](const_req req){
        return "Click Modular Router";
    });

    

}

}
}
}
