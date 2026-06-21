#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "LabelTemplate.h"

struct LabelValidationIssue
{
    std::string message;
    std::size_t elementIndex = 0;
    bool hasElementIndex = false;
};

struct LabelValidationResult
{
    std::vector<LabelValidationIssue> issues;

    bool isValid() const
    {
        return issues.empty();
    }
};

class LabelValidation
{
public:
    static LabelValidationResult ValidateTemplate(const LabelTemplate& labelTemplate)
    {
        LabelValidationResult result;

        if (labelTemplate.name.empty())
        {
            AddTemplateIssue(result, "Template name is required.");
        }

        if (labelTemplate.labelWidthDots <= 0)
        {
            AddTemplateIssue(result, "Label width must be greater than zero.");
        }

        if (labelTemplate.labelHeightDots <= 0)
        {
            AddTemplateIssue(result, "Label height must be greater than zero.");
        }

        if (labelTemplate.elements.empty())
        {
            AddTemplateIssue(result, "Template must contain at least one element.");
        }

        for (std::size_t i = 0; i < labelTemplate.elements.size(); ++i)
        {
            ValidateElement(labelTemplate.elements[i], i, result);
        }

        return result;
    }

private:
    static void ValidateElement(const LabelElement& element, std::size_t index, LabelValidationResult& result)
    {
        if (element.name.empty())
        {
            AddElementIssue(result, index, "Element name is required.");
        }

        if (element.fieldKey.empty())
        {
            AddElementIssue(result, index, "Element field key is required.");
        }

        if (element.x < 0 || element.y < 0)
        {
            AddElementIssue(result, index, "Element position cannot be negative.");
        }

        if (element.type == LabelElementType::Barcode)
        {
            if (element.barcodeHeight <= 0)
            {
                AddElementIssue(result, index, "Barcode height must be greater than zero.");
            }

            if (element.barcodeModuleWidth < 1 || element.barcodeModuleWidth > 10)
            {
                AddElementIssue(result, index, "Barcode module width must be between 1 and 10.");
            }
        }
        else
        {
            if (element.fontHeight <= 0 || element.fontWidth <= 0)
            {
                AddElementIssue(result, index, "Text font dimensions must be greater than zero.");
            }
        }
    }

    static void AddTemplateIssue(LabelValidationResult& result, const std::string& message)
    {
        result.issues.push_back({ message, 0, false });
    }

    static void AddElementIssue(LabelValidationResult& result, std::size_t index, const std::string& message)
    {
        result.issues.push_back({ message, index, true });
    }
};
