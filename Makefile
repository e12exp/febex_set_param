LIBS=-lreadline

GCC=gcc
GCC_FLAGS=-g -Wall

TARGET_BIN=setpar

OBJ_BIN=o/setpar.o o/filestruct.o o/readfile.o o/paramdef.o o/data.o o/consoleinterface.o o/command.o

.PHONY: all clean

all: $(TARGET_BIN)

-include $(OBJ_BIN:.o=.d)

$(TARGET_BIN): $(OBJ_BIN)
	@echo "[LD] $@"
	@$(GCC) $(GCC_FLAGS) $+ $(LIBGOSIP_A) $(LIBS) -o $@

o/%.o: src/%.c
	@echo "[CC] $@"
	@mkdir -p o
	@$(GCC) $(GCC_FLAGS) -c $< -o $@
	@$(GCC) $(GCC_FLAGS) -MM $< | sed 's!\(\w*\.o:\)!o/\1!g' > o/$*.d

clean:
	@rm -rf o $(TARGET_BIN) $(TARGET_STATIC)
	@echo "Cleaning complete"

