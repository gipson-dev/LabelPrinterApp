#pragma once

#include "core/LabelUnits.h"

#include <string>

enum class MediaSensingMode
{
    Gap,
    BlackMark,
    Continuous
};

enum class LabelOrientation
{
    Portrait,
    Landscape
};

struct PrinterSettings
{
    std::string printerName;
    int dpi = 203;
    int darkness = 15;
    int speedIps = 4;
    int copies = 1;
    MediaSensingMode mediaSensing = MediaSensingMode::Gap;
    LabelOrientation orientation = LabelOrientation::Landscape;

    double labelWidthInches = 2.25;
    double labelHeightInches = 0.75;
    double marginLeftInches = 0.0;
    double marginTopInches = 0.0;
    double gapInches = 0.125;

    int labelWidthDots() const { return inchesToDots(labelWidthInches); }
    int labelHeightDots() const { return inchesToDots(labelHeightInches); }
    int marginLeftDots() const { return inchesToDots(marginLeftInches); }
    int marginTopDots() const { return inchesToDots(marginTopInches); }
    int gapDots() const { return inchesToDots(gapInches); }
    int inchesToDots(double inches) const { return LabelUnits::roundDots(LabelUnits::inchesToDots(inches, dpi)); }
};
