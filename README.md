# Revolution SDK WPAD library decompilation (Dec. 11, 2009)

This repository contains a (mostly) matching decompilation of the WPAD library in the Revolution SDK, timestamped `Dec 11 2009`.

Matches are mainly against the functions present in the release and debug ELF binaries from [`[SPQE7T]`](https://wiki.dolphin-emu.org/index.php?title=SPQE7T) *I Spy: Spooky Mansion*, but some are against functions present in the release and debug ELF binaries from [`[SGLEA4]`](https://wiki.dolphin-emu.org/index.php?title=SGLEA4) *Gormiti: The Lords of Nature!*. All symbol names come from the *Gormiti* maps. Struct definitions and other names come directly from DWARF info from both games, as well as other sources listed where referenced.

These binaries do not contain every function in the library (the *Gormiti* maps show exactly what is missing), so only the functions that were present in the binaries are matched here. Your decomp may have some of the extra functions that are not in this repository; you will have to do those yourself, but hopefully the surrounding source can help you with that.

## Building

### Prerequisites
- [`mkdir`](https://en.wikipedia.org/wiki/mkdir) that supports the `-p` flag
- [Make](https://en.wikipedia.org/wiki/Make_(software))
- Metrowerks Wii 1.0 Toolchain
	- `mwcceppc.exe` (*version 4.3, build 145*)
	- `mwldeppc.exe` (*version 4.3, build 145*)
- [Wine](https://wiki.winehq.org/Download) or equivalent (if not compiling natively under Windows)

### Instructions

In the makefile, set the `MWERKS` variable to the path of `mwcceppc.exe` (and `WINE` to the path of Wine, if applicable).

Then run
- `make wpad` to create `lib/wpad.a`,
- `make wpadD` to create `lib/wpadD.a`, or
- `make build/`(`release`/`debug`)`/<file>.o` to compile a specific file, if you're playing around with the source. For example, to compile the release version of `source/WPAD.c`, run `make build/release/WPAD.o`.
- `make clean` comes included.

## Porting to a decompilation

No guarantees about similarity can be made with versions with timestamps outside of a few months of this version, but this source could be a good starting point if it's only a little different.

`source/` should be mostly drag-and-drop into wherever you keep your SDK source, though you may have problems with include directives not finding local headers if you use `-cwd explicit` or `-cwd project`.

The only headers in `include/` you need to take are the WPAD headers in `revolution/` and `context_bte.h`; perhaps `context_rvl.h`, if you don't have some of those declarations. The rest (primarily `stdlib/`) is context; you should use your decomp's own versions of those headers.

Integrating this library into your decomp's build system is going to be specific to your decomp, and so is not covered here.

## Contribution

By its nature, matching decompilation *has* a finish line, and this source is almost there. The few remaining unmatched functions in this repository are

- [ ] `WPAD.c`: `__wpadInitSub` (optimization stuff?)
- [ ] `lint.c`: `LINTMul` (64 bit math)
- [ ] `WUD.c`: `__wudWritePatch` (inlining shenanigans)

as well as some fakematches scattered around.

PRs with functions not already in this repository are fine too, just make sure they match against this specific version of the library.
