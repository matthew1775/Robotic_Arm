## 2024-05-04 - [Tkinter Canvas Performance]
**Learning:** Continuously calling `create_*` and `delete()` on a Tkinter canvas at high frame rates (like 20fps) causes the internal Tcl/Tk item ID counter to increment infinitely, causing unnecessary memory churn and processing overhead. Even if visually identical, it is a significant anti-pattern.
**Action:** Always prefer `canvas.itemconfig(item_tag, ...)` to update properties of existing drawn items instead of deleting and recreating them. Additionally, memoize state to avoid executing expensive redraw operations entirely when input states haven't changed.
## 2024-05-18 - Tkinter Redraw Overhead
**Learning:** In Tkinter loops (like the 20FPS `update_interface` in `gui.py`), repeatedly calling `.config()` or `.itemconfig()` forces the application to cross the Python-Tcl/Tk boundary and queues redraw events, creating high CPU usage even if the visual output doesn't change.
**Action:** Memoize widget states (e.g., label text, circle color, or degrees) and wrap `.config()` and `.itemconfig()` calls in conditionals that only execute if the underlying data has changed.
