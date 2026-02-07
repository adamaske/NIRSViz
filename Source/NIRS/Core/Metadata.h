#pragma once
#include <string>
#include <vector>

namespace NIRS {
    namespace Metadata {

        struct MetadataTag {
            std::string name;
            std::string value;
        };

        struct Metadata {
            std::vector<MetadataTag> tags;

            bool has_wings_generation = false;
        };
    }
}
