# Revolution SDK WPAD library decompilation (Dec. 11, 2009)

This repository contains a (mostly) matching decompilation of the WPAD library in the Revolution SDK, timestamped `Dec 11 2009`.

Matches are against the debug and release objects present in the files of  [`[SC5PGN]`](https://wiki.dolphin-emu.org/index.php?title=SC5PGN) *Challenge Me: Word Puzzles*. Struct definitions for the external API come from DWARF info from debug binaries from [`[SPQE7T]`](https://wiki.dolphin-emu.org/index.php?title=SPQE7T) *I Spy: Spooky Mansion* and  [`[SGLEA4]`](https://wiki.dolphin-emu.org/index.php?title=SGLEA4) *Gormiti: The Lords of Nature!*, as well as other sources listed where referenced. Internal structures within the library did not have such DWARF info, and some names might (will be) inaccurate as a result.

These objects contain every function in the library as provided to developers. They do not contain uncompiled code, such as blocks of code that were `/* comment */`ed or `#ifdef`'d out at compile time. Because of this, the usage of some code is still unknown, and likely will be forever.

## Building

### Prerequisites
- A version of [`mkdir`](https://en.wikipedia.org/wiki/mkdir) that supports the `-p` flag
- [Make](https://en.wikipedia.org/wiki/Make_(software))
- Metrowerks Wii 1.0 Toolchain
	- `mwcceppc.exe` (*version 4.3, build 145*)
	- `mwldeppc.exe` (*version 4.3, build 145*)
- [Wine](https://wiki.winehq.org/Download) or equivalent, if not compiling natively under Windows

### Instructions

In the makefile, set the `MWERKS` variable to the path of `mwcceppc.exe` (and `WINE` to the path of Wine, if applicable).

Then run
- `make wpad` to create `lib/wpad.a`,
- `make wpadD` to create `lib/wpadD.a`, or
- `make build/`(`release`/`debug`)`/<file>.o` to compile a specific file, if you're playing around with the source. For example, to compile the release version of `source/WPAD.c`, run `make build/release/WPAD.o`.
- `make clean` comes included.

## Adding to a decompilation

No guarantees about similarity can be made with versions with timestamps outside of a few months of this version, but this source could be a good starting point if it's only a little different.

`source/` should be mostly drag-and-drop into wherever you keep your SDK source, though you may have problems with include directives not finding local headers if you use `-cwd explicit` or `-cwd project`.

The only headers in `include/` you need to take are the WPAD headers in `revolution/` and `context_bte.h`; perhaps `context_rvl.h`, if you don't have some of those declarations. The rest (primarily `stdlib/`) is context; you should use your decomp's own versions of those headers.

> [!NOTE]
> If you do use your own definitions instead of `context_rvl.h`, make sure that the code still matches. If it does not match or compile, replace offending code with the equivalent code in `context_rvl.h` as necessary. If this regresses or breaks other code, you may need to refactor some headers.

Integrating this library into your decomp's build system is going to be specific to your decomp, and so is not covered here.
However, a guide to add this library to a project using the [dtk-template](https://github.com/encounter/dtk-template) build system may be added in the future, as it is becoming an increasingly popular base for decomps of Gamecube and Wii games.

## Contribution

By its nature, matching decompilation *has* a finish line, and this source is almost there. The few remaining unmatched functions in this repository are

- [x] ~~`WPAD.c`: `__wpadInitSub` (optimization stuff?)~~
- [ ] `lint.c`: `LINTMul` (64 bit math)
- [x] ~~`WUD.c`: `__wudWritePatch` (inlining shenanigans)~~

as well as some fakematches scattered around. PRs focusing on documentation would be more appreciated.
