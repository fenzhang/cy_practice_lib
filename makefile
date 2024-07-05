CXX = clang++
AR = llvm-ar

CXXFLAGS += -std=c++20
CXXFLAGS += -flto
CXXFLAGS += -Wall
CXXFLAGS += -Wextra
CXXFLAGS += -Wpedantic
CXXFLAGS += -Werror
CXXFLAGS += -O2
LDFLAGS += -Wl,--fatal-warnings

ifeq ($(CXX), $(filter $(CXX),clang++))
CXXFLAGS += -ferror-limit=8
CXXFLAGS += -Wconversion
LDFLAGS += -fuse-ld=lld
LDFLAGS += -Wl,--icf=all
else ifeq ($(CXX), $(filter $(CXX),g++))
CXXFLAGS += -fmax-errors=8
CXXFLAGS += -Wuseless-cast
endif

ifeq ($(DEBUG), 1)
CXXFLAGS += -fsanitize=address,undefined,leak
else
CXXFLAGS += -fomit-frame-pointer
CXXFLAGS += -g0
CPPFLAGS += -DNDEBUG
endif

BUILD_DIR := build
BIN_DIR := $(BUILD_DIR)/bin
OBJ_DIR := $(BUILD_DIR)/obj
DEP_DIR := $(BUILD_DIR)/depend

HEADERS := $(wildcard *.h)
SOURCE := $(wildcard *.cpp)
MAIN_TARGETS := thread_pool_test

OBJECTS := $(SOURCE:%.cpp=$(OBJ_DIR)/%.o)
EXECUTABLES := $(MAIN_TARGETS:%=$(BIN_DIR)/%.exe)

DEPS := $(OBJECTS:$(OBJ_DIR)/%.o=$(DEP_DIR)/%.d)

.PHONY: all clean $(DEPS)
all: $(BIN_DIR) $(OBJ_DIR) $(DEP_DIR) $(EXECUTABLES)
-include $(DEPS)

clean:
	rm -f $(BIN_DIR)/* $(OBJ_DIR)/* $(DEP_DIR)/*

$(BIN_DIR):
	mkdir -p $@

$(OBJ_DIR):
	mkdir -p $@

$(DEP_DIR):
	mkdir -p $@

$(EXECUTABLES): $(BIN_DIR)/%.exe: $(OBJECTS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $^ $(LDFLAGS) -o $@

$(OBJECTS): $(OBJ_DIR)/%.o: %.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MMD -MF $(DEP_DIR)/$*.d -c $< -o $@
