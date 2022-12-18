# Immediate2D Changelog and Migration Guide

---

### v2 (Dec-2022) 

#### Breaking Changes

- Renamed drawing.h to immediate2d.h.

- The header and implementation have been merged into a single .h file (in the [stb](https://github.com/nothings/stb)-style).  Instead of compiling immediate2d.cpp separately, now, in exactly one of your own cpp files, add this line before the immediate2d.h include: `#define IMM2D_IMPLEMENTATION`.

- Instead of changing the Width/Height/Scale in the header itself, you can now use `#define IMM2D_WIDTH 640`, `#define IMM2D_WIDTH 480`, and `#define IMM2D_SCALE 2` just above your `IMM2D_IMPLEMENTATION` define in your own code.  Now you shouldn't ever need to modify the immediate2d.h header itself.

- Renamed the top-level user function called by the library to `run()` instead of `main()` (which could cause some name collisions depending on your environment). 

- Renamed `SetPixel` to `DrawPixel` to be more consistent with the rest of the drawing APIs.

- All the `bool filled` parameters on shape drawing got split out into separate fill/stroke colors, which can save a call in the best case and is more readable in the worst case. 

- Split `UseAntiAliasing(bool)` into the separate, parameterless `UseAntiAliasing()` and `StopAntiAliasing()`, which should be a little more legible to beginners.

#### New Stuff

- Added `LoadImage`, `DrawImage`, `ImageWidth`, and `ImageHeight` calls to work with images!  Many file types are supported along with reading from disk, resources embedded in the .exe, or directly from a Base64-encoded string.  Check the new examples to see how to use them. 

- There were many "secret" functions (if you read the implementation) that have now been exposed and documented in the header: `MakeColorHSB`, `DrawArc`, `DrawString`, `LastBufferedKey`, `ClearInputBuffer`, `PlayMusic`, and `ResetMusic`.

- Added buildAll.bat (and cleanAll.bat) which can build (and delete) the examples from a Visual Studio "Native Tools Command Prompt".  If you're interested in building your own project from the command line, these scripts might be a good place to start.
 
- The quick reference PDF has been updated to include HSB colors and image loading/drawing.

#### Fixes / Neutral Changes

- The internal GDI+ based line drawing now uses round end caps, which should join end-to-end better with larger thicknesses.

- Double-buffered `Present` could sometimes cause the screen to flicker. 

---

### v1 (Nov-2017)

- Initial release.