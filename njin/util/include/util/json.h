#pragma once
#include <string>

#include <rapidjson/document.h>
#include <rapidjson/schema.h>

/**
* Utility functions for processing JSONs and JSON schemas with rapidjson
*/
namespace njin::util {

    /**
   * Make a rapidjson document
   * @param path Path to json file
   * @return rapidjson document object
   * @throw std::runtime_error if json ggiven by path is not a valid json
   */
    rapidjson::Document make_document(const std::string& path);

    /**
   *  Make a rapidjson document validated with the given schema
   * @param schema_path Path to schema json
   * @param json_path Path to json to validate and make a document out of
   * @return Valid json document
   * @throw std::runtime_error if either json is invalid
   */
    rapidjson::Document make_validated_document(const std::string& schema_path,
                                                const std::string& json_path);

}  // namespace njin::util