#include "boxedwine.h"
#include "boxedwineui.h"

#include <SDL.h>
#include <stdio.h>

#include "knativesystem.h"

#include "examples/imgui_impl_sdl.h"
#ifdef BOXEDWINE_IMGUI_DX9
#include "examples/imgui_impl_dx9.h"
#include <SDL_syswm.h>
#include <d3d9.h>
static LPDIRECT3D9              g_pD3D = nullptr;
LPDIRECT3DDEVICE9        g_pd3dDevice = nullptr;
static D3DPRESENT_PARAMETERS    g_d3dpp = {};

bool CreateDeviceD3D(HWND hWnd)
{
    if ((g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)) == nullptr)
        return false;

    // Create the D3DDevice
    ZeroMemory(&g_d3dpp, sizeof(g_d3dpp));
    g_d3dpp.Windowed = TRUE;
    g_d3dpp.hDeviceWindow = hWnd;
    g_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    g_d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
    g_d3dpp.EnableAutoDepthStencil = TRUE;
    g_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
    g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;           // Present with vsync
    //g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;   // Present without vsync, maximum unthrottled framerate
    if (g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &g_d3dpp, &g_pd3dDevice) < 0)
        return false;

    return true;
}

void CleanupDeviceD3D()
{
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
    if (g_pD3D) { g_pD3D->Release(); g_pD3D = nullptr; }
}

void ResetDevice()
{
    BoxedTexture::resetAll();
    ImGui_ImplDX9_InvalidateDeviceObjects();
    HRESULT hr = g_pd3dDevice->Reset(&g_d3dpp);
    if (hr == D3DERR_INVALIDCALL)
        IM_ASSERT(0);
    ImGui_ImplDX9_CreateDeviceObjects();
}

#endif

#ifdef BOXEDWINE_OPENGL_SDL
#ifdef BOXEDWINE_OPENGL_IMGUI_V2
#include "examples/imgui_impl_opengl2.h"
#else
#include "examples/imgui_impl_opengl3.h"
#endif
#endif
#include "imgui_internal.h"

static std::vector<ListViewItem> appListViewItems;
static std::vector<AppButton> appButtons;
static int currentViewDeprecated;
static BaseView* currentView;

class RunOnMain {
public:
    RunOnMain(std::function<bool()> f, U64 timeToRun) : f(f), timeToRun(timeToRun) {}

    std::function<bool()> f;
    U64 timeToRun;
};

std::list<RunOnMain> runOnMainFunctions;
std::list<RunOnMain> runAfterFrameFunctions;

void runOnMainUI(std::function<bool()> f, U64 delayInMillies) {
    runOnMainFunctions.push_back(RunOnMain(f, SDL_GetTicks() + delayInMillies));
}

void runAfterFrame(std::function<bool()> f, U64 delayInMillies) {
    runAfterFrameFunctions.push_back(RunOnMain(f, SDL_GetTicks() + delayInMillies));
}

void runFunctions(std::list<RunOnMain>& functions) {
    std::list<RunOnMain>::iterator iter = functions.begin();
    std::list<RunOnMain>::iterator end = functions.end();

    while (iter != end) {
        RunOnMain& item = *iter;
        if (item.timeToRun <= SDL_GetTicks()) {
            if (!item.f()) {
                iter = functions.erase(iter);
            } else {
                ++iter;
            }
        } else {
            ++iter;
        }
    }
}

void uiDraw() {    
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 0;
    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, io.DisplaySize.y));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImGui::GetColorU32(ImGuiCol_MenuBarBg));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));    
    ImGui::Begin("mainWindow", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar);
    ImGui::PopStyleColor();
    drawAppBar(appButtons, currentViewDeprecated, GlobalSettings::largeFontBold);
    //ImGui::Separator();
    ImVec2 size = ImGui::GetWindowContentRegionMax();
    size.y -= ImGui::GetCursorPosY();
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetColorU32(ImGuiCol_WindowBg));
    if (currentViewDeprecated == VIEW_OPTIONS || currentViewDeprecated == VIEW_INSTALL || currentViewDeprecated == VIEW_CONTAINERS || currentViewDeprecated == VIEW_HELP) {
        currentView->run(size);
    } else {        
        drawListView(B("Apps"), appListViewItems, size);
    }
    ImGui::PopStyleColor();
    
    runFunctions(runOnMainFunctions);

    BaseDlg::runDialogs();

    ImGui::End();
    ImGui::PopStyleVar(1);    
}

void gotoView(int viewId, BString tab, BString param1) {
    if (!currentView || currentView->saveChanges()) {
        if (currentView) {
            delete currentView;
            currentView = nullptr;
        }
        currentViewDeprecated = viewId;
        if (viewId == VIEW_OPTIONS) {
            currentView = new OptionsView(tab);
        } else if (viewId == VIEW_INSTALL) {
            currentView = new InstallView(param1, tab);
        } else if (viewId == VIEW_CONTAINERS) {
            currentView = new ContainersView(tab, param1);
        } else if (viewId == VIEW_HELP) {
            currentView = new HelpView(tab);
        }
    }
}

void createButton() {
    appButtons.clear();
    BString name;
    
    if (GlobalSettings::hasIconsFont()) {
        name += " ";
        name += APP_LIST_ICON;
        name += " ";
    }
    name += getTranslation(Msg::MAIN_BUTTON_APPS);
    appButtons.push_back(AppButton(name, [](){
        gotoView(VIEW_APPS);
    }));
    name = "";
    if (GlobalSettings::hasIconsFont()) {
        name += " ";
        name += INSTALL_ICON;
        name += " ";
    }
    name += getTranslation(Msg::MAIN_BUTTON_INSTALL);
    appButtons.push_back(AppButton(name, [](){
        gotoView(VIEW_INSTALL);
    }));  
    name = "";
    if (GlobalSettings::hasIconsFont()) {
        name += " ";
        name += CONTAINER_ICON;
        name += " ";
    }
    name += getTranslation(Msg::MAIN_BUTTON_CONTAINERS);
    appButtons.push_back(AppButton(name, [](){
        gotoView(VIEW_CONTAINERS);
    }));
    name = "";
    if (GlobalSettings::hasIconsFont()) {
        name += " ";
        name += OPTIONS_ICON;
        name += " ";
    }
    name += getTranslation(Msg::MAIN_BUTTON_SETTINGS);
    appButtons.push_back(AppButton(name, [](){
        gotoView(VIEW_OPTIONS);        
    }));
    name = "";
    if (GlobalSettings::hasIconsFont()) {
        name += " ";
        name += QUESTION_ICON;
        name += " ";
    }
    name += getTranslation(Msg::MAIN_BUTTON_HELP);
    appButtons.push_back(AppButton(name, []() {
        gotoView(VIEW_HELP);
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
                        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(GlobalSettings::scaleFloatUI(8.0f), GlobalSettings::scaleFloatUI(8.0f)));
                        ImGui::PushStyleColor(ImGuiCol_PopupBg, ImGui::GetColorU32(ImGuiCol_ScrollbarGrab) | 0xFF000000);
                        bool result = false;
                        if (ImGui::BeginPopup("AppOptionsPopup")) {
                            if (ImGui::Selectable(c_getTranslation(Msg::MAIN_BUTTON_SETTINGS))) {
                                gotoView(VIEW_CONTAINERS, app->getContainer()->getDir(), app->getIniFilePath());
                            }     
                            if (ImGui::Selectable(c_getTranslation(Msg::CONTAINER_VIEW_DELETE_SHORTCUT))) {
                                BString label = getTranslationWithFormat(Msg::CONTAINER_VIEW_DELETE_SHORTCUT_CONFIRMATION, true, app->getName());
                                runOnMainUI([label, app]() {
                                    new YesNoDlg(Msg::GENERIC_DLG_CONFIRM_TITLE, label, [app](bool yes) {
                                        if (yes) {
                                            runOnMainUI([app]() {
                                                app->remove();
                                                GlobalSettings::reloadApps();
                                                return false;
                                                });
                                        }
                                        });
                                    return false;
                                    });
                            }
#ifdef BOXEDWINE_RECORDER
                            if (GlobalSettings::isAutomationEnabled()) {
                                if (ImGui::Selectable(c_getTranslation(Msg::CONTAINER_VIEW_CREATE_AUTOMATION))) {
                                    runOnMainUI([app]() {
                                        new WaitDlg(Msg::WAITDLG_LAUNCH_APP_TITLE, getTranslationWithFormat(Msg::WAITDLG_LAUNCH_APP_LABEL, true, app->getName()));
                                        app->createAutomation();
                                        return false;
                                        });
                                }
                                if (ImGui::Selectable(c_getTranslation(Msg::CONTAINER_VIEW_RUN_AUTOMATION), false, app->hasAutomation()? 0 : ImGuiSelectableFlags_Disabled)) {
                                    runOnMainUI([app]() {
                                        new WaitDlg(Msg::WAITDLG_LAUNCH_APP_TITLE, getTranslationWithFormat(Msg::WAITDLG_LAUNCH_APP_LABEL, true, app->getName()));
                                        app->runAutomation();
                                        return false;
                                        });
                                }
                            }
#endif
                            ImGui::EndPopup();
                            result = true;
                        }
                        ImGui::PopStyleColor();
                        ImGui::PopStyleVar();
                        return result;
                    });
                } else {
                    runOnMainUI([app]() {
                        if (app->getContainer()->doesFileSystemExist()) {
                            new WaitDlg(Msg::WAITDLG_LAUNCH_APP_TITLE, getTranslationWithFormat(Msg::WAITDLG_LAUNCH_APP_LABEL, true, app->getName()));
                            app->launch();
                        } else {
                            std::shared_ptr<FileSystemZip> missingFileSystem = GlobalSettings::getAvailableFileSystemFromName(app->getContainer()->getFileSystemName());
                            if (missingFileSystem) {
                                new YesNoDlg(Msg::GENERIC_DLG_ERROR_TITLE, getTranslation(Msg::ERROR_MISSING_FILE_SYSTEM), [missingFileSystem, app](bool yes) {
                                    if (yes) {
                                        GlobalSettings::downloadFileSystem(missingFileSystem, [app](bool success) {
                                            if (success) {
                                                new WaitDlg(Msg::WAITDLG_LAUNCH_APP_TITLE, getTranslationWithFormat(Msg::WAITDLG_LAUNCH_APP_LABEL, true, app->getName()));
                                                app->launch();
                                            }
                                            });
                                    } else {
                                        gotoView(VIEW_OPTIONS, B("Wine"));
                                    }
                                    });
                            } else {
                            }
                        }
                        return false;
                    });
                }
            }));
        }
    }
    std::sort(appListViewItems.begin(), appListViewItems.end(), [](ListViewItem& a, ListViewItem& b) {
        return a.text.compareTo(b.text, true) < 0;
        });
}

static SDL_Window* window;

#ifdef BOXEDWINE_OPENGL_SDL
static SDL_GLContext gl_context;
#endif
static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

void uiShutdown() {
    int x=0, y=0;
    SDL_GetWindowPosition(window, &x, &y);
    GlobalSettings::saveScreenSize(x, y, (int)ImGui::GetIO().DisplaySize.x, (int)ImGui::GetIO().DisplaySize.y);
    GlobalSettings::saveConfig();
    BaseDlg::stopAllDialogs();

    // Cleanup
#ifdef BOXEDWINE_IMGUI_DX9
    if (StartUpArgs::uiType == UI_TYPE_DX9) {
        ImGui_ImplDX9_Shutdown();
        CleanupDeviceD3D();
    }
#endif
#ifdef BOXEDWINE_OPENGL_SDL
    if (StartUpArgs::uiType == UI_TYPE_OPENGL) {
#ifdef BOXEDWINE_OPENGL_IMGUI_V2
        ImGui_ImplOpenGL2_Shutdown();
#else
        ImGui_ImplOpenGL3_Shutdown();
#endif
        SDL_GL_DeleteContext(gl_context);
    }
#endif
    BoxedTexture::resetAll();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    
    SDL_DestroyWindow(window);
    window = nullptr;
}

bool uiIsRunning() {
    return window!= nullptr;
}

bool uiLoop() {
    // Poll and handle events (inputs, window resize, etc.)
    // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
    // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
    // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
    // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
    SDL_Event event = {};
    bool done = false;
    while (SDL_WaitEventTimeout(&event, GlobalSettings::getFrameDelayMillies()))
    {
        GlobalSettings::updateLastFrameDelayChange();
        ImGui_ImplSDL2_ProcessEvent(&event);
        if (event.type == SDL_QUIT) {
            done = true;
            if (currentView) {
                // :TODO: should we stop the quit action if this save fails?
                currentView->saveChanges();
            }
        } else if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window)) {
            done = true;
        }  else if (event.type >= SDL_USEREVENT) {
            SDL_PushEvent(&event); // if we are currently launching then there is another spot where sdl polling is happening in threadedMainloop.cpp, if we drop this custom msg then another thread will be blocked forever
            break;
        } else if (event.type == SDL_DROPFILE) {
            char* droppedFileOrDir = event.drop.file;
            static BString staticFilePath;
            
            staticFilePath = BString::copy(droppedFileOrDir);
            runOnMainUI([]() {
                gotoView(VIEW_INSTALL, B("Install"), staticFilePath);
                return false;
                });
            SDL_free(droppedFileOrDir);    // Free dropped_filedir memory
            break;
        }
    }

    // Start the Dear ImGui frame
#ifdef BOXEDWINE_IMGUI_DX9
    if (StartUpArgs::uiType == UI_TYPE_DX9) {
        ImGui_ImplDX9_NewFrame();
    }
#endif
#ifdef BOXEDWINE_OPENGL_SDL
    if (StartUpArgs::uiType == UI_TYPE_OPENGL) {
#ifdef BOXEDWINE_OPENGL_IMGUI_V2
        ImGui_ImplOpenGL2_NewFrame();
#else
        ImGui_ImplOpenGL3_NewFrame();
#endif
    }
#endif
    ImGui_ImplSDL2_NewFrame(window);
    ImGui::NewFrame();

    uiDraw();

    // Rendering    
#ifdef BOXEDWINE_IMGUI_DX9
    if (StartUpArgs::uiType == UI_TYPE_DX9) {
        g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
        g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
        g_pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
        D3DCOLOR clear_col_dx = D3DCOLOR_RGBA((int)(clear_color.x * 255.0f), (int)(clear_color.y * 255.0f), (int)(clear_color.z * 255.0f), (int)(clear_color.w * 255.0f));
        g_pd3dDevice->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, clear_col_dx, 1.0f, 0);
        ImGui::Render();
        if (g_pd3dDevice->BeginScene() >= 0)
        {
            ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
            g_pd3dDevice->EndScene();
        }
        HRESULT result = g_pd3dDevice->Present(nullptr, nullptr, nullptr, nullptr);

        // Handle loss of D3D9 device
        if (result == D3DERR_DEVICELOST && g_pd3dDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET) {
            ResetDevice();
        }
    }
#endif
#ifdef BOXEDWINE_OPENGL_SDL
    if (StartUpArgs::uiType == UI_TYPE_OPENGL) {
        ImGui::Render();
        ImGuiIO& io = ImGui::GetIO();
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
#ifdef BOXEDWINE_OPENGL_IMGUI_V2
        ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
#else
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
#endif
        SDL_GL_SwapWindow(window);
    }
#endif    

    runFunctions(runAfterFrameFunctions);
    return done;
}

bool uiShow(BString basePath) {
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

    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);

#ifdef BOXEDWINE_OPENGL_SDL
    const char* glsl_version = nullptr;
    if (StartUpArgs::uiType == UI_TYPE_OPENGL) {
        // Decide GL+GLSL versions
#if __APPLE__
    // GL 3.2 Core + GLSL 150
        glsl_version = "#version 150";
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#elif defined (BOXEDWINE_OPENGL_IMGUI_V2)
        glsl_version = "#version 100";
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
#else
    // GL 3.0 + GLSL 130
        glsl_version = "#version 150";
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif

        // Create window with graphics context
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
        SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
        window_flags = (SDL_WindowFlags)(window_flags | SDL_WINDOW_OPENGL);
    }
#endif

    int cx = GlobalSettings::getPreviousScreenWidth();
    int cy = GlobalSettings::getPreviousScreenHeight();
    U32 scale = SCALE_DENOMINATOR;

    SDL_DisplayMode dm = { 0 };
    SDL_GetDesktopDisplayMode(0, &dm);

    int x = GlobalSettings::getPreviousScreenX();
    int y = GlobalSettings::getPreviousScreenY();

#ifdef BOXEDWINE_HIGHDPI
    scale = KNativeSystem::getDpiScale();
#endif
    if (cx < 320 || cy < 320 || (dm.w && cx > dm.w) || (dm.h && cy > dm.h)) {
        cx = 1024 * scale / SCALE_DENOMINATOR;
        cy = 768 * scale / SCALE_DENOMINATOR;
        x = SDL_WINDOWPOS_CENTERED;
        y = SDL_WINDOWPOS_CENTERED;
    }
    if (y<20 || (x != SDL_WINDOWPOS_CENTERED && ((dm.w && x + cx > dm.w) || (dm.w && y + cy > dm.w)))) {
        x = SDL_WINDOWPOS_CENTERED;
        y = SDL_WINDOWPOS_CENTERED;
    }

    window = SDL_CreateWindow("Boxedwine UI", x, y, cx, cy, window_flags);
    // when launching boxedwine as another process, if that process creates and destroys more than 1 window (changing emulated resolution), on Windows at least, when that process exits the above create window sometimes won't be on top
    SDL_RaiseWindow(window);
#ifdef BOXEDWINE_IMGUI_DX9
    if (StartUpArgs::uiType == UI_TYPE_DX9) {
        SDL_SysWMinfo wmInfo = {};
        SDL_VERSION(&wmInfo.version);
        SDL_GetWindowWMInfo(window, &wmInfo);
        HWND hwnd = (HWND)wmInfo.info.win.window;
        if (!CreateDeviceD3D(hwnd)) {
            CleanupDeviceD3D();
            return false;
        }
    }
#endif
#ifdef BOXEDWINE_OPENGL_SDL
    if (StartUpArgs::uiType == UI_TYPE_OPENGL) {
        gl_context = SDL_GL_CreateContext(window);
        if (!gl_context) {
            fprintf(stderr, "SDL_GL_CreateContext failed.\n");
            return false;
        }
        SDL_GL_MakeCurrent(window, gl_context);

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
            return false;
        }
    }
#endif
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
#ifdef BOXEDWINE_HIGHDPI
    ImGuiContext* context = ImGui::CreateContext();
    GlobalSettings::setScale(scale);
    context->Style.ScaleAllSizes(GlobalSettings::scaleFloatUI(1.0f));
#else
    ImGui::CreateContext();
#endif
    GlobalSettings::loadFonts();
    BoxedwineData::loadUI();
    currentViewDeprecated = VIEW_APPS;
    if (currentView) {
        delete currentView;
        currentView = nullptr;
    }
    loadApps(); // need to be after we create the context for images to work
    createButton();
    if (GlobalSettings::startUpArgs.runOnRestartUI) {
        runOnMainUI([]() {
            if (GlobalSettings::startUpArgs.runOnRestartUI) {
                std::function<void()> f = GlobalSettings::startUpArgs.runOnRestartUI;
                GlobalSettings::startUpArgs.runOnRestartUI = nullptr;
                f();                
            }
            return false;
            });        
    }
    if (GlobalSettings::startUpArgs.showAppPickerForContainerDir.length()) {
        runOnMainUI([]() {
            BoxedContainer* container = BoxedwineData::getContainerByDir(GlobalSettings::startUpArgs.showAppPickerForContainerDir);
            if (container) {
                std::vector<BoxedApp> items;
                container->getNewApps(items);
                new AppChooserDlg(items, [container](BoxedApp app) {
                    gotoView(VIEW_CONTAINERS, container->getDir(), app.getIniFilePath());
                    });
            }
            GlobalSettings::startUpArgs.showAppPickerForContainerDir = B("");
            return false;
        });

    }
    //ImGuiIO& io = ImGui::GetIO();
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    GlobalSettings::loadTheme();

    // Setup Platform/Renderer bindings    
#ifdef BOXEDWINE_IMGUI_DX9
    if (StartUpArgs::uiType == UI_TYPE_DX9) {
        ImGui_ImplSDL2_InitForD3D(window);
        ImGui_ImplDX9_Init(g_pd3dDevice);
    }
#endif
#ifdef BOXEDWINE_OPENGL_SDL
    if (StartUpArgs::uiType == UI_TYPE_OPENGL) {
        ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
#ifdef BOXEDWINE_OPENGL_IMGUI_V2
        ImGui_ImplOpenGL2_Init();
#else
        ImGui_ImplOpenGL3_Init(glsl_version);
#endif
    }
#endif
    if (GlobalSettings::getWineVersions().size()==0) {
        runOnMainUI([]() {
            if (GlobalSettings::getAvailableWineVersions().size() == 0 && GlobalSettings::isFilesListDownloading()) {
                new WaitDlg(Msg::WAITDLG_GET_FILE_LIST_TITLE, getTranslation(Msg::WAITDLG_GET_FILE_LIST_LABEL), []() {
                    if (!GlobalSettings::isFilesListDownloading()) {
                        runOnMainUI([]() {
                            if (GlobalSettings::getAvailableWineVersions().size() == 0) {
                                new OkDlg(Msg::GENERIC_DLG_ERROR_TITLE, getTranslation(Msg::ERROR_NO_FILE_LIST), []() {
                                    });
                            } else {
                                askToDownloadDefaultWine();
                            }
                            return false;
                            });
                    }
                    return GlobalSettings::isFilesListDownloading();
                    });
            } else if (GlobalSettings::getAvailableWineVersions().size()) {
                askToDownloadDefaultWine();
            } else {
                // :TODO:
            }
            return false;
            });
    }
    return uiContinue();    
}

bool uiContinue() {
    // Main loop
    bool done = false;
    while (!done && !GlobalSettings::startUpArgs.readyToLaunch && !GlobalSettings::restartUI) {
        done = uiLoop();
    }
    if (done || GlobalSettings::restartUI) {
        uiShutdown();
    }
    if (GlobalSettings::restartUI) {
        return true;
    }
    return GlobalSettings::startUpArgs.readyToLaunch;
}