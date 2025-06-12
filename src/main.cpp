#include <cassert>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <iostream>

// nlohmann
#include <json.hpp>
// yhirose
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <httplib.h>

#define ASSERT(expr, msg) \
do { \
    if (!(expr)) { \
        fprintf(stderr, "Assertion failed: %s (%s), file %s, line %d\n", \
                #expr, msg, __FILE__, __LINE__); \
        abort(); \
    } \
} while (0)

struct ConfigJson {
    std::string api_base_url;
    std::string api_key;
    std::vector<std::string> models;
};

// somehow converts the above struct to json? magic?
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ConfigJson, api_base_url, api_key, models)


ConfigJson parse_config_file(std::filesystem::path& cfg_path) {
    ConfigJson config;
    std::ifstream f(cfg_path);
    nlohmann::json jason = nlohmann::json::parse(f);

    // parsed base url
    if(jason.contains("api_base_url") && jason["api_base_url"].is_string()) {
        config.api_base_url = jason["api_base_url"];
    } else {
        throw std::runtime_error("api_base_url not set in config");
    }

    // start parsing api_key and api_key_env
    if(jason.contains("api_key") && jason["api_key"].is_string()) {
        // config says to read from ENV
        if (std::strcmp(jason["api_key"].get<std::string>().c_str(), "READ_FROM_ENV") == 0) {
            if(jason.contains("api_key_env") && jason["api_key_env"].is_string()) {
                const char* read_from_env_api_key = std::getenv(jason["api_key_env"].get<std::string>().c_str());
                if (read_from_env_api_key) {
                    config.api_key = read_from_env_api_key;
                } else {
                    std::runtime_error("Couldn't read api key from environment");
                }
            } else {
                throw std::runtime_error("The config file wants to read from ENV but it didn't provide the variable name \"api_key_env\"");
            }
        } else { // READ_FROM_ENV wasn't specified, just read the api_key normally
            config.api_key = jason["api_key"];
        }
    } else {
        throw std::runtime_error("api_key not set in config");
    }


    if(jason.contains("models") && jason["models"].is_array()) {
        if (jason["models"].size() == 0) {
            std::runtime_error("\"models\" in config is of length 0, provide at least one ai model");
        }
        config.models = jason["models"];
    } else {
        std::runtime_error("\"models\" doesn't exist in config or isn't an array");
    }

    return config;
}

int main (int argc, char *argv[]) {
    bool verbose = false;
    bool insecure_http_enabled = false;
    std::string user_prompt = "null";
    if (argc > 1) {
        // if user wants -v or --verbose
        for (size_t i = 0; i < argc; ++i) {
            if ((strcmp(argv[i], "-v") == 0) || (strcmp(argv[i], "--verbose") == 0)) {
                verbose = true;
            }
            if ((strcmp(argv[i], "--insecure_http") == 0)) {
                insecure_http_enabled = true;
            }
            if ((strcmp(argv[i], "-h") == 0) || (strcmp(argv[i], "--help") == 0)) {
                printf("Usage: %s -m \"Your prompt here\"\n", argv[0]);
            }
            if ((strcmp(argv[i], "-m") == 0) || (strcmp(argv[i], "--message") == 0)) {
                if (argc <= i) {
                    printf("Attempted to send message but there was no argument provided:\n");
                    printf("Usage: %s -m \"Your prompt here\"\n", argv[0]);
                } else {
                    user_prompt = argv[i+1];
                }
            }


        }
    }

    if (strcmp("null", user_prompt.c_str()) == 0) {
        printf("A user prompt wasn't found, did you forget to add one? Check %s -h\n", argv[0]);
        return 1;
    }
    const char* config_path = (const char *)secure_getenv("XDG_CONFIG_HOME");
    if (verbose) {
        printf("%s\n", config_path);
    }

    ASSERT(std::filesystem::exists(std::filesystem::path(config_path) / "ai_cli" / "config.json"), "config file doesn't exist");

    std::filesystem::path config_file_path = (std::filesystem::path(config_path) / "ai_cli" / "config.json");
    if (verbose) {
        printf("config file: %s\n", config_file_path.c_str());
    }

    ConfigJson config = parse_config_file(config_file_path);

    if (verbose) {
        std::cout << nlohmann::json(config).dump(4) << std::endl;
    }

    // we read the config folder hopefully
    if (insecure_http_enabled) {

    }

    httplib::Client client(config.api_base_url);
    client.set_default_headers({
        { "Authorization", "Bearer " + config.api_key },
        { "Content-Type", "application/json" }
    });

    if (verbose) {
        printf("Set default http headers with api key\n");
    }

    nlohmann::json request = {
        { "model", config.models[0]},
        { "messages", {
            {{ "role", "system"}, {"content", "You are a helpful assistant"}},
            {{ "role", "user"}, {"content", user_prompt}},
        }},
        { "stream", false }
    };

    if (verbose) {
        printf("Sending AI api request (%s)\n", (config.api_base_url + "/v1/chat/completions").c_str());
    }

    if (httplib::Result result = client.Post("/v1/chat/completions", request.dump(), "application/json")) {
        if (verbose) {
            printf("AI request sent, awaiting HTTP status code and payload\n");
        }
        if (result->status >= 200 && result->status < 300) {
            if (verbose) {
                printf("AI request succeeded, printing model output!\n");
            }
            nlohmann::basic_json response = nlohmann::json::parse(result->body);
            printf("%s\n", response["choices"][0]["message"]["content"].get<std::string>().c_str());

            if (verbose) {
                std::cout << response.dump() << std::endl;
            }
        } else {
            if (verbose) {
                printf("Request returned a status code that is < 200 or >= 300\n");
            }
            printf("HTTP error: %s (status code: %d)\n", httplib::to_string(result.error()).c_str(), result->status);
        }
    }


    return 0;
}
