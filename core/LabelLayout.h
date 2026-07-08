#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "core/LabelGeometry.h"
#include "core/LabelTemplate.h"
#include "core/VariableResolver.h"

enum class CanvasLayer
{
    Background = 0,
    Label = 100,
    Grid = 150,
    Elements = 200,
    ActiveElements = 300,
    Selection = 500,
    Handles = 600,
    Guides = 700,
    Overlay = 1000
};

struct PrinterTextCalibration
{
    double xOffsetDots = 0.0;
    double yOffsetDots = 0.0;
    double widthScale = 1.0;
    double heightScale = 1.0;
};

struct LabelResolveOptions
{
    bool fillMissingSampleData = false;
    PrinterTextCalibration textCalibration;
};

struct ResolvedLabelElement
{
    std::size_t sourceIndex = 0;
    int zIndex = 0;
    CanvasLayer layer = CanvasLayer::Elements;
    LabelElement element;
    std::string value;
    LabelRect contentBoundsDots;
    LabelRect selectionBoundsDots;
    int fontHeightDots = 0;
    int fontWidthDots = 0;
    int textLineCount = 1;
    std::vector<std::string> textLines;
};

struct ResolvedTextLayout
{
    std::size_t sourceIndex = 0;
    std::string id;
    std::string text;
    double xDots = 0.0;
    double yDots = 0.0;
    double boxWidthDots = 0.0;
    double boxHeightDots = 0.0;
    double glyphOriginXDots = 0.0;
    double glyphOriginYDots = 0.0;
    double fontHeightDots = 0.0;
    double fontWidthDots = 0.0;
    std::vector<std::string> lines;
    ElementRotation rotation = ElementRotation::Deg0;
};

struct ResolvedLabelLayout
{
    int dpi = 203;
    double labelWidthDots = 0.0;
    double labelHeightDots = 0.0;
    std::vector<ResolvedLabelElement> elements;
    std::vector<ResolvedTextLayout> textLayouts;
};

class LabelLayoutEngine
{
public:
    static ResolvedLabelLayout resolve(
        const LabelTemplate& labelTemplate,
        const VariableContext& context = {},
        const LabelResolveOptions& options = {});

    static LabelRect elementModelBoundsDots(
        const LabelTemplate& labelTemplate,
        const LabelElement& element,
        const VariableContext& context = {},
        const LabelResolveOptions& options = {});

    static int effectiveTextFontHeightDots(
        const LabelTemplate& labelTemplate,
        const LabelElement& element,
        const std::string& value);

    static int effectiveTextFontWidthDots(const LabelElement& element, int effectiveFontHeightDots);
    static int textLineCount(const LabelElement& element);
    static double textBoxHeightDots(const LabelElement& element, int effectiveFontHeightDots);
    static double estimatedTextWidthDots(const std::string& text, int fontWidthDots);
    static std::vector<std::string> layoutTextLines(
        const LabelElement& element,
        const std::string& value,
        int effectiveFontWidthDots,
        double boxWidthDots);
};
