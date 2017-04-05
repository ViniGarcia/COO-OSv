/*
* Copyright (C) 2014 Cloudius Systems, Ltd.
*
* This work is open source software, licensed under the terms of the
* BSD license as described in the LICENSE f in the top-level directory.
*
*  This is an Auto-Generated-code  
*  Changes you do in this file will be erased on next code generation
*/

#ifndef __JSON_AUTO_GENERATED_click_plugin_json
#define __JSON_AUTO_GENERATED_click_plugin_json

#include <string>
#include "json/json_elements.hh"
#include "path_holder.hh"

namespace httpserver {

namespace json {

/**
 * Initialize the path
 */
void click_plugin_json_init_path(const std::string& description);
namespace click_plugin_json {

static const path_holder click_version("click_version");
static const path_holder click_is_running("click_is_running");
static const path_holder click_start("click_start");
static const path_holder click_stop("click_stop");
static const path_holder click_read_file("click_read_file");
static const path_holder click_write_file("click_write_file");
}
}
}
#endif //__JSON_AUTO_GENERATED_HEADERS
