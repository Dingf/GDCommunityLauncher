#ifndef INC_GDCL_EXTRACTOR_CRAFTING_TEMPLATE_H
#define INC_GDCL_EXTRACTOR_CRAFTING_TEMPLATE_H

#include <string>
#include "ItemType.h"

struct CraftingTemplate
{
    std::string _recordName;
    float _prefixAdjustment;
    float _suffixAdjustment;

    static const std::unordered_map<uint64_t, CraftingTemplate>& GetTemplates();
};

#endif//INC_GDCL_EXTRACTOR_CRAFTING_TEMPLATE_H
