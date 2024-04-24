#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_video.h>
#include <stdbool.h>
#include <emscripten.h>
#include <stdio.h>
#include <stdlib.h>
#include "slotted_page.h"

// State
SDL_GLContext gl_context = NULL;
SDL_Window *window = NULL;
static bool done = false;
ImVec4 clear_color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
ImGuiIO *io = NULL;

SlottedPage *g_page = NULL;

bool init()
{
    bool success = true;
    // Setup SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0)
    {
        printf("Error: %s\n", SDL_GetError());
        return false;
    }

    // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100
    const char *glsl_version = "#version 100";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#elif defined(__APPLE__)
    // GL 3.2 Core + GLSL 150
    const char *glsl_version = "#version 150";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#else
    // GL 3.0 + GLSL 130
    const char *glsl_version = "#version 130";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif

    // From 2.0.18: Enable native IME.
#ifdef SDL_HINT_IME_SHOW_UI
    SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif

    // Create window with graphics context
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    window = SDL_CreateWindow("Slotted Page", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
    if (window == nullptr)
    {
        printf("Error: SDL_CreateWindow(): %s\n", SDL_GetError());
        return false;
    }

    gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1); // Enable vsync
    if (gl_context == nullptr)
    {
        printf("Error: SDL_GL_CreateContext(): %s\n", SDL_GetError());
        return false;
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    io = &ImGui::GetIO();
    (void)io;
    io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    if (!ImGui_ImplSDL2_InitForOpenGL(window, gl_context))
    {
        printf("sdl2 imgui init open gl BAD\n");
        return false;
    }
    if (!ImGui_ImplOpenGL3_Init(glsl_version))
    {
        printf("opengl3 imgui init BAD\n");
        return false;
    }

    // Initialize Page
    g_page = new SlottedPage();
    return success;
}

void close()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

DAS *marshal_text(std::string text)
{
    uint size = text.length();
    uint offset = 0;

    char *bytes = new char[BLOCK_SZ];
    *(u_int16_t *)(bytes + offset) = size;
    offset += sizeof(u_int16_t);
    memcpy(bytes + offset, text.c_str(), size);
    offset += size;

    char *right_size_bytes = new char[offset];
    memcpy(right_size_bytes, bytes, offset);
    delete[] bytes;
    return new DAS(right_size_bytes, offset);
}

std::string unmarshal_text(DAS &data)
{
    char *bytes = (char *)data.get_data();
    u_int16_t size;
    memcpy(&size, bytes, sizeof(u_int16_t));
    std::string text(bytes + sizeof(u_int16_t), size);
    return text;
}

static void mainloop(void)
{
    if (done)
    {
        // clean up
        close();
        emscripten_cancel_main_loop();
    }
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        ImGui_ImplSDL2_ProcessEvent(&event);
        if (event.type == SDL_QUIT)
            done = true;
        if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
            done = true;
    }

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();


    {
        static float f = 0.0f;
        ImGui::SetNextWindowPos(ImVec2(50,10), ImGuiCond_FirstUseEver);
        ImGui::Begin("Slotted Page I/O"); // Create a window called "Hello, world!" and append into it.

        ImGui::SeparatorText("DATA ENTRY");
        ImGui::Text("Record Number: %d", g_page->size());
        static char put_key[128] = "";
        static char put_val[128] = "";
        ImGui::InputText("Key##put", put_key, IM_ARRAYSIZE(put_key));
        ImGui::InputText("Value##put", put_val, IM_ARRAYSIZE(put_val));
        if (ImGui::Button("Put Record")) // Buttons return true when clicked (most widgets return true when edited/activated)
            {
                DAS *key = marshal_text(put_key);
		        DAS *val = marshal_text(put_val);
                g_page->put(key, val);
                // do we delete?
            }
        ImGui::SeparatorText("DATA RETRIEVAL");
        static char get_key[128] = "";
        static char *get_value = nullptr;
        ImGui::InputText("Key##get", get_key, IM_ARRAYSIZE(get_key));
        ImGui::Text("%s", get_value);

        if (ImGui::Button("Get Record"))
            {
                DAS *key = marshal_text(get_key);
                DAS *data = g_page->get(key);
                if (data->get_data()) {
		            auto get_str = new std::string(unmarshal_text(*data));
                    get_value = &(*get_str)[0];
                } else {
                    get_value = nullptr;
                }
                delete key;
            }

        ImGui::SeparatorText("DATA MANAGEMENT");
        static char del_key[128] = "nothing";
        ImGui::InputText("Key##del", del_key, IM_ARRAYSIZE(del_key));
        ImGui::Text("%s marked for deletion", del_key);

        if (ImGui::Button("Delete Record"))
            {
                DAS *key = marshal_text(del_key);
                g_page->del(key);
                delete key;
            }


        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io->Framerate, io->Framerate);
        ImGui::End();
    }

    {
        ImGui::SetNextWindowPos(ImVec2(400,10), ImGuiCond_FirstUseEver);

        ImGui::Begin("Slotted Page Hex View");
        static ImGuiTableFlags flags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_Hideable;
        auto bytes = g_page->show();
        if (ImGui::BeginTable("table1", 8, flags))
        {
            for (int row = 0; row < 512; row++)
            {
                ImGui::TableNextRow();
                for (int column = 0; column < 8; column++)
                {
                    ImGui::TableSetColumnIndex(column);
                    ImGui::Text("%s", bytes[row][column]->c_str());
                }
            }
            ImGui::EndTable();
        }
        ImGui::End();
    }

    // Rendering
    ImGui::Render();
    glViewport(0, 0, (int)io->DisplaySize.x, (int)io->DisplaySize.y);
    glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    SDL_GL_SwapWindow(window);
}

int main(int argc, char *argv[])
{
    if (!init())
    {
        printf("Failed to initialize!\n");
        return -1;
    }
    emscripten_set_main_loop(mainloop, 0, true);
    return 0;
}