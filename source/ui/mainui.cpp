#include "boxedwine.h"
#include "boxedwineui.h"

#include <SDL.h>
#include <stdio.h>

#include "examples/imgui_impl_sdl.h"
#include "examples/imgui_impl_opengl3.h"

static std::vector<ListViewItem> appListViewItems;
static std::vector<AppButton> appButtons;
static ImFont* appBarFont;
static ImFont* defaultFont;
static bool extendedAppBarFont;
static int currentView;

// should correspond to index in appButtons
#define VIEW_APPS 0
#define VIEW_CONTAINERS 2

class RunOnMain {
public:
    RunOnMain(std::function<bool()> f, U64 timeToRun) : f(f), timeToRun(timeToRun) {}

    std::function<bool()> f;
    U64 timeToRun;
};

std::list<RunOnMain> runOnMainFunctions;

void runOnMainUI(std::function<bool()> f, U64 delayInMillies) {
    runOnMainFunctions.push_back(RunOnMain(f, Platform::getMicroCounter() + delayInMillies*1000));
}

void uiDraw() {    
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 0;
    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, io.DisplaySize.x));
    ImGui::Begin("mainWindow", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar);
    drawAppBar(appButtons, currentView, appBarFont, extendedAppBarFont);
    ImGui::Separator();
    if (currentView==VIEW_CONTAINERS) {
        drawContainersView(ImGui::GetWindowContentRegionMax());
    } else {
        drawListView("Apps", appListViewItems, ImGui::GetWindowContentRegionMax());
    }

    std::list<RunOnMain>::iterator iter = runOnMainFunctions.begin();
    std::list<RunOnMain>::iterator end = runOnMainFunctions.end();

    while (iter != end) {
        RunOnMain& item = *iter;
        if (item.timeToRun <= Platform::getMicroCounter()) {
            if (!item.f()) {
                iter = runOnMainFunctions.erase(iter);
            } else {
                ++iter;
            }
        } else {
            ++iter;
        }
    }
    BaseDlg::runDialogs();

    ImGui::End();
}

void loadFonts(const std::string& basePath) {
    std::string fontPath = basePath + "LiberationMono-Regular.ttf";
    if (Fs::doesNativePathExist(fontPath)) {
        ImGuiIO& io = ImGui::GetIO();
        defaultFont = io.Fonts->AddFontFromFileTTF(fontPath.c_str(), 15.0f);
        fontPath = basePath + "LiberationSans-Bold.ttf";
        if (Fs::doesNativePathExist(fontPath)) {
        appBarFont = io.Fonts->AddFontFromFileTTF(fontPath.c_str(), 24.0f);    
            if (appBarFont) {
                static const ImWchar icons_ranges[] = { 0xe000, 0xe0fe, 0 }; // Will not be copied by AddFont* so keep in scope.
                ImFontConfig config;
                config.MergeMode = true;
                fontPath = basePath + "OpenFontIcons.ttf";
                if (Fs::doesNativePathExist(fontPath)) {
                    io.Fonts->AddFontFromFileTTF(fontPath.c_str(), 24.0f, &config, icons_ranges);
                    extendedAppBarFont = io.Fonts->Build();
                }
            }
        }
    }
}

void createButton() {
    appButtons.clear();
    appButtons.push_back(AppButton(getTranslation(MAIN_BUTTON_APPS), [](){
        currentView = VIEW_APPS;
    }));
    appButtons.push_back(AppButton(getTranslation(MAIN_BUTTON_INSTALL), [](){
        new InstallDlg();
    }));    
    appButtons.push_back(AppButton(getTranslation(MAIN_BUTTON_CONTAINERS), [](){
        BoxedwineData::updateCachedContainerSizes();
        currentView = VIEW_CONTAINERS;
    }));
    appButtons.push_back(AppButton(getTranslation(MAIN_BUTTON_SETTINGS), [](){
        new SettingsDlg();
    }));
    appButtons.push_back(AppButton(getTranslation(MAIN_BUTTON_HELP), [](){
    }));
}

void loadApps() {    
    appListViewItems.clear();
    for (auto& container : BoxedwineData::getContainers()) {
        for (auto& app : container->getApps()) {
            appListViewItems.push_back(ListViewItem(app->getName(), app->getIconTexture(), [app](bool right) {
                if (right) {
                    runOnMainUI([]() {
                        ImGui::OpenPopup("AppOptionsPopup");
                        return false;
                        });

                    runOnMainUI([app]() {                            
                        if (!ImGui::BeginPopup("AppOptionsPopup")) {
                            return false;
                        } else {
                            if (ImGui::Selectable("Options")) {
                                ImGui::EndPopup();
                                new AppOptionsDlg(app);
                                return true;
                            }
                            ImGui::EndPopup();
                            return true;
                        }
                    });
                } else {
                    runOnMainUI([app]() {
                        new WaitDlg(WAITDLG_LAUNCH_APP_TITLE, getTranslationWithFormat(WAITDLG_LAUNCH_APP_LABEL, true, app->getName()));
                        app->launch();
                        return false;
                    });
                }
            }));
        }
    }
    std::sort(appListViewItems.begin(), appListViewItems.end(), [](ListViewItem& a, ListViewItem& b) {
        return stringIsLessCaseInsensative(a.text, b.text);
        });
}

static SDL_Window* window;
static SDL_GLContext gl_context;
static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

void uiShutdown() {
    BaseDlg::stopAllDialogs();

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    window = NULL;    
}

bool uiIsRunning() {
    return window!=NULL;
}

bool uiLoop() {
    // Poll and handle events (inputs, window resize, etc.)
    // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
    // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
    // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
    // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
    SDL_Event event;
    bool done = false;
    while (SDL_PollEvent(&event))
    {
        ImGui_ImplSDL2_ProcessEvent(&event);
        if (event.type == SDL_QUIT) {
            done = true;
        } else if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window)) {
            done = true;
        }  else if (event.type >= SDL_USEREVENT) {
            SDL_PushEvent(&event); // if we are currently launching then there is another spot where sdl polling is happening in threadedMainloop.cpp, if we drop this custom msg then another thread will be blocked forever
            break;
        } else if (event.type == SDL_DROPFILE) {
            char* droppedFileOrDir = event.drop.file;
            static std::string staticFilePath;
            
            staticFilePath = droppedFileOrDir;
            runOnMainUI([]() {
                new InstallDlg(staticFilePath);
                return false;
                });
            SDL_free(droppedFileOrDir);    // Free dropped_filedir memory
            break;
        }

    }

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame(window);
    ImGui::NewFrame();

    uiDraw();

    // Rendering
    ImGui::Render();
    ImGuiIO& io = ImGui::GetIO();
    glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
    glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    SDL_GL_SwapWindow(window);
    return done;
}

bool uiShow(const std::string& basePath) {        
    // Setup SDL
    // (Some versions of SDL before <2.0.10 appears to have performance/stalling issues on a minority of Windows systems,
    // depending on whether SDL_INIT_GAMECONTROLLER is enabled or disabled.. updating to latest version of SDL is recommended!)
    /*
    // done in main
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0)
    {
        printf("Error: %s\n", SDL_GetError());
        return false;
    }
    */

    // Decide GL+GLSL versions
#if __APPLE__
    // GL 3.2 Core + GLSL 150
    const char* glsl_version = "#version 150";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif

    // Create window with graphics context
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    window = SDL_CreateWindow("Boxedwine UI", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1024, 768, window_flags);
    gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1); // Enable vsync

    // Initialize OpenGL loader
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
    bool err = gl3wInit() != 0;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
    bool err = glewInit() != GLEW_OK;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
    bool err = gladLoadGL() == 0;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING)
    bool err = false;
    glbinding::initialize([](const char* name) { return (glbinding::ProcAddress)SDL_GL_GetProcAddress(name); });
#else
    bool err = false; // If you use IMGUI_IMPL_OPENGL_LOADER_CUSTOM, your loader is likely to requires some form of initialization.
#endif
    if (err)
    {
        fprintf(stderr, "Failed to initialize OpenGL loader!\n");
        return 1;
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();    
    loadFonts(basePath);
    BoxedwineData::loadUI();
    currentView = VIEW_APPS;
    loadApps(); // need to be after we create the context for images to work
    createButton();
    if (GlobalSettings::startUpArgs.showAppPickerForContainer.length()) {
        runOnMainUI([]() {                    
            BoxedContainer* container = BoxedwineData::getContainerByName(GlobalSettings::startUpArgs.showAppPickerForContainer);
            if (container) {
                new AppChooserDlg(container);
            }
            GlobalSettings::startUpArgs.showAppPickerForContainer = "";
            return false;
        });

    }
    //ImGuiIO& io = ImGui::GetIO();
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    GlobalSettings::loadTheme();

    // Setup Platform/Renderer bindings
    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);    

    // Main loop
    bool done = false;
    while (!done && !GlobalSettings::startUpArgs.readyToLaunch)
    {
        done = uiLoop();
    }
    if (done) {
        uiShutdown();
    }
    return GlobalSettings::startUpArgs.readyToLaunch;
}
