########################################################################################################################
###                                             CONFIGURATION                                                        ###

#	CDEBUG configuration
#	add these options at CDEBUG to configure code-internal debugging
#
#DEBUG 			enable debugging-output (high level)
#DEBUG_LL 		enable debugging-ouutput (low level) (enables high-level as well)
#
#ENABLE_TIME		add time-stamps to messages
#DEBUG_FILE		specify file to which debugging-log is stored
#					(if not defined run/debug-<time_stamp>.log is used)
#NO_FILE			don't use any file/console instead
#TIME_FORMAT		use a specific timeformat
#STACK_DEPTH		how many stack-elements should be displayed (default 100)
#
# Example: CDEBUG = DEBUG_LL ENABLE_TIME DEBUG_FILE=/some/path/file
#						will print all messages with time-stamp, enables low-level debugging
#						and write the debug-log to the specified file

CDEBUG = DEBUG_LL DEBUG_FILE=run/debug.txt

#compiler flags
STD = -std=c++14
DEBUG = -g
WARNINGS = -Wall -Wextra -pedantic
ADDITIONAL =
CADDITIONAL =
LADDITIONAL =

#files
OBJ_DIR = build/obj
BIN_DIR = build/bin
SRC = $(wildcard *.cpp)

#compiler/linker
CC = g++

########################################################################################################################
###                                      GENERAL MACROS                                                              ###
#DO NOT MODIFY

#shell
SHELL := /bin/bash

#TODO this part of the makefile is executed once for each target - optimizable
#processed version of compiler-flags
PCDEBUG = $(shell 	DF='';							\
													\
					CD=( $(CDEBUG) );				\
													\
					for i in "$${CD[@]}";			\
					do								\
						if [[ $$i = *=* ]];			\
						then						\
							DF="$$DF -D $$i";		\
						else						\
							DF="$$DF -D $$i=";		\
						fi;							\
					done;							\
													\
					echo $$DF; )					\

#compiler/linker flags
CFLAGS = -c $(STD) $(DEBUG) $(WARNINGS) $(ADDITIONAL) $(CADDITIONAL) $(PCDEBUG)
LFLAGS = $(STD) $(DEBUG) $(WARNINGS) $(ADDITIONAL) $(LADDITIONAL)

#files/directories
OBJS = $(addprefix $(OBJ_DIR)/, $(SRC:.cpp=.o))

#executable
DVL = $(BIN_DIR)/dvl

########################################################################################################################
###                                               GENERAL                                                            ###
# DO NOT MODIFY

all: clean build map


#base rules
.PHONY: clean
clean:
	rm -rdf build/**

.PHONY: rebuild
rebuild: clean build

.PHONY: doc
doc:
	doxygen ./util/doxy.cfg

default: all

map:
	gcc

build: depend $(DVL)

#build main application
$(DVL): $(OBJS)
	$(CC) $(LFLAGS) $(OBJS) -o $@

#compile objs
$(OBJ_DIR)/%.o: %.cpp
	$(CC) $(CFLAGS) $< -o $@

########################################################################################################################
###                                         DEPENDENCIES                                                             ###

DEPEND_F = util/depend.d

depend: $(DEPEND_F)

$(DEPEND_F): $(SRC)
	$(CC) -M -std=c++11 $(SRC) > $@

include $(DEPEND_F)

########################################################################################################################
###                                                   FILE SYSTEM                                                    ###
# DO NOT MODIFY

#assert existance of output-directory
$(DVL): | $(BIN_DIR)

$(OBJS): | $(OBJ_DIR)

$(OBJ_DIR):
	mkdir $(OBJ_DIR)

$(BIN_DIR):
	mkdir $(BIN_DIR)

########################################################################################################################
###                                          STATIC LIBRARIES                                                        ###
