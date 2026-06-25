# Classic Label Designer UI

## Goal

LabelPrinterApp now uses a classic LabelDirect/ZebraDesigner-style desktop layout while keeping the modern Qt/ZPL feature set: templates, text fields, barcodes, QR codes, Excel/CSV records, preview, ZPL generation, and raw Zebra printing.

## Current Layout

```text
+--------------------------------------------------------------------------------+
| File Insert Edit Layout View Stock Database Printer Preferences Templates Help   |
+--------------------------------------------------------------------------------+
| New Open Save Print Cut Copy Paste Undo Redo Zoom +/- Fit Text Barcode QR ...    |
+--------------+---------------------------------------------+-------------------+
| Toolbox      | inch ruler                                  | Element Property  |
| Canvas       |---------------------------------------------| Editor            |
| Template     |                                             | Text Formatting   |
| Select       |   gray workspace                            | Position          |
| Text         |      +-------------------------------+      | Data Barcode      |
| Number       |      | white label with grid          |      | Print             |
| Description  |      | blue resize anchors            |      |                   |
| Barcode      |      | dashed printable boundary      |      |                   |
| QR Code      |      | white label with grid          |      | Print             |
| Date/Time    |      | dashed printable boundary      |      |                   |
| Serial #     |      | blue selection handles         |      | Filtered fields   |
| Line         |      +-------------------------------+      | for active page   |
| Box          |                                             |                   |
| Image        |                                             |                   |
+--------------+---------------------------------------------+-------------------+
| Align left | Align center | Align right | Align top | Align middle | ...         |
+--------------------------------------------------------------------------------+
| For Help, press F1                         | Label 2.25 x 0.75 | DPI 203 | Zoom |
+--------------------------------------------------------------------------------+
```

## Qt Widgets

- `QMainWindow`: menu bar, main toolbar, workflow tabs, bottom alignment toolbar, status bar.
- `QToolBar`: compact command strips for file/edit/view/layout actions.
- `QTabWidget`: workflow pages for Design, Elements, Data, Templates, Print, and Settings.
- `QSplitter`: movable left toolbox, center designer, and right property editor sections.
- `QFrame`: classic framed left toolbox and right property panel.
- `PreviewWidget`: custom painter for rulers, canvas, optional grid, label outline, printable margin, elements, movement, and side/corner resize handles.
- `ElementEditorWidget`: filtered property pages for Text, Formatting, Position, Data, Barcode, and Print.
- `ExcelRecordsWidget`: editable records table for CSV and `.xlsx` database printing.

## Visual Style

- App background: `#ececec`
- Toolbar/panels: `#eeeeee`
- Canvas workspace: `#d6d6d6`
- Rulers: `#ebebeb`
- Label: white with a dark outline
- Printable margin: dashed dark outline
- Grid: pale blue-gray dotted lines
- Selection: strong blue outline with large blue/white handles
- Checkbox indicators: high-contrast border and checked fill
- Font: Segoe UI/Arial, compact 9 pt

## Inspector Pages

The right-side header reads `Element Property Editor`. The section buttons are functional filtered pages:

- `Text`: name, type, and text value.
- `Formatting`: font size dropdown, font width, bold, italic, underline, wrap, auto-fit, max lines, and alignment.
- `Position`: X, Y, box width, and rotation.
- `Data`: source mode, variable name, prefix, and suffix.
- `Barcode`: Code 128/Code 39 height, module width, human-readable toggle, or QR magnification.
- `Print`: do-not-print and locked toggles.

The filtered page design prevents the old long property list from making the page buttons look inactive.
The bottom-right `Quick Print` button sends the current label to the selected printer from the Design tab.

## Designer Behavior

- Click elements in the preview to select them.
- Use the Design tab's `Canvas Template` selector to load a blank 2.25 x 0.75 or 4 x 2 canvas without switching tabs.
- Drag unlocked elements to move them.
- Drag selected side/corner anchors to resize elements. Text resize adjusts box width and font height, barcode resize adjusts width/height, and QR resize adjusts magnification.
- Use the `Grid` toolbar button to show or hide the design grid.
- Use the `Snap` toolbar button to snap dragged elements to the 0.25 inch design grid.
- Locked elements remain selectable but do not drag.
- The status bar shows cursor position, label size, DPI/dot size, and zoom.
- The bottom layout toolbar applies to the selected element:
  - Align left/center/right/top/middle/bottom
  - Equal spacing for three or more elements
  - Bring forward/send backward for layer order
  - Lock/unlock
- Toolbar layout actions mirror the bottom toolbar where practical.
- Preferences and Settings actions can save the current app setup or reset saved app settings. Startup reloads the saved printer, stock, DPI, label setup, grid/snap toggles, active tab, and window layout.
- Print actions append simple success/failure history to `logs\print_history.csv`.

## Stock And Preview Notes

The default preset is Uline S-8599 2.25 x 0.75 direct thermal stock.

- 203 DPI: `457 x 152` dots
- 300 DPI: `675 x 225` dots

The Settings tab also includes Uline S-22422, Zebra 2.25 x 0.75 generic, and Zebra ZD620 4 x 2 direct thermal presets.

The bundled Templates tab content is split into blank 2.25 x 0.75 and blank 4 x 2 starting templates. The app does not prefill production-looking label content. The standard imported-data workflow uses `Number` and `Description` fields.
