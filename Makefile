CC        = g++
OBJ       = build
SRCS     += $(wildcard src/*.cpp)
DEPDIR   := $(OBJ)/deps
DEPFILES := $(patsubst src/%.cpp,$(DEPDIR)/%.d,$(SRCS))
OBJS      = $(patsubst src/%.cpp,$(OBJ)/%.o,$(SRCS))
DEPFLAGS  = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.d
EXE       = nothing
CFLAGS    = -g -w -std=c++17
LDLIBS    = -lm -lstdc++


all: $(EXE)

$(DEPFILES):
	@mkdir -p "$(@D)"

$(EXE): $(OBJS) | $(BIN)
	@echo "($(MODE)) Building final executable $@"
	@$(CC) $(CFLAGS) $^ -o $@ $(LDLIBS)

$(OBJ)/%.o : src/%.cpp $(DEPDIR)/%.d | $(DEPDIR)
	@echo "($(MODE)) Compiling $@"
	@mkdir -p "$(@D)"
	@$(CC) $(DEPFLAGS) $(CFLAGS) -c $< -o $@ 

$(OBJ):
	@mkdir -p $@

$(DEPDIR): 
	@mkdir -p $@

clean:
	rm -rf build $(EXE)

include $(wildcard $(DEPFILES))
