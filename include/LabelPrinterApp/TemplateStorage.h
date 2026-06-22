#pragma once

#include <cctype>
#include <fstream>
#include <map>
#include <stdexcept>
#include <sstream>
#include <string>
#include <vector>

#include "LabelTemplate.h"
#include "LabelValidation.h"

class TemplateStorage
{
public:
    static LabelTemplate LoadFromFile(const std::string& path)
    {
        std::ifstream file(path);
        if (!file)
        {
            return LoadDefaultTemplate();
        }

        std::ostringstream buffer;
        buffer << file.rdbuf();
        LabelTemplate label = ParseTemplate(buffer.str());

        if (!LabelValidation::ValidateTemplate(label).isValid())
        {
            return LoadDefaultTemplate();
        }

        return label;
    }

    static bool SaveToFile(const LabelTemplate& label, const std::string& path)
    {
        if (!LabelValidation::ValidateTemplate(label).isValid())
        {
            return false;
        }

        std::ofstream file(path, std::ios::trunc);
        if (!file)
        {
            return false;
        }

        file << "{\n";
        file << "  \"name\": \"" << EscapeJson(label.name) << "\",\n";
        file << "  \"labelWidthDots\": " << label.labelWidthDots << ",\n";
        file << "  \"labelHeightDots\": " << label.labelHeightDots << ",\n";
        file << "  \"marginLeftDots\": " << label.marginLeftDots << ",\n";
        file << "  \"marginTopDots\": " << label.marginTopDots << ",\n";
        file << "  \"gapDots\": " << label.gapDots << ",\n";
        file << "  \"mediaTrackingMode\": \"" << ToString(label.mediaTrackingMode) << "\",\n";
        file << "  \"orientation\": \"" << ToString(label.orientation) << "\",\n";
        file << "  \"elements\": [\n";

        for (std::size_t i = 0; i < label.elements.size(); ++i)
        {
            const LabelElement& element = label.elements[i];
            file << "    {\n";
            file << "      \"name\": \"" << EscapeJson(element.name) << "\",\n";
            file << "      \"type\": \"" << ToString(element.type) << "\",\n";
            file << "      \"fieldKey\": \"" << EscapeJson(element.fieldKey) << "\",\n";
            file << "      \"text\": \"" << EscapeJson(element.text) << "\",\n";
            file << "      \"x\": " << element.x << ",\n";
            file << "      \"y\": " << element.y;

            if (element.type == LabelElementType::Barcode)
            {
                file << ",\n";
                file << "      \"barcodeHeight\": " << element.barcodeHeight << ",\n";
                file << "      \"barcodeModuleWidth\": " << element.barcodeModuleWidth << ",\n";
                file << "      \"barcodeSymbology\": \"" << ToString(element.barcodeSymbology) << "\",\n";
                file << "      \"barcodeHumanReadable\": " << (element.barcodeHumanReadable ? "true" : "false") << ",\n";
                file << "      \"rotation\": \"" << ToString(element.rotation) << "\",\n";
                file << "      \"editable\": " << (element.editable ? "true" : "false") << "\n";
            }
            else
            {
                file << ",\n";
                file << "      \"fontHeight\": " << element.fontHeight << ",\n";
                file << "      \"fontWidth\": " << element.fontWidth << ",\n";
                file << "      \"fontName\": \"" << EscapeJson(element.fontName) << "\",\n";
                file << "      \"boxWidth\": " << element.boxWidth << ",\n";
                file << "      \"maxLines\": " << element.maxLines << ",\n";
                file << "      \"rotation\": \"" << ToString(element.rotation) << "\",\n";
                file << "      \"alignment\": \"" << ToString(element.alignment) << "\",\n";
                file << "      \"bold\": " << (element.bold ? "true" : "false") << ",\n";
                file << "      \"italic\": " << (element.italic ? "true" : "false") << ",\n";
                file << "      \"underline\": " << (element.underline ? "true" : "false") << ",\n";
                file << "      \"multiline\": " << (element.multiline ? "true" : "false") << ",\n";
                file << "      \"variable\": " << (element.variable ? "true" : "false") << ",\n";
                file << "      \"autoFit\": " << (element.autoFit ? "true" : "false") << ",\n";
                file << "      \"wrap\": " << (element.wrap ? "true" : "false") << ",\n";
                file << "      \"editable\": " << (element.editable ? "true" : "false") << "\n";
            }

            file << "    }" << (i + 1 == label.elements.size() ? "\n" : ",\n");
        }

        file << "  ]\n";
        file << "}\n";
        return true;
    }

    static LabelTemplate LoadDefaultTemplate()
    {
        LabelTemplate label;
        label.name = "Default Label";
        label.labelWidthDots = 400;
        label.labelHeightDots = 240;
        label.marginLeftDots = 0;
        label.marginTopDots = 0;
        label.gapDots = 24;

        label.addElement(LabelElement::Text("Item Number", "ITEM: 123456", 30, 30, 35, 35, true));
        label.addElement(LabelElement::Text("Part Name", "PART A-100", 30, 80, 30, 30));
        label.addElement(LabelElement::Text("Quantity", "QTY: 25", 30, 125, 30, 30));
        label.addElement(LabelElement::Barcode("Barcode", "123456", 30, 170, 50));

        return label;
    }

private:
    enum class JsonType
    {
        Null,
        Object,
        Array,
        String,
        Number,
        Boolean
    };

    struct JsonValue
    {
        JsonType type = JsonType::Null;
        std::map<std::string, JsonValue> objectValues;
        std::vector<JsonValue> arrayValues;
        std::string stringValue;
        int numberValue = 0;
        bool boolValue = false;

        const JsonValue* find(const std::string& key) const
        {
            auto it = objectValues.find(key);
            return it == objectValues.end() ? nullptr : &it->second;
        }
    };

    class JsonParser
    {
    public:
        explicit JsonParser(const std::string& json)
            : input(json)
        {
        }

        JsonValue parse()
        {
            JsonValue value = parseValue();
            skipWhitespace();
            if (position != input.size())
            {
                throw std::runtime_error("Unexpected trailing JSON content.");
            }
            return value;
        }

    private:
        JsonValue parseValue()
        {
            skipWhitespace();
            if (position >= input.size())
            {
                throw std::runtime_error("Unexpected end of JSON.");
            }

            char next = input[position];
            if (next == '{')
            {
                return parseObject();
            }
            if (next == '[')
            {
                return parseArray();
            }
            if (next == '"')
            {
                JsonValue value;
                value.type = JsonType::String;
                value.stringValue = parseString();
                return value;
            }
            if (next == '-' || std::isdigit(static_cast<unsigned char>(next)))
            {
                return parseNumber();
            }
            if (startsWith("true"))
            {
                position += 4;
                JsonValue value;
                value.type = JsonType::Boolean;
                value.boolValue = true;
                return value;
            }
            if (startsWith("false"))
            {
                position += 5;
                JsonValue value;
                value.type = JsonType::Boolean;
                value.boolValue = false;
                return value;
            }
            if (startsWith("null"))
            {
                position += 4;
                return {};
            }

            throw std::runtime_error("Unexpected JSON value.");
        }

        JsonValue parseObject()
        {
            JsonValue value;
            value.type = JsonType::Object;
            consume('{');
            skipWhitespace();

            if (tryConsume('}'))
            {
                return value;
            }

            while (true)
            {
                skipWhitespace();
                std::string key = parseString();
                skipWhitespace();
                consume(':');
                value.objectValues[key] = parseValue();
                skipWhitespace();

                if (tryConsume('}'))
                {
                    break;
                }

                consume(',');
            }

            return value;
        }

        JsonValue parseArray()
        {
            JsonValue value;
            value.type = JsonType::Array;
            consume('[');
            skipWhitespace();

            if (tryConsume(']'))
            {
                return value;
            }

            while (true)
            {
                value.arrayValues.push_back(parseValue());
                skipWhitespace();

                if (tryConsume(']'))
                {
                    break;
                }

                consume(',');
            }

            return value;
        }

        std::string parseString()
        {
            consume('"');
            std::string output;

            while (position < input.size())
            {
                char c = input[position++];
                if (c == '"')
                {
                    return output;
                }
                if (c != '\\')
                {
                    output += c;
                    continue;
                }

                if (position >= input.size())
                {
                    throw std::runtime_error("Unfinished JSON escape sequence.");
                }

                char escaped = input[position++];
                switch (escaped)
                {
                case '"':
                case '\\':
                case '/':
                    output += escaped;
                    break;
                case 'b':
                    output += '\b';
                    break;
                case 'f':
                    output += '\f';
                    break;
                case 'n':
                    output += '\n';
                    break;
                case 'r':
                    output += '\r';
                    break;
                case 't':
                    output += '\t';
                    break;
                case 'u':
                    appendUtf8(output, parseUnicodeEscape());
                    break;
                default:
                    throw std::runtime_error("Unsupported JSON escape sequence.");
                }
            }

            throw std::runtime_error("Unterminated JSON string.");
        }

        JsonValue parseNumber()
        {
            std::size_t start = position;
            if (input[position] == '-')
            {
                ++position;
            }

            if (position >= input.size() || !std::isdigit(static_cast<unsigned char>(input[position])))
            {
                throw std::runtime_error("Invalid JSON number.");
            }

            while (position < input.size() && std::isdigit(static_cast<unsigned char>(input[position])))
            {
                ++position;
            }

            JsonValue value;
            value.type = JsonType::Number;
            value.numberValue = std::stoi(input.substr(start, position - start));
            return value;
        }

        int parseUnicodeEscape()
        {
            if (position + 4 > input.size())
            {
                throw std::runtime_error("Invalid JSON unicode escape.");
            }

            int value = 0;
            for (int i = 0; i < 4; ++i)
            {
                char c = input[position++];
                value *= 16;
                if (c >= '0' && c <= '9')
                {
                    value += c - '0';
                }
                else if (c >= 'a' && c <= 'f')
                {
                    value += c - 'a' + 10;
                }
                else if (c >= 'A' && c <= 'F')
                {
                    value += c - 'A' + 10;
                }
                else
                {
                    throw std::runtime_error("Invalid JSON unicode escape.");
                }
            }

            return value;
        }

        static void appendUtf8(std::string& output, int codePoint)
        {
            if (codePoint <= 0x7F)
            {
                output += static_cast<char>(codePoint);
            }
            else if (codePoint <= 0x7FF)
            {
                output += static_cast<char>(0xC0 | ((codePoint >> 6) & 0x1F));
                output += static_cast<char>(0x80 | (codePoint & 0x3F));
            }
            else
            {
                output += static_cast<char>(0xE0 | ((codePoint >> 12) & 0x0F));
                output += static_cast<char>(0x80 | ((codePoint >> 6) & 0x3F));
                output += static_cast<char>(0x80 | (codePoint & 0x3F));
            }
        }

        void skipWhitespace()
        {
            while (position < input.size() && std::isspace(static_cast<unsigned char>(input[position])))
            {
                ++position;
            }
        }

        bool startsWith(const char* value) const
        {
            std::size_t i = 0;
            while (value[i] != '\0')
            {
                if (position + i >= input.size() || input[position + i] != value[i])
                {
                    return false;
                }
                ++i;
            }
            return true;
        }

        bool tryConsume(char expected)
        {
            if (position < input.size() && input[position] == expected)
            {
                ++position;
                return true;
            }
            return false;
        }

        void consume(char expected)
        {
            if (!tryConsume(expected))
            {
                throw std::runtime_error("Unexpected JSON token.");
            }
        }

        const std::string& input;
        std::size_t position = 0;
    };

    static LabelTemplate ParseTemplate(const std::string& json)
    {
        LabelTemplate label;
        JsonValue root;
        try
        {
            root = JsonParser(json).parse();
        }
        catch (const std::exception&)
        {
            return label;
        }

        if (root.type != JsonType::Object)
        {
            return label;
        }

        label.name = GetString(root, "name", label.name);
        label.labelWidthDots = GetInt(root, "labelWidthDots", label.labelWidthDots);
        label.labelHeightDots = GetInt(root, "labelHeightDots", label.labelHeightDots);
        label.marginLeftDots = GetInt(root, "marginLeftDots", label.marginLeftDots);
        label.marginTopDots = GetInt(root, "marginTopDots", label.marginTopDots);
        label.gapDots = GetInt(root, "gapDots", label.gapDots);
        label.mediaTrackingMode = MediaTrackingModeFromString(GetString(root, "mediaTrackingMode", "Gap"));
        label.orientation = LabelOrientationFromString(GetString(root, "orientation", "Portrait"));

        const JsonValue* elements = root.find("elements");
        if (!elements || elements->type != JsonType::Array)
        {
            return label;
        }

        for (const JsonValue& object : elements->arrayValues)
        {
            if (object.type != JsonType::Object)
            {
                continue;
            }

            LabelElementType type = LabelElementTypeFromString(GetString(object, "type", "Text"));
            std::string name = GetString(object, "name", GetString(object, "text", "Element"));
            std::string fieldKey = GetString(object, "fieldKey", name);
            std::string text = GetString(object, "text", "");
            int x = GetInt(object, "x", 20);
            int y = GetInt(object, "y", 20);

            if (type == LabelElementType::Barcode)
            {
                LabelElement element = LabelElement::Barcode(name, text, x, y, GetInt(object, "barcodeHeight", 60));
                element.fieldKey = fieldKey;
                element.barcodeModuleWidth = GetInt(object, "barcodeModuleWidth", 2);
                element.barcodeSymbology = BarcodeSymbologyFromString(GetString(object, "barcodeSymbology", "Code128"));
                element.barcodeHumanReadable = GetBool(object, "barcodeHumanReadable", true);
                element.rotation = FieldRotationFromString(GetString(object, "rotation", "Normal"));
                element.editable = GetBool(object, "editable", true);
                label.addElement(element);
            }
            else
            {
                LabelElement element = LabelElement::Text(
                    name,
                    text,
                    x,
                    y,
                    GetInt(object, "fontHeight", 30),
                    GetInt(object, "fontWidth", 30),
                    GetBool(object, "bold", false));
                element.fieldKey = fieldKey;
                element.fontName = GetString(object, "fontName", "0");
                element.boxWidth = GetInt(object, "boxWidth", 260);
                element.maxLines = GetInt(object, "maxLines", 1);
                element.rotation = FieldRotationFromString(GetString(object, "rotation", "Normal"));
                element.alignment = TextAlignmentFromString(GetString(object, "alignment", "Left"));
                element.italic = GetBool(object, "italic", false);
                element.underline = GetBool(object, "underline", false);
                element.multiline = GetBool(object, "multiline", false);
                element.variable = GetBool(object, "variable", true);
                element.autoFit = GetBool(object, "autoFit", false);
                element.wrap = GetBool(object, "wrap", false);
                element.editable = GetBool(object, "editable", true);
                label.addElement(element);
            }
        }

        return label;
    }

    static std::string GetString(const JsonValue& object, const std::string& key, const std::string& fallback)
    {
        const JsonValue* value = object.find(key);
        return value && value->type == JsonType::String ? value->stringValue : fallback;
    }

    static int GetInt(const JsonValue& object, const std::string& key, int fallback)
    {
        const JsonValue* value = object.find(key);
        return value && value->type == JsonType::Number ? value->numberValue : fallback;
    }

    static bool GetBool(const JsonValue& object, const std::string& key, bool fallback)
    {
        const JsonValue* value = object.find(key);
        return value && value->type == JsonType::Boolean ? value->boolValue : fallback;
    }

    static std::string EscapeJson(const std::string& value)
    {
        std::string escaped;
        for (char c : value)
        {
            switch (c)
            {
            case '\\':
            case '"':
                escaped += '\\';
                escaped += c;
                break;
            case '\n':
                escaped += "\\n";
                break;
            case '\r':
                escaped += "\\r";
                break;
            case '\t':
                escaped += "\\t";
                break;
            default:
                escaped += c;
                break;
            }
        }
        return escaped;
    }
};
