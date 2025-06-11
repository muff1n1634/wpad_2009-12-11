# basic makefile

# paths (can also pass via command line)
MKDIR	?=	mkdir # should support POSIX '-p'
WINE	?=	wine # wine path here if not building on Windows natively
MWERKS	?=	# mwcc path here (Wii 1.0; should also have mwld in the same directory)

# targets
.PHONY: help wpad wpadD clean

help:
	@echo 'options:'
	@echo '    wpad                   creates wpad.a (release)'
	@echo '    wpadD                  creates wpadD.a (debug)'
	@echo
	@echo '    build/release/<file>.o compiles <file>.o (release)'
	@echo '    build/debug/<file>.o   compiles <file>.o (debug)'
	@echo
	@echo '    clean                  cleans up build artifacts'

wpad: lib/wpad.a
wpadD: lib/wpadD.a
clean:; -@rm -rf build/ lib/

# system include search directories
export MWCIncludes = include:include/stdlib

# flags
flags_main = -proc gekko -fp hardware -lang c99 -enum int -cpp_exceptions off -cwd include -enc UTF-8 -flag no-cats

flags_opt_release = -O4,p -ipa file -DNDEBUG
flags_opt_debug = -opt off -inline off -gdwarf-2

# source files
wpad_src :=	\
	WPAD.c			\
	WPADHIDParser.c	\
	WPADEncrypt.c	\
	WPADClamp.c		\
	WPADMem.c		\
	lint.c			\
	WUD.c			\
	WUDHidHost.c

.PHONY: envcheck

envcheck:
ifeq ($(strip ${MWERKS}),)
	$(error MWERKS not set)
endif

# object files
build/release/%.o: source/%.c envcheck
	@${MKDIR} -p build/release/$(dir $*)
	${WINE} ${MWERKS} ${flags_main} ${flags_opt_release} ${FLAGS} -o $@ -c $<

build/debug/%.o: source/%.c envcheck
	@${MKDIR} -p build/debug/$(dir $*)
	${WINE} ${MWERKS} ${flags_main} ${flags_opt_debug} ${FLAGS} -o $@ -c $<

# library archives
.NOTPARALLEL: lib/wpad.a lib/wpadD.a

lib/wpad.a: $(foreach f,$(basename ${wpad_src}),build/release/$f.o)
	@${MKDIR} -p lib/
	${WINE} ${MWERKS} ${flags_main} -o $@ -library $^

lib/wpadD.a: $(foreach f,$(basename ${wpad_src}),build/debug/$f.o)
	@${MKDIR} -p lib/
	${WINE} ${MWERKS} ${flags_main} -o $@ -library $^