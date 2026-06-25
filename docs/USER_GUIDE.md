# LabelPrinterApp User Guide

This guide covers the normal operator workflow for designing, importing, previewing, and printing Zebra labels.

## First Run

1. Start `LabelPrinterApp.exe`.
2. Open the Settings tab.
3. Select the installed Zebra printer.
4. Select the label stock preset.
5. Confirm the DPI matches the printer, usually 203 or 300.
6. Click `Save Settings`.

For Uline S-8599 at 203 DPI, the calculated size should show `457 x 152` dots.

## Make A Basic Label

1. Open the Design tab.
2. Select the blank canvas from `Canvas Template`.
3. Click `Number`.
4. Click `Description`.
5. Move the fields by dragging them on the canvas.
6. Resize the selected field by dragging the blue side or corner anchors.
7. Use the right-side `Element Property Editor` to adjust formatting, data source, position, barcode settings, and print options.

The standard data fields are `Number` and `Description`.

## Resize Elements

When an element is selected, blue anchors appear around it.

- Drag left or right side anchors to change width.
- Drag top or bottom anchors to change height.
- Drag corner anchors to change width and height together.
- Text height changes font height.
- Text width changes the text box width.
- Barcode height changes bar height.
- Barcode width adjusts module width.
- QR resizing adjusts QR magnification.

Use larger font sizes from the Font Size dropdown when you need bold shelf-style or item-number labels. Font sizes above `72 dots` are available.

## Import CSV Data

The recommended CSV format is:

```csv
Number,Description
TEST-001,"Test description"
TEST-002,"Second test description"
```

To print imported records:

1. Open the Data tab.
2. Click `Load...`.
3. Choose the CSV file.
4. Check rows in the `Print` column or highlight table rows.
5. Make sure the current label has fields using `{Number}` and `{Description}`.
6. Click `Print Selected CSV` or `Print All CSV`.

The app also supports `.xlsx` files through the same Data tab table.

## Manual User Input

If no CSV or Excel row is active, variable fields still work. When printing, the app prompts for values such as `Number` and `Description`.

Use this for one-off labels:

1. Add `Number` and `Description` fields.
2. Leave the Data tab unloaded.
3. Click `Quick Print` or print from the Print tab.
4. Enter the prompted values.

## Preview ZPL

Use `Preview ZPL` before printing a new design. Check that the generated ZPL contains visible fields such as:

```zpl
^FDTEST-001^FS
^FDTest description^FS
```

If the app warns that the template is blank, add at least one printable text, barcode, or QR element before printing.

## Print Safely

1. Print one test label first.
2. Confirm the label stops at the right gap.
3. Confirm barcode text scans or reads correctly.
4. Adjust darkness and speed if print quality is too light, heavy, or blurry.
5. Print the full selected batch only after the test label is correct.

Print results are logged to `logs\print_history.csv`.
