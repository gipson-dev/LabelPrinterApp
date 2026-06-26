# Printer Calibration

These notes are for Zebra ZD410, ZD620, and similar ZPL desktop printers.

## Load Uline S-8599 Labels

1. Open the printer cover.
2. Place the Uline S-8599 roll on the holder with labels feeding from the bottom/front of the roll.
3. Feed the label web under the guides and through the front slot.
4. Adjust the guides so they touch the label edges without bending the liner.
5. Close the cover and press Feed once.

For Uline S-8599 stock, use 2.25 x 0.75 inch direct thermal gap labels.

## Calibrate Media

Use calibration when labels skip, stop halfway, or print between labels.

From ZPL, send:

```zpl
~JC
```

Then press Feed once or twice. The printer should advance to the next label gap and stop cleanly at the tear edge.

## Set Printer To ZPL Mode

Use the Zebra driver or Zebra Setup Utilities:

1. Open the Zebra printer settings.
2. Confirm the printer language is ZPL.
3. Disable EPL mode unless you intentionally use EPL labels.
4. Print a configuration label if needed and confirm the language reports ZPL.

## Confirm 203 vs 300 DPI

Check the printer model label, Zebra driver, or configuration printout.

- 203 DPI: 2.25 x 0.75 inches is `457 x 152` dots.
- 300 DPI: 2.25 x 0.75 inches is `675 x 225` dots.
- 203 DPI: 4 x 2 inches is `812 x 406` dots.
- 300 DPI: 4 x 2 inches is `1200 x 600` dots.

In LabelPrinterApp, select the matching DPI in Settings before previewing or printing.

## Adjust Darkness

Use darkness when print is too light, too dark, or barcodes scan poorly.

- Increase darkness if text or bars are faint.
- Decrease darkness if bars bleed together or text looks heavy.
- Start around `15` for direct thermal labels and adjust in small steps.

## Adjust Speed

Use speed when print quality is inconsistent.

- Slower speed usually improves barcode quality.
- Faster speed may be acceptable for simple text labels.
- Start around `4 ips` for small direct thermal labels.

## Fix Alignment

If labels print too far left or right:

- Check that the roll guides are snug.
- Confirm the correct label width is selected.
- Adjust X positions in the designer.
- Use label left margin only for broad stock-level correction.

If labels print too far up or down:

- Calibrate media with `~JC`.
- Confirm the correct label height and gap.
- Confirm the current build includes the corrected normal-text vertical origin and that text is vertically centered in the designer selection box.
- Adjust Y positions in the designer.
- Use label top margin only for broad stock-level correction.

If each label drifts farther over time:

- Recalibrate media.
- Confirm media type is Gap Labels for Uline S-8599.
- Confirm the liner gap sensor is not blocked by dust or adhesive.

## Print A Test ZPL Label

For a 203 DPI Zebra printer with 2.25 x 0.75 inch stock:

```zpl
^XA
^PW457
^LL152
^LH0,0
^FO40,30
^A0N,35,35
^FDCALIBRATION TEST^FS
^XZ
```

Expected result: one centered `CALIBRATION TEST` label. If the text is clipped, shifted, or lands on the next label, recalibrate and confirm the selected DPI/stock size.
