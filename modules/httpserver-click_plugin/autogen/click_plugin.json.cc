/*
* Copyright (C) 2014 Cloudius Systems, Ltd.
*
* This work is open source software, licensed under the terms of the
* BSD license as described in the LICENSE f in the top-level directory.
*
*  This is an Auto-Generated-code  
*  Changes you do in this file will be erased on next code generation
*/

#include "click_plugin.json.hh"
#include "json/json_path.hh"
#include "json/api_docs.hh"

namespace httpserver {

namespace json {

void click_plugin_json_init_path(const std::string& description)
{
register_api("click_plugin", description);
    /**
     * Returns Click Info
     */
    path_description::add_path("/click_plugin/info",GET,"click_info")
    ;
}
}
}
