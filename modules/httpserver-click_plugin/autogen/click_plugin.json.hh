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
 * Single Metric
 */
struct Metric : public json::json_base {
    /**
     * Unique identifier for each metric
     */
  json::json_element< int > id;

    /**
     * Value of current metric
     */
  json::json_element< long > value;

    /**
     * Metric name
     */
  json::json_element< std::string > name;

void register_params() {
  add(&id,"id");
  add(&value,"value");
  add(&name,"name");

}
Metric() {
  register_params();
}
Metric(const Metric & e) {
  register_params();
  id = e.id;
  value = e.value;
  name = e.name;

}
template<class T>
Metric& operator=(const T& e) {
  id = e.id;
  value = e.value;
  name = e.name;

  return *this;
}
Metric& operator=(const Metric& e) {
  id = e.id;
  value = e.value;
  name = e.name;

  return *this;
}
template<class T>
Metric& update(T& e) {
  e.id = id;
  e.value = value;
  e.name = name;

  return *this;
}
};


/**
 * List of all metrics
 */
struct Metrics : public json::json_base {
    /**
     * Time when metrics were taken (milliseconds since epoch)
     */
  json::json_element< long > time_ms;

    /**
     * List of metric objects
     */
  json::json_list< Metric > list;

void register_params() {
  add(&time_ms,"time_ms");
  add(&list,"list");

}
Metrics() {
  register_params();
}
Metrics(const Metrics & e) {
  register_params();
  time_ms = e.time_ms;
  list = e.list;

}
template<class T>
Metrics& operator=(const T& e) {
  time_ms = e.time_ms;
  list = e.list;

  return *this;
}
Metrics& operator=(const Metrics& e) {
  time_ms = e.time_ms;
  list = e.list;

  return *this;
}
template<class T>
Metrics& update(T& e) {
  e.time_ms = time_ms;
  e.list = list;

  return *this;
}
};


/**
 * Single VNF identification value
 */
struct VNF_id : public json::json_base {
    /**
     * VNF Provider
     */
  json::json_element< std::string > provider;

    /**
     * VNF description
     */
  json::json_element< std::string > description;

    /**
     * Version of VNF
     */
  json::json_element< std::string > version;

    /**
     * VNF Unique Identifier
     */
  json::json_element< std::string > id;

    /**
     * VNF Name
     */
  json::json_element< std::string > name;

    /**
     * VNF Framework
     */
  json::json_element< std::string > framework;

void register_params() {
  add(&provider,"provider");
  add(&description,"description");
  add(&version,"version");
  add(&id,"id");
  add(&name,"name");
  add(&framework,"framework");
}
VNF_id() {
  register_params();
}
VNF_id(const VNF_id & e) {
  register_params();
  provider = e.provider;
  description = e.description;
  version = e.version;
  id = e.id;
  name = e.name;
  framework = e.framework;
}
template<class T>
VNF_id& operator=(const T& e) {
  provider = e.provider;
  description = e.description;
  version = e.version;
  id = e.id;
  name = e.name;
  framework = e.framework;
  return *this;
}
VNF_id& operator=(const VNF_id& e) {
  provider = e.provider;
  description = e.description;
  version = e.version;
  id = e.id;
  name = e.name;
  framework = e.framework;
  return *this;
}
template<class T>
VNF_id& update(T& e) {
  e.provider = provider;
  e.description = description;
  e.version = version;
  e.id = id;
  e.name = name;
  e.framework = framework;
  return *this;
}
};


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
static const path_holder click_vnf_id("click_vnf_id");
static const path_holder click_metrics("click_metrics");
static const path_holder click_log("click_log");
}
}
}
#endif //__JSON_AUTO_GENERATED_HEADERS
