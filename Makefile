CXX = em++
CXX_TEST = g++

WEB_DIR = web/assets
IMGUI_DIR = app/imgui
SRC_DIR = app/src
CEL_DIR = $(shell brew --cellar)

SRCS = $(wildcard $(SRC_DIR)/*.cpp)
SRCS += $(wildcard $(IMGUI_DIR)/*.cpp)
SRCS += $(IMGUI_DIR)/backends/imgui_impl_sdl2.cpp $(IMGUI_DIR)/backends/imgui_impl_opengl3.cpp
OBJS = $(addsuffix .o, $(basename $(notdir $(SRCS))))

CPP_FLAGS = -I$(IMGUI_DIR) -I$(IMGUI_DIR)/backends
EMS_FLAGS = --use-port=sdl2
LD_FLAGS =  --use-preload-plugins --preload-file app/assets/smile.png $(EMS_FLAGS) -sALLOW_MEMORY_GROWTH # FIXME: remove assets

GTEST_VER := $(shell brew list --versions googletest | awk '{print $$2}')
GTEST_DIR := $(strip $(CEL_DIR)/googletest/$(GTEST_VER))

TEST_DIR = tests
TEST_SRCS = $(filter-out $(SRC_DIR)/main.cpp, $(wildcard $(SRC_DIR)/*.cpp)) $(wildcard $(TEST_DIR)/*.cpp)
TEST_OBJS = $(addsuffix .test.o, $(basename $(notdir $(TEST_SRCS))))
TEST_FLAGS = -std=c++20 -I$(SRC_DIR) -I$(GTEST_DIR)/include -g
TEST_LD_FLAGS = -L$(GTEST_DIR)/lib -lgtest -lgtest_main -pthread

%.o:$(SRC_DIR)/%.cpp
	$(CXX) $(CPP_FLAGS) $(EMS_FLAGS) -c -o $@ $<

%.o:$(IMGUI_DIR)/%.cpp
	$(CXX) $(CPP_FLAGS) $(EMS_FLAGS) -c -o $@ $<

%.o:$(IMGUI_DIR)/backends/%.cpp
	$(CXX) $(CPP_FLAGS) $(EMS_FLAGS) -c -o $@ $<

%.test.o: $(TEST_DIR)/%.cpp
	$(CXX_TEST) $(TEST_FLAGS) -c -o $@ $<

%.test.o: $(SRC_DIR)/%.cpp
	$(CXX_TEST) $(TEST_FLAGS) -c -o $@ $<

main: $(OBJS) $(WEB_DIR)
	$(CXX) -o $(WEB_DIR)/out.js $(OBJS) $(LD_FLAGS)

emhtml: $(OBJS) $(WEB_DIR)
	$(CXX) -o tests/index.html $(OBJS) $(LD_FLAGS)

test: $(TEST_OBJS)
	$(CXX_TEST) -o test $(TEST_OBJS) $(TEST_LD_FLAGS)

clean:
	rm -f web/assets/out* tests/index* *.o *.test.o test
