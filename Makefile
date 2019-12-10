
GCC=g++
CXXFLAGS=-g -Wall -m32 -Wno-write-strings -D__STDC_FORMAT_MACROS -std=c++0x -Wno-unused-variable
LIBS=-lm $(shell $(GCC) $(CXXFLAGS) --print-file-name=libreadline.a ) $(shell $(GCC) $(CXXFLAGS) --print-file-name=libtermcap.a )


TARGET_BIN=setpar

OBJ_BIN=o/setpar.o o/filestruct.o o/readfile.o o/paramdef.o o/data.o o/consoleinterface.o o/command.o o/binaryval.o \
	o/cmd/firmware.o  o/cmd/help.o  o/cmd/list.o  o/cmd/module.o  o/cmd/save.o  o/cmd/set.o o/cmd/sfp.o o/cmd/display.o \
	o/cmd/file.o o/cmd/slist.o o/cmd/tlist.o o/cmd/batchfile.o \
	o/fw/hooks/febex_1.3.o o/fw/hooks/pulser_2.0.o

.PHONY: all clean presets

all: $(TARGET_BIN) presets

febex_set_param: all

-include $(OBJ_BIN:.o=.d)

$(TARGET_BIN): $(OBJ_BIN)
	@/bin/bash -c "echo -e \"\e[1;33mLD\e[0m $@\""
	$(GCC)   $(CXXFLAGS) $+ $(LIBGOSIP_A) $(LIBS) -o $@
	@/bin/bash -c "echo -e \"\e[1;32mBuilding complete\e[0m\""

o/%.o: src/%.c o/.dirs
	$(GCC) $(CXXFLAGS) -c $< -o $@

o/%.o: src/%.cxx o/.dirs
	$(GCC) $(CXXFLAGS) -c $< -o $@

#	@/bin/bash -c "echo -e \"\e[1;33mCC\e[0m $@\""
#	
#	@$(GCC) $(GCC_FLAGS) -MM $< | sed 's!\(\w*\.o:\)!o/\1!g' > o/$*.d

o/.dirs:
	mkdir -p o/cmd o/fw/hooks
	touch $@

clean:
	@rm -rf o $(TARGET_BIN) $(TARGET_STATIC) presets/*.db
	@echo "Cleaning complete"

presets: $(patsubst %.par, %.db, $(wildcard presets/*.par))

presets/%.db: presets/%.par $(TARGET_BIN)
	@echo "Creating preset $@"
	@rm -f $@
	@/bin/bash -c "./$(TARGET_BIN) $@ < $< &>/dev/null"

