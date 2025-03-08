CXX = g++
CXXFLAGS = -std=c++11 -Isrc/cm -Isrc/utils
LDFLAGS = -lssh2
SRC_DIR = src
OBJ_DIR = build
OUT = connection-manager

SRC = $(shell find $(SRC_DIR) -name '*.cpp')

OBJ = $(SRC:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

OBJ_CM_DIR = $(OBJ_DIR)/cm
OBJ_UTILS_DIR = $(OBJ_DIR)/utils
OBJ_CM = $(OBJ:$(OBJ_DIR)/cm/%.o=$(OBJ_CM_DIR)/%.o)
OBJ_UTILS = $(OBJ:$(OBJ_DIR)/utils/%.o=$(OBJ_UTILS_DIR)/%.o)

$(shell mkdir -p $(OBJ_CM_DIR) $(OBJ_UTILS_DIR))

all: $(OUT)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OUT): $(OBJ)
	$(CXX) $(OBJ) $(LDFLAGS) -o $(OUT)

clean:
	rm -rf $(OBJ_DIR) $(OUT)