//
// Created by pc on 2024/4/12.
// 86310854

#ifndef BAAS_CONFIG_BAASCONFIG_H_
#define BAAS_CONFIG_BAASCONFIG_H_

#define CONFIG_TYPE_STATIC_CONFIG 0
#define CONFIG_TYPE_DEFAULT_CONFIG 1
#define CONFIG_TYPE_CONFIG_NAME_CHANGE 2

#include <filesystem>
#include <vector>
#include <mutex>
#include <fstream>
#include <regex>

#include "nlohmann/json.hpp"

#include "BAASLogger.h"
#include "BAASUtil.h"
#include "BAASExceptions.h"

class BAASConfig {
public:
    /*
     *  create a simple json read and write config
     */
    explicit BAASConfig(const std::string &path, BAASLogger *logger);

    /* special config files which are read only
     *  use global logger
     */
    explicit BAASConfig(const int config_type);

    /* config_name / config.json or event.json
     *  create a config with unique logger for it
     */
    explicit BAASConfig(const std::string &path);

    void load();

    void save();

    inline bool contains(const std::string &key) {
        assert(!key.empty());
        std::lock_guard<std::mutex> lock(mtx);
        if(key[0] != '/') return config.contains(key);
        try{
            nlohmann::json &j = config.at(nlohmann::json::json_pointer(key));
            return true;
        }catch(std::exception &e) {return false;}
    }

    inline int getInt(const std::string &key, int default_value=0) {
        return get<int>(key, default_value);
    }

    inline float getFloat(const std::string &key, float default_value = 0.0f) {
        return get<float>(key, default_value);
    }

    inline std::string getString(const std::string &key, const std::string& default_value = "") {
        return get<std::string>(key, default_value);
    }

    inline bool getBool(const std::string &key, bool default_value = false) {
        return get<bool>(key, default_value);
    }
    /*
     * get value, if key not exist, return default value
     * "A" or "/A/B/C" (json_pointer)
     */

    template <typename T>
    inline T get(const std::string &key, T default_value) {
        assert(!key.empty());
        std::lock_guard<std::mutex> lock(mtx);
        if(key[0] != '/') {
            auto it = config.find(key);
            if(it == config.end()) {
                logger->BAASInfo("Key [ " + key + " ] not found, use default value.");
                return default_value;
            }
            return *it;
        }
        try {
            return config.at(nlohmann::json::json_pointer(key));
        }
        catch (std::exception &e) {
            logger->BAASInfo("Key [ " + key + " ] not found, use default value.");
            return default_value;
        }
    }

    template <typename T>
    inline T get(const std::string &key) {
        assert(!key.empty());
        std::lock_guard<std::mutex> lock(mtx);
        if(key[0] != '/') {
            auto it = config.find(key);
            if(it == config.end()) throwKeyError("Key [ " + key + " ] not found.");
            return *it;
        }
        try {
            return config.at(nlohmann::json::json_pointer(key));
        }
        catch (std::exception &e) {
            throwKeyError("Key [ " + key + " ] not found.");
        }
    }
    /*
     * replace value, the key must exist
     */
    template <typename T>
    void replace(const std::string &key, T &value) {
        assert(!key.empty());
        std::lock_guard<std::mutex> lock(mtx);
        if(key[0] != '/') {
            auto it = findByKey(key);
            if(*it != value) {
                modified.push_back({{"op", "replace"}, {"path", "/" + key}, {"value", value}});
                *it = value;
                return;
            }
        }
        try {
            nlohmann::json &j = config.at(nlohmann::json::json_pointer(key));
            if(j != value) {
                modified.push_back({{"op", "replace"}, {"path", key}, {"value", value}});
                j = value;
            }
        }
        catch (std::exception &e) {
            throwKeyError("Key [ " + key + " ] not found : " + e.what());
        }
    }

    template <typename T>
    inline void replace_and_save(const std::string &key, T &value) {
        replace(key, value);
        save();
    }

    /*
     * update value,if the key not exist, create it(add)
     * else replace it (replace)
     */
    template <typename T>
    void update(const std::string &key, T &value) {
        assert(!key.empty());
        std::lock_guard<std::mutex> lock(mtx);
        if(key[0]!='/') {
            auto it = config.find(key);
            if(it == config.end()) {            // not exist, create it
                logger->BAASInfo("create \" " + key + " \" in config file : [ " + path + " ]");
                config[key] = value;
                modified.push_back({{"op", "add"}, {"path", "/" + key}, {"value", value}});
            }
            else if(*it != value) {             // exist but not equal, replace it
                modified.push_back({{"op", "replace"}, {"path", "/" + key}, {"value", value}});
                *it = value;
            }
            return;
        }
        try {
            nlohmann::json &j = config.at(nlohmann::json::json_pointer(key));
            if(j != value) {
                modified.push_back({{"op", "replace"}, {"path", key}, {"value", value}});
                j = value;
            }
        }
        catch (std::exception &e) {
            config[nlohmann::json::json_pointer(key)] = value;
            modified.push_back({{"op", "add"}, {"path", key}, {"value", value}});
        }
    }
    template<typename T>
    inline void update_and_save(const std::string &key, T &value) {
        update(key, value);
        save();
    }

    /*
     * remove "A" or "/A/B/C" "
     */
    void remove(const std::string &key);

    inline void remove_and_save(const std::string &key) {
        remove(key);
        save();
    }

    void my_flatten();

    void flatten(std::string& jp, nlohmann::json &tar, nlohmann::json &result);

    void my_unflatten();

    void unflatten(nlohmann::json &value);

    void show();

    void show_modify_history();

    void diff(nlohmann::json &j, nlohmann::json& result);

    static inline void parent_pointer(std::string &ptr) {
        if(!ptr.empty()) ptr = ptr.substr(0, ptr.find_last_of('/'));
    }

    [[nodiscard]] inline const std::string& get_config_name() const {
        return config_name;
    }

    [[nodiscard]] inline BAASLogger* get_logger() const {
        return logger;
    }
    [[nodiscard]] inline const std::string& get_path() const {
        return path;
    }
    [[nodiscard]] inline const nlohmann::json& get_config() const {
        return config;
    }
protected:
    // findByKey key must exist
    inline nlohmann::json::iterator findByKey(const std::string &key) {
        nlohmann::json::iterator it = config.find(key);
        if(it == config.end()) throwKeyError("Key [ " + key + " ] not found.");
        return it;
    }

    template <typename T>
    inline void updateByKey(const std::string &key, T &value) {
        std::lock_guard<std::mutex> lock(mtx);
        if(config.contains(key)) {
            if(config[key] != value) {
                modified.push_back({{"op", "replace"}, {"path", key}, {"value", value}});
                config[key] = value;
            }
        }
        else {
            modified.push_back({{"op", "add"}, {"path", key}, {"value", value}});
            config[key] = value;
        }
    }

    inline void removeByKey(const std::string &key) {
        std::lock_guard<std::mutex> lock(mtx);
        if(config.contains(key)) {
            modified.push_back({{"op", "remove"}, {"path", key}});
            config.erase(key);
        }
    }

    inline void preProcessValue() {
        std::lock_guard<std::mutex> lock(mtx);
        std::string jp;
        preprocess(jp, config);
    }

    void preprocess(std::string& jp, nlohmann::json &value);

    inline void throwKeyError(const std::string &desc) {
        throw KeyError("In Config file [ " + path + " ] : \n" + desc);
    }

    inline void save_modify_history() {
        if (modified.empty()) return;
        std::ifstream in(modify_history_path);
        nlohmann::json j = nlohmann::json::parse(in);
        in.close();
        auto it = j.find(BAASUtil::current_time_string());
        if(it != j.end())it->push_back(modified);
        else j[BAASUtil::current_time_string()] = modified;
        std::ofstream out(modify_history_path, std::ios::out | std::ios::trunc);
        out << j.dump(4);
        out.close();
        modified.clear();
    }
    BAASLogger* logger;

    nlohmann::json config, modified;

    std::string path, modify_history_path, config_name;

    std::mutex mtx;

};



extern BAASConfig* config_name_change;


#endif //BAAS_CONFIG_BAASCONFIG_H_
