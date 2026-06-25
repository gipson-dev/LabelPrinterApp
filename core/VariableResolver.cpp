#include "core/VariableResolver.h"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <regex>
#include <sstream>

std::string VariableResolver::resolveText(const std::string& input, const VariableContext& context)
{
    std::regex tokenPattern("\\{([^{}]+)\\}");
    std::string output;
    std::sregex_iterator begin(input.begin(), input.end(), tokenPattern);
    std::sregex_iterator end;

    std::size_t last = 0;
    for (auto it = begin; it != end; ++it)
    {
        output.append(input, last, static_cast<std::size_t>(it->position()) - last);
        std::string name = (*it)[1].str();
        auto value = context.values.find(name);
        output += value != context.values.end() ? value->second : builtInValue(name, context);
        last = static_cast<std::size_t>(it->position() + it->length());
    }

    output.append(input, last, std::string::npos);
    return output;
}

std::string VariableResolver::elementValue(const LabelElement& element, const VariableContext& context)
{
    std::string value;
    if (element.source == FieldSource::SerialNumber)
    {
        value = formatSerial(context.serialNumber, element.serialWidth);
    }
    else if (!element.variableName.empty())
    {
        auto it = context.values.find(element.variableName);
        value = it != context.values.end() ? it->second : resolveText(element.text, context);
    }
    else
    {
        value = resolveText(element.text, context);
    }

    return element.prefix + value + element.suffix;
}

std::map<std::string, std::string> VariableResolver::findPlaceholders(const std::string& text)
{
    std::map<std::string, std::string> placeholders;
    std::regex tokenPattern("\\{([^{}]+)\\}");
    for (std::sregex_iterator it(text.begin(), text.end(), tokenPattern), end; it != end; ++it)
    {
        placeholders[(*it)[1].str()] = "";
    }
    return placeholders;
}

std::string VariableResolver::builtInValue(const std::string& name, const VariableContext& context)
{
    auto now = std::chrono::system_clock::now();
    std::time_t time = std::chrono::system_clock::to_time_t(now);
    std::tm localTime{};
#ifdef _WIN32
    localtime_s(&localTime, &time);
#else
    localtime_r(&time, &localTime);
#endif

    std::ostringstream out;
    if (name == "Date")
    {
        out << std::put_time(&localTime, "%Y-%m-%d");
        return out.str();
    }
    if (name == "Time")
    {
        out << std::put_time(&localTime, "%H:%M:%S");
        return out.str();
    }
    if (name == "DateTime")
    {
        out << std::put_time(&localTime, "%Y-%m-%d %H:%M:%S");
        return out.str();
    }
    if (name == "Serial")
    {
        return std::to_string(context.serialNumber);
    }
    if (name == "RecordIndex")
    {
        return std::to_string(context.recordIndex);
    }

    return "{" + name + "}";
}

std::string VariableResolver::formatSerial(int value, int width)
{
    std::ostringstream out;
    if (width > 0)
    {
        out << std::setw(width) << std::setfill('0');
    }
    out << value;
    return out.str();
}
