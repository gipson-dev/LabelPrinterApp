#pragma once

#include <string>

#include "core/LabelTemplate.h"
#include "core/VariableResolver.h"

namespace SampleData
{
inline bool isBuiltInPlaceholder(const std::string& name)
{
    return name == "Date" ||
           name == "Time" ||
           name == "DateTime" ||
           name == "Serial" ||
           name == "RecordIndex";
}

inline std::string placeholderValue(const std::string& name)
{
    if (name == "Number" || name == "ItemNumber")
    {
        return "TEST-001";
    }
    if (name == "Description")
    {
        return "Test description";
    }
    if (name == "Order id")
    {
        return "1001";
    }
    if (name == "Name")
    {
        return "Database school 2";
    }
    if (name == "Lot")
    {
        return "LOT-001";
    }
    if (name == "Bin")
    {
        return "A-01";
    }
    if (name == "Quantity")
    {
        return "1";
    }
    return "Sample";
}

inline void fillMissingForElement(const LabelElement& element, VariableContext& context)
{
    if (context.serialNumber == 0)
    {
        context.serialNumber = 1;
    }

    if ((element.source == FieldSource::PromptAtPrint || element.source == FieldSource::Variable) &&
        !element.variableName.empty() &&
        context.values.find(element.variableName) == context.values.end())
    {
        context.values[element.variableName] = placeholderValue(element.variableName);
    }

    for (const auto& placeholder : VariableResolver::findPlaceholders(element.text))
    {
        if (isBuiltInPlaceholder(placeholder.first))
        {
            continue;
        }
        if (context.values.find(placeholder.first) == context.values.end())
        {
            context.values[placeholder.first] = placeholderValue(placeholder.first);
        }
    }
}

inline void fillMissingForTemplate(const LabelTemplate& labelTemplate, VariableContext& context)
{
    for (const LabelElement& element : labelTemplate.elements)
    {
        fillMissingForElement(element, context);
    }
}
}
