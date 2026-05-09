## 2024-05-04 - [Tkinter Canvas Performance]
**Learning:** Continuously calling `create_*` and `delete()` on a Tkinter canvas at high frame rates (like 20fps) causes the internal Tcl/Tk item ID counter to increment infinitely, causing unnecessary memory churn and processing overhead. Even if visually identical, it is a significant anti-pattern.
**Action:** Always prefer `canvas.itemconfig(item_tag, ...)` to update properties of existing drawn items instead of deleting and recreating them. Additionally, memoize state to avoid executing expensive redraw operations entirely when input states haven't changed.

## 2026-05-09 - [Tkinter Geometry Updating for Polygons]
**Learning:** When using `canvas.coords()` to update `create_polygon` or lines with multiple points, Tkinter expects a single flattened sequence (or varargs) of x, y coordinates instead of a list of tuples. E.g. `[x1, y1, x2, y2...]`. Passing a list of tuples like `[(x1, y1), ...]` will throw a TclError during high-frequency update loops.
**Action:** When migrating from `delete` + `create_polygon([(x,y)])` to `canvas.coords(tag, ...)`, explicitly flatten the list of point coordinates.
