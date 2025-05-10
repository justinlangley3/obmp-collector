/*
 * Copyright (c) 2013-2015 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 *
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#include <string>
#include <list>
#include <map>
#include <yaml-cpp/yaml.h>
#include <boost/xpressive/xpressive.hpp>
#include <boost/exception/all.hpp>
#include <sys/types.h>

#define MAX_THREADS 200

using namespace boost::xpressive;

/**
 * \class   Config
 *
 * \brief   Configuration class for openbmpd
 * \details
 *      Parses the yaml configuration file and loads value in this class instance.
 */
class Config {
public:
    u_char      c_hash_id[16];            ///< Collector Hash ID (raw format)
    char        admin_id[64];             ///< Admin ID

    std::string kafka_brokers;
    std::map<std::string, std::string> kafka_config_map;

    uint16_t    bmp_port;                 ///< BMP listening port
    std::string bind_ipv4;                ///< IP to listen on for IPv4
    std::string bind_ipv6;                ///< IP to listen on for IPv6

    int         bmp_buffer_size;          ///< BMP buffer size in bytes (min is 2M max is 128M)
    bool        svr_ipv4;                 ///< Indicates if server should listen for IPv4 connections
    bool        svr_ipv6;                 ///< Indicates if server should listen for IPv6 connections

    bool        debug_general;
    bool        debug_bgp;
    bool        debug_bmp;
    bool        debug_msgbus;

    int         heartbeat_interval;     ///< Heartbeat interval in seconds for collector updates
    std::string compression;            ///< Compression to use :none, gzip, snappy
    int         max_concurrent_routers; ///<Maximum allowed routers that can connect
    int         initial_router_time;    ///<Initial time in allowing another concurrent router
    bool        calculate_baseline;     ///<Indicates if router baseline time should be calculated
    bool        pat_enabled;            ///<Indicates if router hash needs to be based on INIT message instead of source IP

    struct match_type_regex {
        boost::xpressive::sregex  regexp;
    };

    struct match_type_ip {
        bool        isIPv4;
        uint32_t    prefix[4]  __attribute__ ((aligned));
        uint8_t     bits;
    };

    std::map<std::string, std::list<match_type_regex>> match_router_group_by_name;
    typedef std::map<std::string, std::list<match_type_regex>>::iterator match_router_group_by_name_iter;

    std::map<std::string, std::list<match_type_ip>> match_router_group_by_ip;
    typedef std::map<std::string, std::list<match_type_ip>>::iterator match_router_group_by_ip_iter;

    std::map<std::string, std::list<match_type_regex>> match_peer_group_by_name;
    typedef std::map<std::string, std::list<match_type_regex>>::iterator match_peer_group_by_name_iter;

    std::map<std::string,  std::list<match_type_ip>> match_peer_group_by_ip;
    typedef std::map<std::string, std::list<match_type_ip>>::iterator match_peer_group_by_ip_iter;

    std::map<std::string,  std::list<uint32_t>> match_peer_group_by_asn;
    typedef std::map<std::string, std::list<uint32_t>>::iterator match_peer_group_by_asn_iter;

    std::map<std::string, std::string> topic_vars_map;
    typedef std::map<std::string, std::string>::iterator topic_vars_map_iter;

    std::map<std::string, std::string> topic_names_map;
    typedef std::map<std::string, std::string>::iterator topic_names_map_iter;

    std::map<std::string, float> router_baseline_time;
    typedef std::map<std::string, float>::iterator router_baseline_time_iter;

    Config();
    void load(const char *cfg_filename);

private:
    void parseBase(const YAML::Node &node);
    void parseDebug(const YAML::Node &node);
    void parseKafka(const YAML::Node &node);
    void parseBrokers(const YAML::Node &node);
    void parseTopics(const YAML::Node &node);
    void parseMapping(const YAML::Node &node);
    void parsePrefixList(const YAML::Node &node, std::string name, std::map<std::string, std::list<match_type_ip>> &map);
    void parseRegexpList(const YAML::Node &node, std::string name, std::map<std::string, std::list<match_type_regex>> &map);
    void printWarning(const std::string msg, const YAML::Node &node);
    void topicSubstitutions();

    bool isSensitive(const std::string& key);
    void printKafkaProperties();
};

#endif /* CONFIG_H_ */

