//
// Created by pc on 2024/4/12.
// 86310854

#ifndef BAAS_CONFIG_BAASCONFIG_H_
#define BAAS_CONFIG_BAASCONFIG_H_

#define CONFIG_TYPE_STATIC_CONFIG 0
#define CONFIG_TYPE_DEFAULT_CONFIG 1
#define CONFIG_TYPE_CONFIG_NAME_CHANGE 2
#define CONFIG_TYPE_DEFAULT_GLOBAL_SETTING 3

#include <opencv2/opencv.hpp>

// json included in this file
#include "BAASTypes.h"
#include "BAASLogger.h"
#include "BAASExceptions.h"

BAAS_NAMESPACE_BEGIN

class BAASConfig {
public:
    explicit BAASConfig() = default;

    // init with raw json data
    explicit BAASConfig(
            const nlohmann::json& j,
            BAASLogger* logger
    );

    // init with json path
    explicit BAASConfig(
            const std::filesystem::path& path,
            BAASLogger* logger
    );

    // create a simple json read and write config with json path and logger
    explicit BAASConfig(
            const std::string& path,
            BAASLogger* logger
    );

    /*
     * special config files which are read only
     *  use global logger
     */
    explicit BAASConfig(const int config_type);

    /*
     * config_name / config.json or event.json
     *  create a config with unique logger for it
     */
    explicit BAASConfig(const std::string& path);

    explicit BAASConfig(const std::filesystem::path& path);

    void save();

    nlohmann::json::const_iterator begin() const
    {
        return config.begin();
    }

    nlohmann::json::const_iterator end() const
    {
        return config.end();
    }

    nlohmann::json::const_iterator find(const std::string& key) const
    {
        return config.find(key);
    }

    inline unsigned int get_array_size(const std::string& key) const
    {
        assert(!key.empty());
        if (key[0] != '/') {
            auto it = config.find(key);
            if (it == config.end()) throw_key_error("Key [ " + key + " ] not found.");
            if (!it->is_array()) {
                throw_key_error("Value of [ " + key + " ] is not an array.");
            }
            return it->size();
        }
        nlohmann::json j;
        try {
            j = config.at(nlohmann::json::json_pointer(key));
        }
        catch (std::exception& e) {
            throw_key_error("Key [ " + key + " ] not found.");
        }
        if (!j.is_array()) {
            throw_key_error("Value of [ " + key + " ] is not an array.");
        }
        return j.size();
    }

    inline nlohmann::json::value_t value_type(const std::string& key) const
    {
        assert(!key.empty());
        if (key[0] != '/') {
            auto it = config.find(key);
            if (it == config.end()) throw_key_error("Key [ " + key + " ] not found.");
            return it->type();
        }
        try {
            return config.at(nlohmann::json::json_pointer(key)).type();
        }
        catch (std::exception& e) {
            throw_key_error("Key [ " + key + " ] not found.");
        }
    }

    inline bool contains(const std::string& key) const
    {
        assert(!key.empty());
        if (key[0] != '/') return config.contains(key);
        try {
            config.at(nlohmann::json::json_pointer(key));
            return true;
        } catch (std::exception& e) { return false; }
    }

    inline void getBAASConfig(
            const std::string& key,
            BAASConfig& output,
            BAASLogger* logger = (BAASLogger*)(BAASGlobalLogger)
    )   const
    {
        assert(!key.empty());
        if (key[0] != '/') {
            auto it = config.find(key);
            if (it == config.end()) {
                throw_key_error("Key [ " + key + " ] not found.");
            }
            output = BAASConfig(*it, logger);
            return;
        }
        try {
            output = BAASConfig(config.at(nlohmann::json::json_pointer(key)), logger);
        }
        catch (std::exception& e) {
            throw_key_error("Key [ " + key + " ] not found.");
        }
    }

    inline int getInt(
            const std::string& key,
            int default_value = 0
    )   const
    {
        return get<int>(key, default_value);
    }

    inline unsigned int getUInt(
            const std::string& key,
            unsigned int default_value = 0
    )   const
    {
        return get<unsigned int>(key, default_value);
    }

    inline uint8_t getUInt8(
            const std::string& key,
            uint8_t default_value = 0
    )   const
    {
        return get<uint8_t>(key, default_value);
    }

    inline cv::Vec3b getVec3b(
            const std::string& key,
            const cv::Vec3b& default_value = {0, 0, 0}
    )   const
    {
        if (contains(key)) {
            nlohmann::json it;
            try{
                it = config.at(nlohmann::json::json_pointer(key));
            }
            catch (std::exception &e) {
                return default_value;
            }
            if (!it.is_array() or it.size() != 3) { return default_value; }
            try {
                return {
                        it[0].get<uint8_t>(),
                        it[1].get<uint8_t>(),
                        it[2].get<uint8_t>()
                };
            } catch (std::exception& e) {
                return default_value;
            }
        }
        return default_value;
    }

    inline BAASPoint get_point(
            const std::string& key,
            const BAASPoint& default_value = {0, 0}
    )   const
    {
        return get<BAASPoint>(key, default_value);
    }

    inline BAASRectangle get_rect(
            const std::string& key,
            const BAASRectangle& default_value = {0, 0, 0, 0}
    )   const
    {
        return get<BAASRectangle>(key, default_value);
    }

    inline long getLong(
            const std::string& key,
            long default_value = 0
    )   const
    {
        return get<long>(key, default_value);
    }

    inline unsigned long getULong(
            const std::string& key,
            unsigned long default_value = 0
    )   const
    {
        return get<unsigned long>(key, default_value);
    }

    inline long long getLLong(
            const std::string& key,
            long long default_value = 0
    )   const
    {
        return get<long long>(key, default_value);
    }

    inline unsigned long long getULLong(
            const std::string& key,
            unsigned long long default_value = 0
    )   const
    {
        return get<unsigned long long>(key, default_value);
    }

    inline nlohmann::json getJson(
            const std::string& key,
            const nlohmann::json& default_value = nlohmann::json()
    )   const
    {
        return get<nlohmann::json>(key, default_value);
    }

    inline float getFloat(
            const std::string& key,
            float default_value = 0.0f
    )   const
    {
        return get<float>(key, default_value);
    }

    inline double getDouble(
            const std::string& key,
            double default_value = 0.0
    ) const
    {
        return get<double>(key, default_value);
    }

    inline std::string getString(
            const std::string& key,
            const std::string& default_value = ""
    ) const
    {
        return get<std::string>(key, default_value);
    }

    inline std::filesystem::path getPath(
            const std::string& key,
            const std::filesystem::path& default_value = ""
    )   const
    {
        std::string str_path = getString(key, default_value.string());
#ifdef _WIN32
        const size_t size = str_path.size();
        if (size == 0) return std::filesystem::path();
        std::wstring wstr_path;
        const size_t expected_utf16_words = simdutf::utf16_length_from_utf8(str_path.c_str(), size);
        wstr_path.resize(expected_utf16_words);
        const size_t real = simdutf::convert_utf8_to_utf16le(
                str_path.c_str(),
                size,
                reinterpret_cast<char16_t*>(wstr_path.data())
        );
        std::filesystem::path p = wstr_path;
#else
        std::filesystem::path p = str_path;
#endif
        return p;
    }

    inline bool getBool(
            const std::string& key,
            bool default_value = false
    ) const
    {
        return get<bool>(key, default_value);
    }

    /*
    * get value, if key not exist, return default value
    * "A" or "/A/B/C" (json_pointer)
    */
    template<typename T>
    inline T get(
            const std::string& key,
            const T& default_value
    )   const noexcept
    {
        assert(!key.empty());
        if (key[0] != '/') {
            auto it = config.find(key);
            if (it == config.end()) {
                return default_value;
            }
            try {
                return *it;
            } catch (std::exception& e) {
                return default_value;
            }
        }
        try {
            return config.at(nlohmann::json::json_pointer(key));
        }
        catch (std::exception& e) {
            return default_value;
        }
    }

    // will throw exception if key not exist or data type mismatch
    template<typename T>
    inline T get(const std::string& key) const
    {
        T default_value;
        assert(!key.empty());
        if (key[0] != '/') {
            auto it = config.find(key);
            if (it == config.end()) throw_key_error("Key [ " + key + " ] not found.");
            try {
                return *it;
            } catch (std::exception& e) {
                throw_type_error(
                        "Value With Key [ " + key + " ] Type Error. Real : " + std::string(it->type_name()) +
                        " .Expected : " + typeid(default_value).name());
            }
        }
        try {
            return config.at(nlohmann::json::json_pointer(key));
        }
        // key incorrect / out_of_range / value type mismatch
        catch (nlohmann::json::out_of_range& e) {
            throw_key_error("Key [ " + key + " ] not found." + e.what());
        }
        catch (nlohmann::json::type_error& e) {
            throw_type_error("Value With Key [ " + key + " ] Type Error. Real : " + std::string(config.type_name()) +
                    " .Expected : " + typeid(default_value).name());
        }
    }

    /*
    * replace value, the key must exist
    */
    template<typename T>
    void replace(
            const std::string& key,
            T& value
    )
    {
        assert(!key.empty());
        if (key[0] != '/') {
            auto it = findByKey(key);
            if (*it != value) {
                modified.push_back(
                        {{"op",    "replace"},
                         {"path",  "/" + key},
                         {"value", value}}
                );
                *it = value;
                return;
            }
        }
        try {
            nlohmann::json &j = config.at(nlohmann::json::json_pointer(key));
            if (j != value) {
                modified.push_back(
                        {{"op",    "replace"},
                         {"path",  key},
                         {"value", value}}
                );
                j = value;
            }
        }
        catch (std::exception& e) {
            throw_key_error("Key [ " + key + " ] not found : " + e.what());
        }
    }

    template<typename T>
    inline void replace_and_save(
            const std::string& key,
            T& value
    )
    {
        replace(key, value);
        save();
    }
/*
    * update value,if the key not exist, create it(add)
    * else replace it (replace)
    */
    template<typename T>
    void update(
            const std::string& key,
            T value
    )
    {
        assert(!key.empty());
        if (key[0] != '/') {
            auto it = config.find(key);
            if (it == config.end()) {            // not exist, create it
                std::string log = "create key [ " + key + " ]";
                if (!path.empty()) log += " \" in config file : [ " + path.string() + " ]";
                logger->BAASInfo(log);
                config[key] = value;
                modified.push_back(
                        {{"op",    "add"},
                         {"path",  "/" + key},
                         {"value", value}}
                );
            } else if (*it != value) {             // exist but not equal, replace it
                modified.push_back(
                        {{"op",    "replace"},
                         {"path",  "/" + key},
                         {"value", value}}
                );
                *it = value;
            }
            return;
        }
        try {
            nlohmann::json& j = config.at(nlohmann::json::json_pointer(key));
            if (j != value) {
                modified.push_back(
                        {{"op",    "replace"},
                         {"path",  key},
                         {"value", value}}
                );
                j = value;
            }
        }
        catch (std::exception& e) {
            config[nlohmann::json::json_pointer(key)] = value;
            modified.push_back(
                    {{"op",    "add"},
                     {"path",  key},
                     {"value", value}}
            );
        }
    }
    /*
    * update value,if the key not exist, create it(add)
    * else replace it (replace)
    */
    template<typename T>
    void update_reference(
            const std::string& key,
            const T& value
    )
    {
        assert(!key.empty());
        if (key[0] != '/') {
            auto it = config.find(key);
            if (it == config.end()) {            // not exist, create it
                std::string log = "create key [ " + key + " ]";
                if (!path.empty()) log += " \" in config file : [ " + path.string() + " ]";
                logger->BAASInfo(log);
                config[key] = value;
                modified.push_back(
                        {{"op",    "add"},
                         {"path",  "/" + key},
                         {"value", value}}
                );
            } else if (*it != value) {             // exist but not equal, replace it
                modified.push_back(
                        {{"op",    "replace"},
                         {"path",  "/" + key},
                         {"value", value}}
                );
                *it = value;
            }
            return;
        }
        try {
            nlohmann::json& j = config.at(nlohmann::json::json_pointer(key));
            if (j != value) {
                modified.push_back(
                        {{"op",    "replace"},
                         {"path",  key},
                         {"value", value}}
                );
                j = value;
            }
        }
        catch (std::exception& e) {
            config[nlohmann::json::json_pointer(key)] = value;
            modified.push_back(
                    {{"op",    "add"},
                     {"path",  key},
                     {"value", value}}
            );
        }
    }

    void update(const BAASConfig* patch)
    {
        for (auto &i: patch->get_config().items())
            update_reference(i.key(), i.value());
    }

    template<typename T>
    inline void update_and_save(
            const std::string& key,
            const T& value
    )
    {
        update_reference(key, value);
        save();
    }

    template<typename T>
    inline void insert(
            const std::string& key,
            const T& value
    )
    {
        assert(!key.empty());
        config[key] = value;
    }

    /*
    * remove "A" or "/A/B/C" "
    */
    void remove(const std::string& key);

    inline void remove_and_save(const std::string& key)
    {
        remove(key);
        save();
    }

    void my_flatten();

    void flatten(
            std::string& jp,
            nlohmann::json& tar,
            nlohmann::json& result
    );

    void my_unflatten();

    static void unflatten(nlohmann::json& value);

    void show(
            int indent = 4,
            bool ensure_ascii = false
    ) const;

    void show_modify_history();

    void diff(
            nlohmann::json& j,
            nlohmann::json& result
    );

    // clear and replace_all do not change modify history
    void clear() noexcept;

    void replace_all(nlohmann::json& new_config);

    static inline void parent_pointer(std::string& ptr)
    {
        if (!ptr.empty()) ptr = ptr.substr(0, ptr.find_last_of('/'));
    }

    [[nodiscard]] inline const std::string& get_config_name() const
    {
        return config_name;
    }

    [[nodiscard]] inline BAASLogger* get_logger() const
    {
        return logger;
    }

    [[nodiscard]] inline const std::filesystem::path& get_path() const
    {
        return path;
    }

    [[nodiscard]] inline const nlohmann::json& get_config() const
    {
        return config;
    }

protected:

    void _check_p(const std::filesystem::path& _p);

    void _get_logger();

    void _init_config();

    void _init_modify_history();

    // findByKey key must exist
    inline nlohmann::json::iterator findByKey(const std::string& key)
    {
        nlohmann::json::iterator it = config.find(key);
        if (it == config.end()) throw_key_error("Key [ " + key + " ] not found.");
        return it;
    }

    template<typename T>
    inline void updateByKey(
            const std::string& key,
            T& value
    )
    {
        if (config.contains(key)) {
            if (config[key] != value) {
                modified.push_back(
                        {{"op",    "replace"},
                         {"path",  key},
                         {"value", value}}
                );
                config[key] = value;
            }
        } else {
            modified.push_back(
                    {{"op",    "add"},
                     {"path",  key},
                     {"value", value}}
            );
            config[key] = value;
        }
    }

    inline void removeByKey(const std::string& key)
    {
        if (config.contains(key)) {
            modified.push_back(
                    {{"op",   "remove"},
                     {"path", key}}
            );
            config.erase(key);
        }
    }

    inline void _preprocess_value()
    {
        std::string jp;
        _preprocess(jp, config);
    }

    void _preprocess(
            std::string& jp,
            nlohmann::json& value
    );

    inline void throw_key_error(const std::string& desc)  const
    {
        std::string msg;
        if (!path.empty()) msg = "In Config file : [ " + path.string() + " ] : \n";
        msg += desc;
        throw KeyError(msg);
    }

    inline void throw_type_error(const std::string& desc) const
    {
        std::string msg;
        if (!path.empty()) msg = "In Config file : [ " + path.string() + " ] : \n";
        msg += desc;
        throw TypeError(msg);
    }

    void save_modify_history();

    BAASLogger* logger;
    nlohmann::json config, modified;
    std::filesystem::path path, modify_history_path;
    std::string config_name, config_type;
};

extern BAASConfig* config_name_change;
extern BAASConfig* default_global_setting;

BAAS_NAMESPACE_END


#endif //BAAS_CONFIG_BAASCONFIG_H_
