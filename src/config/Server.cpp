//
// Created by pc on 2024/7/22.
//

#include "config/Server.h"
using namespace std;

vector<string> Server::valid_servers = vector<string> {};
map<string, string> Server::package2serverMap = map<string, string> {};

void Server::init() {
    assert(static_config != nullptr);

    valid_servers.clear();
    valid_servers = static_config->get("servers", std::vector<std::string> {});
    assert(!valid_servers.empty());

    package2serverMap.clear();
    package2serverMap = static_config->get("package2server", std::map<std::string, std::string> {});
    assert(!package2serverMap.empty());
}

