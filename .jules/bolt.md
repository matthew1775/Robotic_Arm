## 2024-05-04 - [Tkinter Canvas Performance]
**Learning:** Continuously calling `create_*` and `delete()` on a Tkinter canvas at high frame rates (like 20fps) causes the internal Tcl/Tk item ID counter to increment infinitely, causing unnecessary memory churn and processing overhead. Even if visually identical, it is a significant anti-pattern.
**Action:** Always prefer `canvas.itemconfig(item_tag, ...)` to update properties of existing drawn items instead of deleting and recreating them. Additionally, memoize state to avoid executing expensive redraw operations entirely when input states haven't changed.

## $(date +%Y-%m-%d) - Tkinter Canvas Optimization
**Learning:** In high-frequency GUI loops using `tkinter.Canvas`, calling `c.delete("all")` and `c.create_*()` on every frame causes significant CPU overhead and memory leaks due to unbounded internal Tcl/Tk ID incrementing. The previous code was recreating shapes ~20 times per second needlessly.
**Action:** Always use tags (e.g., `tags="arm_base"`) and update elements via `c.coords()` and `c.itemconfig()`. Remember that `c.coords()` for a `create_polygon` takes a flattened list of coordinates `[x1, y1, x2, y2, ...]`.
