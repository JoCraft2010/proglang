CXX := g++
LLC := llc
CC := gcc

CXXFLAGS ?= -Wall -std=c++17 -pipe -march=native
LLCFLAGS ?= -relocation-model=dynamic-no-pic
CCFLAGS := -no-pie -pipe

CXXFLAGS_VERBOSE := -D VERBOSE

ifneq ($(shell cat /etc/os-release | grep -q "ID=arch" && echo "arch" || echo "other"), arch)
    LLCFLAGS += -opaque-pointers
endif

COMPILER_SRC_DIR := compiler/src
COMPILER_SRC := $(wildcard $(COMPILER_SRC_DIR)/*.cpp)
COMPILER_BIN_DIR := compiler/bin
COMPILER_BIN := $(COMPILER_BIN_DIR)/compiler

LANGUAGE_SRC_DIR := language/src
LANGUAGE_SRC := $(LANGUAGE_SRC_DIR)/main.proglang
LANGUAGE_TEMP_DIR := language/temp
LANGUAGE_LL := $(LANGUAGE_TEMP_DIR)/main.ll
LANGUAGE_ASM := $(LANGUAGE_TEMP_DIR)/main.s
LANGUAGE_BIN_DIR := language/bin
LANGUAGE_BIN := $(LANGUAGE_BIN_DIR)/main

HANGMAN_SRC := $(LANGUAGE_SRC_DIR)/hangman.proglang

MKDIR_P := mkdir -p

REMOVE_TEMP_FILES := rm -r $(LANGUAGE_TEMP_DIR)

all: compiler build_proglang run_result

compiler_verbose: CXXFLAGS += $(CXXFLAGS_VERBOSE)
compiler_verbose: compiler

compiler:
	$(MKDIR_P) $(COMPILER_BIN_DIR)
	$(CXX) $(CXXFLAGS) $(COMPILER_SRC) -o $(COMPILER_BIN)

build_proglang:
	$(MKDIR_P) $(LANGUAGE_TEMP_DIR) $(LANGUAGE_BIN_DIR)
	$(COMPILER_BIN) $(LANGUAGE_SRC) $(LANGUAGE_LL)
	$(LLC) $(LLCFLAGS) $(LANGUAGE_LL)
	$(CC) $(CCFLAGS) $(LANGUAGE_ASM) -o $(LANGUAGE_BIN)
	$(REMOVE_TEMP_FILES)

build_hangman:
	$(MKDIR_P) $(LANGUAGE_TEMP_DIR) $(LANGUAGE_BIN_DIR)
	$(COMPILER_BIN) $(HANGMAN_SRC) $(LANGUAGE_LL)
	$(LLC) $(LLCFLAGS) $(LANGUAGE_LL)
	$(CC) $(CCFLAGS) $(LANGUAGE_ASM) -o $(LANGUAGE_BIN)
	$(REMOVE_TEMP_FILES)

run_result:
	$(LANGUAGE_BIN)

.PHONY: all compiler_verbose compiler build_proglang build_hangman run_result
