#ifndef HTTP_CLICK_PLUGIN
#define HTTP_CLICK_PLUGIN

#include "routes.hh"

namespace httpserver {

namespace api {

namespace click_plugin {

/**
 * Initialize the routes object with specific routes mapping
 * @param routes - the routes object to fill
 */
void init(routes& routes);

}
}
}



#endif /* HTTP_CLICK_PLUGIN */