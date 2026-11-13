# LuxMeter Logger Design Direction

- **Identity:** Precision bench instrument companion for a handheld ESP32 + TSL2591 lux meter.
- **Audience:** Makers, engineers, and technicians logging ambient light on Windows or Android.
- **Personality:** Calm, factual, legible at arm's length. The live reading is the primary focal point; controls and history stay quiet until needed.
- **Theme:** Dark default (`#141311` ground, `#1c1b18` instrument panels) so a phone or laptop screen next to the sensor does not spill stray light into low-lux measurements, paired with a high-contrast light mode (`#f4f3ef` ground, `#ffffff` panels) for bright labs and outdoor use.
- **Palette:** Warm stone neutrals (`#141311`, `#1c1b18`, `#3a3832`, `#ecebe6`, `#a9a59b` in dark; `#f4f3ef`, `#ffffff`, `#d6d3ca`, `#1d1c19`, `#5c5850` in light) + 1 deliberate accent: warm amber (`#f2b338` dark / `#9a6200` light) reserved strictly for the measurement itself (live lux value, logarithmic scale marker, graph trace, and live status dot) + semantic alert red (`#ff8a7a` dark / `#b3261e` light).
- **Typography:** System sans-serif (`system-ui, -apple-system, "Segoe UI", Roboto, sans-serif`) with `font-variant-numeric: tabular-nums` on every numeric readout, scale tick, axis label, and table column so digits never jitter as readings stream in at 2 Hz.
- **Identity Motif:** Logarithmic 7-decade instrument scale (`0.01` to `100k` lux) beneath the primary readout showing where the current reading sits across the TSL2591's full dynamic range.
- **Dial:** ENERGY 1 / RHYTHM 2 / MOTION 1
