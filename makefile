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

DEBUG ?= 1
ifeq ($(DEBUG), 1)
CXXFLAGS += -fsanitize=address,undefined,leak
BUILD_DIR := build/debug
else
CXXFLAGS += -fomit-frame-pointer
CXXFLAGS += -g0
CPPFLAGS += -DNDEBUG
BUILD_DIR := build/release
endif

BIN_DIR := $(BUILD_DIR)/bin
OBJ_DIR := $(BUILD_DIR)/obj
DEP_DIR := $(BUILD_DIR)/depend

HEADERS := $(wildcard *.h)
TEST_SOURCE := $(wildcard *_test.cpp)
LIB_SOURCE := $(filter-out $(TEST_SOURCE),$(wildcard *.cpp))

TEST_OBJECTS := $(TEST_SOURCE:%.cpp=$(OBJ_DIR)/%.o)
LIB_OBJECTS := $(LIB_SOURCE:%.cpp=$(OBJ_DIR)/%.o)
OBJECTS := $(TEST_OBJECTS) $(LIB_OBJECTS)
EXECUTABLES := $(TEST_SOURCE:%.cpp=$(BIN_DIR)/%.exe)

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

$(EXECUTABLES): $(BIN_DIR)/%.exe: $(OBJ_DIR)/%.o $(LIB_OBJECTS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $^ $(LDFLAGS) -o $@

$(OBJECTS): $(OBJ_DIR)/%.o: %.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MMD -MF $(DEP_DIR)/$*.d -c $< -o $@
