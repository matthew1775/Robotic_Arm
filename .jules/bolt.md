## 2024-05-04 - [Tkinter Canvas Performance]
**Learning:** Continuously calling `create_*` and `delete()` on a Tkinter canvas at high frame rates (like 20fps) causes the internal Tcl/Tk item ID counter to increment infinitely, causing unnecessary memory churn and processing overhead. Even if visually identical, it is a significant anti-pattern.
**Action:** Always prefer `canvas.itemconfig(item_tag, ...)` to update properties of existing drawn items instead of deleting and recreating them. Additionally, memoize state to avoid executing expensive redraw operations entirely when input states haven't changed.

## 2024-05-24 - Tkinter Config Call Overhead
**Learning:** In Tkinter, calls to `.config()` on labels and `.itemconfig()` on canvas elements involve crossing the C/Tcl-Tk boundary and are relatively expensive, even if the new value is identical to the old one. During a continuous 20Hz UI update loop, doing this unconditionally creates a significant baseline CPU usage overhead.
**Action:** Always memoize (cache) the state of Tkinter UI elements locally, and only issue `.config()` or `.itemconfig()` calls when the computed text, color, or dimension is distinctly different from the previously rendered state.
