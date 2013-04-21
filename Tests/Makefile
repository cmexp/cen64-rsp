#  ============================================================================
#   Makefile for *NIX.
#
#   RSPSIM: Reality Signal Processor SIMulator.
#   Copyright (C) 2013, Tyler J. Stachecki.
#   All rights reserved.
#
#   This file is subject to the terms and conditions defined in
#   file 'LICENSE', which is part of this source code package.
#  ============================================================================
TARGET = rspsim

# ============================================================================
#  A list of files to link into the library.
# ============================================================================
SOURCES := $(wildcard *.c)
OBJECTS = $(addprefix $(OBJECT_DIR)/, $(notdir $(SOURCES:.c=.o)))

LIBDIRS = -L..
LIBS = -lrsp

# =============================================================================
#  Build variables and settings.
# =============================================================================
OBJECT_DIR=Objects

BLUE=$(shell tput setaf 4)
PURPLE=$(shell tput setaf 5)
TEXTRESET=$(shell tput sgr0)
YELLOW=$(shell tput setaf 3)

# ============================================================================
#  General build rules.
# ============================================================================
ECHO=/usr/bin/printf "%s\n"
MKDIR = /bin/mkdir -p

DOXYGEN = doxygen

WARNINGS = -Wall -Wextra -pedantic
RSP_FLAGS = -DLITTLE_ENDIAN -DUSE_SSE

COMMON_CFLAGS = $(WARNINGS) $(RSP_FLAGS) -std=c99 -march=native -I..
COMMON_CXXFLAGS = $(WARNINGS) $(RSP_FLAGS) -std=c++0x -march=native -I..
OPTIMIZATION_FLAGS = -flto -fwhole-program -fuse-linker-plugin \
	-fdata-sections -ffunction-sections -funsafe-loop-optimizations

RELEASE_CFLAGS = -DNDEBUG -O3 $(OPTIMIZATION_FLAGS)
DEBUG_CFLAGS = -DDEBUG -O0 -ggdb -g3

$(OBJECT_DIR)/%.o: %.c
	@$(MKDIR) $(OBJECT_DIR)
	@$(ECHO) "$(BLUE)Compiling$(YELLOW): $(PURPLE)$(PREFIXDIR)$<$(TEXTRESET)"
	@$(CC) $(CFLAGS) $< -c -o $@

# ============================================================================
#  Targets.
# ============================================================================
all: CFLAGS = $(COMMON_CFLAGS) $(DEBUG_CFLAGS) $(RSP_FLAGS)
all: $(TARGET)

all-cpp: CFLAGS = $(COMMON_CXXFLAGS) $(DEBUG_CFLAGS) $(RSP_FLAGS)
all-cpp: $(TARGET)
all-cpp: CC = $(CXX)

$(TARGET): $(OBJECTS)
	@$(ECHO) "Building librsp..."
	@$(MAKE) -C ..

	@$(ECHO) "$(BLUE)Linking$(YELLOW): $(PURPLE)$@$(TEXTRESET)"
	@$(CC) $(CFLAGS) $(LDFLAGS) $(LIBDIRS) $(OBJECTS) $(LIBS) -o $@

.PHONY: clean documentation inspect inspect-cpp

clean:
	@$(ECHO) "$(BLUE)Cleaning tests...$(TEXTRESET)"
	@$(RM) $(OBJECTS) $(TARGET)

inspect: $(TARGET)
	objdump -d $< | less

inspect-cpp: $(TARGET)
	objdump -d $< | c++filt | less

