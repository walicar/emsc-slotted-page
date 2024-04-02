CXX = em++
CXX_TEST = g++
WEB_DIR = web/assets
IMGUI_DIR = app/imgui
SRC_DIR = app/src
SRCS = $(wildcard $(SRC_DIR)/*.cpp)
SRCS += $(wildcard $(IMGUI_DIR)/*.cpp)
SRCS += $(IMGUI_DIR)/backends/imgui_impl_sdl2.cpp $(IMGUI_DIR)/backends/imgui_impl_opengl3.cpp
OBJS = $(addsuffix .o, $(basename $(notdir $(SRCS))))
CPP_FLAGS = -I$(IMGUI_DIR) -I$(IMGUI_DIR)/backends -I/opt/homebrew/Cellar/sdl2/2.30.1/include
EMS_FLAGS = --use-port=sdl2 -sALLOW_MEMORY_GROWTH
LD_FLAGS = --use-preload-plugins --preload-file app/assets/smile.png $(EMS_FLAGS) # FIXME: remove assets

TEST_DIR = tests/
TEST_SRCS = $(filter-out $(SRC_DIR)/main.cpp, $(wildcard $(SRC_DIR)/*.cpp)) $(wildcard $(TEST_DIR)/*.cpp)
TEST_OBJS = $(addsuffix .test.o, $(basename $(notdir $(TEST_SRCS))))
TEST_FLAGS = -std=c++20 -I$(SRC_DIR) -I/opt/homebrew/Cellar/googletest/1.14.0/include -g
TEST_LD_FLAGS = -L/opt/homebrew/Cellar/googletest/1.14.0/lib -lgtest -lgtest_main -pthread

%.o:$(SRC_DIR)/%.cpp
	$(CXX) $(CPP_FLAGS) -c -o $@ $<

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
	rm -f web/assets/out* tests/index* *.o *.test.o tester
