//
// Created by pc on 2024/4/12.
//
#include <string>

#include "config/BAASConfig.h"
#include "BAASGlobals.h"

using namespace std;
using json = nlohmann::json;

BAAS_NAMESPACE_BEGIN

BAASConfig::BAASConfig(
        const json& j,
        BAASLogger* logger
)
{
    assert(logger != nullptr);
    this->logger = logger;
    config = j;
}


BAASConfig::BAASConfig(
        const string& path,
        BAASLogger* logger
)
{
    assert(logger != nullptr);
    this->logger = logger;
    this->path = path;
    if (filesystem::exists(path)) {
        try {
            std::ifstream file(path);
            config = json::parse(file);
            BAASGlobalLogger->sub_title("Config Load");
            BAASGlobalLogger->Path(path);
        }
        catch (json::parse_error &e) {
            logger->BAASError("Config");
            logger->Path(path, 3);
            logger->BAASError("parse error : " + string(e.what()));
        }
    } else {
        logger->sub_title("Config Not Exist");
        logger->Path(path);
        throw PathError("Config file not exist.");
    }
}

BAASConfig::BAASConfig(
        const filesystem::path &path,
        BAASLogger *logger
)
{
    assert(logger != nullptr);
    this->logger = logger;
    this->path = path;
    if (filesystem::exists(path)) {
        try {
            std::ifstream file(path);
            config = json::parse(file);
            BAASGlobalLogger->sub_title("Config Load");
            BAASGlobalLogger->Path(path);
        }
        catch (json::parse_error &e) {
            logger->BAASError("Config");
            logger->Path(path, 3);
            logger->BAASError("parse error : " + string(e.what()));
        }
    } else {
        logger->sub_title("Config Not Exist");
        logger->Path(path);
        throw PathError("Config file not exist.");
    }
}

BAASConfig::BAASConfig(const int config_type)
{
    path = BAAS_PROJECT_DIR / "resource";
    switch (config_type) {
        case CONFIG_TYPE_STATIC_CONFIG:
            path = path / "static.json";
            break;
        case CONFIG_TYPE_DEFAULT_CONFIG:
            path = path / "default_config.json";
            break;
        case CONFIG_TYPE_CONFIG_NAME_CHANGE:
            path = path / "config_name_change.json";
            break;
        case CONFIG_TYPE_DEFAULT_GLOBAL_SETTING:
            path = path / "global_setting.json";
            break;
        default:
            throw ValueError("Invalid config type : [ " + to_string(config_type) + " ].");
    }
    if (filesystem::exists(path)) {
        try {
            std::ifstream file(path);
            config = json::parse(file);
            logger = (BAASLogger *) (BAASGlobalLogger);
            BAASGlobalLogger->sub_title("Config Load");
            BAASGlobalLogger->Path(path);
        }
        catch (json::parse_error &e) {
            BAASGlobalLogger->BAASError("Config");
            BAASGlobalLogger->Path(path, 4);
            BAASGlobalLogger->BAASError("parse error : " + string(e.what()));
            throw ValueError("Config parse error");
        }
    } else {
        BAASGlobalLogger->sub_title("Config Not Exist");
        BAASGlobalLogger->Path(path);
        throw PathError("Config file not exist.");
    }
}

BAASConfig::BAASConfig(const filesystem::path& path)
{
    _check_p(path);
    _init_config();
    _init_modify_history();

    _preprocess_value();
    save();
}

BAASConfig::BAASConfig(const string& path)
{
    std::filesystem::path config_path = path;
    _check_p(config_path);
    _init_config();
    _init_modify_history();

    _preprocess_value();
    save();
}

void BAASConfig::save()
{
    std::ofstream file(path);
    file << config.dump(4);
    file.close();
    save_modify_history();
}

void BAASConfig::_preprocess(
        string &jp,
        nlohmann::json &value
)
{
    switch (value.type()) {
        case json::value_t::object:
            for (auto it = value.begin(); it != value.end(); it++) {
                int jp_len = int(jp.size());
                jp += '/' + it.key();
                _preprocess(jp, *it);
                jp.resize(jp_len);
            }
            break;
        case json::value_t::array:
            for (auto it = value.begin(); it != value.end(); it++) {
                int jp_len = int(jp.size());
                jp += '/' + to_string(it - value.begin());
                _preprocess(jp, *it);
                jp.resize(jp_len);
            }
            break;
        case json::value_t::string: {
            string str = value.template get<string>();
            if (str.empty()) return;
            if (str == "true") {
                value = true;
                modified.push_back(
                        {{"op",    "replace"},
                         {"path",  jp},
                         {"value", value}}
                );
            } else if (str == "false") {
                value = false;
                modified.push_back(
                        {{"op",    "replace"},
                         {"path",  jp},
                         {"value", value}}
                );
            } else if (str == "None" or str == "null") {
                value = nullptr;
                modified.push_back(
                        {{"op",    "replace"},
                         {"path",  jp},
                         {"value", value}}
                );
            } else {
                // try to convert to number
                if (BAASUtil::allNumberChar(str)) {
                    if (str.find('.') == string::npos) {
                        try {
                            value = stoi(str);
                            modified.push_back(
                                    {{"op",    "replace"},
                                     {"path",  jp},
                                     {"value", value}}
                            );
                        } catch (invalid_argument &e) {}
                    } else {
                        try {
                            value = stod(str);
                            modified.push_back(
                                    {{"op",    "replace"},
                                     {"path",  jp},
                                     {"value", value}}
                            );
                        }
                        catch (invalid_argument &e) {}
                    }
                }
            }
            break;
        }
        case json::value_t::null:
        case json::value_t::boolean:
        case json::value_t::number_integer:
        case json::value_t::number_unsigned:
        case json::value_t::number_float:
        case json::value_t::binary:
        case json::value_t::discarded:
        default: {
            return;
        }
    }
}


void BAASConfig::remove(const string &key)
{
    assert(!key.empty());
    if (!count(key.begin(), key.end(), '/')) {
        auto it = findByKey(key);
        config.erase(it);
    } else {
        vector<string> keys;
        BAASUtil::stringSplit(key, '/', keys);
        json &tar = config;
        int siz = int(keys.size()) - 1;
        for (int i = 0; i <= siz - 1; ++i) {
            if (tar.is_object()) {
                auto it = tar.find(keys[i]);
                if (it != tar.end())tar = *it;
                else throw_key_error("Key : [ " + key + " ] not fount.");
            } else if (tar.is_array()) {
                try {
                    int idx = stoi(keys[i]);
                    tar = tar.at(idx);
                } catch (exception &e) { throw_key_error(e.what()); }
            } else throw_key_error("key : [ " + key + " ] not found.");

        }

        if (tar.is_object()) {
            if (tar.contains(keys[siz])) tar.erase(keys[siz]);
            else throw_key_error("key : [ " + key + " ] not found.");
        } else if (tar.is_array()) {
            try {
                int idx = stoi(keys[siz]);
                tar.erase(idx);
            } catch (exception &e) { throw_key_error(e.what()); }
        } else throw_key_error("key : [ " + key + " ] not found.");
    }
    modified.push_back(
            {{"op",   "remove"},
             {"path", key}}
    );
}

void BAASConfig::show(
        int indent,
        bool ensure_ascii
) const
{
    cout << config.dump(indent, ' ', ensure_ascii) << endl;
}

void BAASConfig::show_modify_history()
{
    cout << modified.dump(4) << endl;
}

void BAASConfig::diff(
        json &j,
        nlohmann::json &result
)
{
    result = json::diff(j, config);
}

void BAASConfig::clear() noexcept
{
    config.clear();
}

void BAASConfig::replace_all(json &new_config)
{
    config = new_config;
}

void BAASConfig::my_flatten()
{
    json res = json::object({});
    string jp;
    modified.push_back({{"op", "flatten"}});
    flatten(jp, config, res);
    config = res;
}

void BAASConfig::flatten(
        string &jp,
        json &tar,
        json &result
)
{
    switch (tar.type()) {
        case json::value_t::object:
            if (tar.empty()) {
                result[jp] = nullptr;
                break;
            } else {
                for (auto it = tar.begin(); it != tar.end(); it++) {
                    int jp_len = int(jp.size());
                    jp += '/' + it.key();
                    flatten(jp, *it, result);
                    jp.resize(jp_len);
                }
            }
            break;
        case json::value_t::array:
        case json::value_t::null:
        case json::value_t::string:
        case json::value_t::boolean:
        case json::value_t::number_integer:
        case json::value_t::number_unsigned:
        case json::value_t::number_float:
        case json::value_t::binary:
        case json::value_t::discarded:
        default:
            result[jp] = tar;
    }
}

void BAASConfig::my_unflatten()
{
    modified.push_back({{"op", "unflatten"}});
    unflatten(config);
}

void BAASConfig::unflatten(json &value)
{
    json result = json::object({});
    for (auto it = value.begin(); it != value.end(); it++) {
        assert(it.key()
                 .front() == '/');
        assert(!it->is_object());
        result[json::json_pointer(it.key())] = it.value();
    }
    value = result;
}

void BAASConfig::_get_logger()
{
    logger = BAASLogger::get(config_name);
}

// path : "name/config.json"
void BAASConfig::_check_p(const std::filesystem::path &_p)
{
    config_name = _p.parent_path().string();
    _get_logger();
    config_type = _p.filename().string();

    if (!_p.filename().string().ends_with(".json"))
        throw ValueError("Config file must be a json file : [ " + _p.string() + " ]");
    this->path = BAAS_CONFIG_DIR / _p;
    if (!std::filesystem::exists(this->path)) {
        BAASGlobalLogger->sub_title("Config Not Exist");
        BAASGlobalLogger->Path(this->path);
        throw PathError("Config file not exist.");
    }

}

void BAASConfig::_init_modify_history()
{
    // save in "output/{date}/config_modify_history/name/config.json" date is generated by global logger
    modify_history_path = GlobalLogger::get_folder_path() / "config_modify_history";
    modified = json::array({});
    if (!std::filesystem::exists(modify_history_path)) {
        logger->sub_title("Config Modify History Dir");
        logger->Path(modify_history_path);
        std::filesystem::create_directory(modify_history_path);
    }
    modify_history_path.append(config_name);
    std::filesystem::create_directory(modify_history_path);
    modify_history_path.append(config_type);
    std::ofstream modify_record_file(modify_history_path);
    modify_record_file << json::object({}).dump(4);
    modify_record_file.close();
}

void BAASConfig::_init_config()
{
    try {
        std::ifstream file(this->path);
        config = json::parse(file);
        BAASGlobalLogger->sub_title("Config Load");
        BAASGlobalLogger->Path(this->path);
    }
    catch (json::parse_error &e) {
        BAASGlobalLogger->BAASError("Config");
        BAASGlobalLogger->Path(path, 3);
        BAASGlobalLogger->BAASError("parse error : " + string(e.what()));
        throw ValueError("Config Parse Error.");
    }
}


BAASConfig *config_name_change = nullptr;
BAASConfig *default_global_setting = nullptr;

BAAS_NAMESPACE_END