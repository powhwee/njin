#include "util/json.h"

#include <stdexcept>

#include "util/file.h"

namespace rj = rapidjson;

namespace njin::util {

    rapidjson::Document make_document(const std::string& path) {
        std::string document_contents{ util::read_file(path) };

        rj::Document document{};
        if (document.Parse(document_contents.c_str()).HasParseError()) {
            throw std::runtime_error("failed to parse json");
        }

        return document;
    }

    rapidjson::Document make_validated_document(const std::string& schema_path,
                                                const std::string& json_path) {
        // validator
        std::string schema_contents{ util::read_file(schema_path) };
        rj::Document schema_document{};
        if (schema_document.Parse(schema_contents.c_str()).HasParseError()) {
            throw std::runtime_error("failed to parse scene schema");
        }

        rj::SchemaDocument schema{ schema_document };
        rj::SchemaValidator validator{ schema };

        // document
        rj::Document document{ make_document(json_path) };

        // validate the document
        if (!document.Accept(validator)) {
            // Input JSON is invalid according to the schema
            // Output diagnostic information
            rj::StringBuffer sb;
            validator.GetInvalidSchemaPointer().StringifyUriFragment(sb);
            printf("Invalid schema: %s\n", sb.GetString());
            printf("Invalid keyword: %s\n",
                   validator.GetInvalidSchemaKeyword());
            sb.Clear();
            validator.GetInvalidDocumentPointer().StringifyUriFragment(sb);
            printf("Invalid document: %s\n", sb.GetString());
            throw std::runtime_error("failed to validate json");
        }

        return document;
    }
}  // namespace njin::util