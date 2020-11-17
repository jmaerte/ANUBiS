//
// Created by jmaerte on 03.10.20.
//

#include "ANUBiS/probabilistic/multi-parametric.hpp"
#include "data_types/potence/potence.hpp"

// SDL
//#include <glad/glad.h>
//#include <SDL.h>

// ImGUI
//#include "lib/imgui/imgui.h"
//#include "lib/imgui/imgui_impl_sdl.h"
//#include "lib/imgui/imgui_impl_opengl3.h"

#include <numeric>
#include "../data_types/potence/potence.hpp"

int windowWidth = 1280,
        windowHeight = 720;


namespace jmaerte {
    namespace anubis {
        namespace probabilistic {

            void proceed(mp_experiment* exp, mp_experiment::node * node, potence<int> * pot, complex* cmplx) {
                while (!pot->done()) {
                    int * simplex = new int[pot->order()];
                    for (int i = 0; i < pot->order(); i++) simplex[i] = pot->get(i);

                    if (cmplx->is_external(simplex)) {
                        mp_experiment::node * next = new mp_experiment::node;
                        node->children.push_back(next);
//                        next->m_complex = cmplx->im_insert(simplex);
                        auto c = cmplx->im_insert(simplex);
                        next->parent = node;
                        next->prob = exp->p(pot->order() - 1) * node->prob;
                        next->is_P = exp->P(c);

                        potence<int>* pt = pot->copy();
                        delete simplex;
                        proceed(exp, next, pt, c);
                    } else {
                        node->prob *= exp->p(pot->order() - 1);
                        pot->operator++();
                        delete simplex;
                    }
                }
            }

            double mp_experiment::generate_poset() {
                if (!root) {
                    return 0;
                }

                root = new mp_experiment::node;
//                root->m_complex = s_list<true>::from_facets({}, "complex", -1);
                auto c = s_list<true>::from_facets({}, "complex", -1);
                root->prob = 1.0;
                root->is_P = P(c);
                root->parent = nullptr;

                std::vector<int> vertices (n);
                std::iota(vertices.begin(), vertices.end(), 1);

                proceed(this, root, new potence<int>(vertices, 1), c);
            }


            void mp_experiment::render_poset() {
//                // initiate SDL
//                if (SDL_Init(SDL_INIT_VIDEO) != 0)
//                {
//                    printf("[ERROR] %s\n", SDL_GetError());
//                    return -1;
//                }
//
//                // set OpenGL attributes
//                SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
//                SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
//                SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
//
//                SDL_GL_SetAttribute(
//                        SDL_GL_CONTEXT_PROFILE_MASK,
//                        SDL_GL_CONTEXT_PROFILE_CORE
//                );
//
//                std::string glsl_version = "";
//#ifdef __APPLE__
//                // GL 3.2 Core + GLSL 150
//    glsl_version = "#version 150";
//    SDL_GL_SetAttribute( // required on Mac OS
//        SDL_GL_CONTEXT_FLAGS,
//        SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG
//        );
//    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
//    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
//#elif __linux__
//                // GL 3.2 Core + GLSL 150
//    glsl_version = "#version 150";
//    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
//    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
//    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
//#elif _WIN32
//                // GL 3.0 + GLSL 130
//    glsl_version = "#version 130";
//    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
//    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
//    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
//#endif
//
//                SDL_WindowFlags window_flags = (SDL_WindowFlags)(
//                        SDL_WINDOW_OPENGL
//                        | SDL_WINDOW_RESIZABLE
//                        | SDL_WINDOW_ALLOW_HIGHDPI
//                );
//                SDL_Window *window = SDL_CreateWindow(
//                        "Dear ImGui SDL",
//                        SDL_WINDOWPOS_CENTERED,
//                        SDL_WINDOWPOS_CENTERED,
//                        windowWidth,
//                        windowHeight,
//                        window_flags
//                );
//                // limit to which minimum size user can resize the window
//                SDL_SetWindowMinimumSize(window, 500, 300);
//
//                SDL_GLContext gl_context = SDL_GL_CreateContext(window);
//                SDL_GL_MakeCurrent(window, gl_context);
//
//                // enable VSync
//                SDL_GL_SetSwapInterval(1);
//
//                if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
//                {
//                    std::cerr << "[ERROR] Couldn't initialize glad" << std::endl;
//                }
//                else
//                {
//                    std::cout << "[INFO] glad initialized\n";
//                }
//
//                glViewport(0, 0, windowWidth, windowHeight);
//
//                // setup Dear ImGui context
//                IMGUI_CHECKVERSION();
//                ImGui::CreateContext();
//                ImGuiIO& io = ImGui::GetIO(); (void)io;
//
//
//
//                // setup Dear ImGui style
//                ImGui::StyleColorsDark();
//                ImGuiStyle &style = ImGui::GetStyle();
//                style.Colors[ImGuiCol_Text]          = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
//
//
//
//                // setup platform/renderer bindings
//                ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
//                ImGui_ImplOpenGL3_Init(glsl_version.c_str());
//
//                // colors are set in RGBA, but as float
//                ImVec4 background = ImVec4(35/255.0f, 35/255.0f, 35/255.0f, 1.00f);
//
//                glClearColor(background.x, background.y, background.z, background.w);
//
//                // TODO: INIT
//
//                bool loop = true;
//                while (loop)
//                {
//                    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
//
//                    SDL_Event event;
//                    while (SDL_PollEvent(&event))
//                    {
//                        // without it you won't have keyboard input and other things
//                        ImGui_ImplSDL2_ProcessEvent(&event);
//                        // you might also want to check io.WantCaptureMouse and io.WantCaptureKeyboard
//                        // before processing events
//
//                        switch (event.type)
//                        {
//                            case SDL_QUIT:
//                                loop = false;
//                                break;
//
//                            case SDL_WINDOWEVENT:
//                                switch (event.window.event)
//                                {
//                                    case SDL_WINDOWEVENT_RESIZED:
//                                        windowWidth = event.window.data1;
//                                        windowHeight = event.window.data2;
//                                        // std::cout << "[INFO] Window size: "
//                                        //           << windowWidth
//                                        //           << "x"
//                                        //           << windowHeight
//                                        //           << std::endl;
//                                        glViewport(0, 0, windowWidth, windowHeight);
//                                        break;
//                                }
//                                break;
//
//                            case SDL_KEYDOWN:
//                                switch (event.key.keysym.sym)
//                                {
//                                    case SDLK_ESCAPE:
//                                        loop = false;
//                                        break;
//                                }
//                                break;
//                        }
//                    }
//
//                    // TODO: Render using "window" variable
//                    ImGui_ImplOpenGL3_NewFrame();
//                    ImGui_ImplSDL2_NewFrame(window);
//                    ImGui::NewFrame();
//
//                    // RENDER
//
//
//                    ImGui::Render();
//                    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
//                    ImGui::ResetMouseDragDelta();
//
//                    SDL_GL_SwapWindow(window);
//                }
//
//                ImGui_ImplOpenGL3_Shutdown();
//                ImGui_ImplSDL2_Shutdown();
//                ImGui::DestroyContext();
//
//                SDL_GL_DeleteContext(gl_context);
//                SDL_DestroyWindow(window);
//                SDL_Quit();

            }
        }
    }
}