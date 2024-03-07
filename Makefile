CXX = em++
WEB_DIR = web/assets
IMGUI_DIR = app/imgui
SRC_DIR = app/src
SRCS = $(SRC_DIR)/main.cpp
SRCS += $(wildcard $(IMGUI_DIR)/*.cpp)
SRCS += $(IMGUI_DIR)/backends/imgui_impl_sdl2.cpp $(IMGUI_DIR)/backends/imgui_impl_opengl3.cpp
OBJS = $(addsuffix .o, $(basename $(notdir $(SRCS))))
CPP_FLAGS = -I$(IMGUI_DIR) -I$(IMGUI_DIR)/backends
EMS_FLAGS = --use-port=sdl2
LD_FLAGS = --use-preload-plugins --preload-file app/assets/smile.png $(EMS_FLAGS) # FIXME: remove assets

%.o:$(SRC_DIR)/%.cpp
	$(CXX) $(CPP_FLAGS) -c -o $@ $<

%.o:$(IMGUI_DIR)/%.cpp
	$(CXX) $(CPP_FLAGS) $(EMS_FLAGS) -c -o $@ $<

%.o:$(IMGUI_DIR)/backends/%.cpp
	$(CXX) $(CPP_FLAGS) $(EMS_FLAGS) -c -o $@ $<

main: $(OBJS) $(WEB_DIR)
	$(CXX) -o $(WEB_DIR)/out.js $(OBJS) $(LD_FLAGS)

emhtml: $(OBJS) $(WEB_DIR)
	$(CXX) -o test/index.html $(OBJS) $(LD_FLAGS)

old-main: 
	em++ app/src/main.cxx -I$(IMGUI_DIR) -I$(IMGUI_DIR)/backends -o web/assets/out.js

old-emhtml:
	em++ app/src/main.cxx -I$(IMGUI_DIR) -I$(IMGUI_DIR)/backends -o test/index.html 

clean:
	rm -f web/assets/out* test/index* *.o