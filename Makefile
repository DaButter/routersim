CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -g
INC = -Isrc -Isrc/utils -Isrc/network_layer -Isrc/transport_layer

OBJDIR = obj
SRC = $(wildcard src/*.cpp) $(wildcard src/utils/*.cpp) $(wildcard src/network_layer/*.cpp) $(wildcard src/transport_layer/*.cpp)
OBJ = $(SRC:src/%.cpp=$(OBJDIR)/%.o)

OUT = router_sim

OBJDIRS = $(OBJDIR) $(OBJDIR)/utils $(OBJDIR)/network_layer $(OBJDIR)/transport_layer

.PHONY: all debug release clean help

all: CXXFLAGS += -O1
all: $(OUT)

debug: CXXFLAGS += -O0 -DDEBUG_BUILD
debug: $(OUT)

release: CXXFLAGS += -O3 -DNDEBUG
release: $(OUT)

$(OBJDIRS):
	mkdir -p $@

$(OBJDIR)/%.o: src/%.cpp
	@echo "Compiling $<"
	$(CXX) $(CXXFLAGS) $(INC) -c $< -o $@

$(OUT): $(OBJDIRS) $(OBJ)
	$(CXX) $(OBJ) -o $@
	@echo "Build complete: $@"

clean:
	rm -rf $(OBJDIR) $(OUT)
	@echo "Clean complete"

help:
	@echo "Makefile targets:"
	@echo "  all      - Build the project with default settings (-O1)"
	@echo "  debug    - Build the project with debug settings (-O0, -DDEBUG_BUILD)"
	@echo "  release  - Build the project with optimizations (-O3, -DNDEBUG)"
	@echo "  clean    - Remove object files and executable"
	@echo "  help     - Show this help message"