//
// Created by pc on 2024/7/22.
//

#include "config/GameServer.h"

#include "config/BAASStaticConfig.h"

using namespace std;

BAAS_NAMESPACE_BEGIN

vector<string> GameServer::valid_servers = vector<string>{};
map<string, string> GameServer::package2serverMap = map<string, string>{};

void GameServer::init() {
    assert(static_config != nullptr);

    valid_servers.clear();
    valid_servers = static_config->get("servers", vector<string>{});
    assert(!valid_servers.empty());

    package2serverMap.clear();
    package2serverMap = static_config->get("package2server", map<string, string>{});
    assert(!package2serverMap.empty());
}

BAAS_NAMESPACE_END