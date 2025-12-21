#pragma once
#include <filesystem>
#include <string>

namespace NIRS {
    namespace Metadata {

        struct MetadataTag {
            std::string name;
            std::string value;

        };
        struct Metadata {
            // TODO : Flexible parsing of the "MetaDataTags"
        };

        inline Metadata LoadMetadata(std::filesystem::path filepath) {
            Metadata md;

            return md;
        }
    }
}