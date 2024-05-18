#include "boxedwine.h"
#include "../boxedwineui.h"
#include "../../../lib/imgui/addon/imguitinyfiledialogs.h"

#ifdef BOXEDWINE_OPENGL_OSMESA
bool isMesaOpenglAvailable();
#endif

ContainersView::ContainersView(BString tab, BString app) : BaseView(B("ContainersView")), currentContainer(nullptr), currentContainerChanged(false), currentContainerMountChanged(false), currentApp(nullptr), currentAppChanged(false) {
    std::shared_ptr<ImGuiLayout> model = std::make_shared<ImGuiLayout>();        
    section = model->addSection();

    containerNameControl = section->addTextInputRow(Msg::CONTAINER_VIEW_CONTAINER_NAME_LABEL, Msg::NONE);
    containerNameControl->onChange = [this]() {
        this->setTabName(this->tabIndex, this->containerNameControl->getText());
        currentContainer->setName(this->containerNameControl->getText());
        section->setTitle(currentContainer->getName());
        this->currentContainerChanged = true;
    };

    containerFileSystemControl = createFileSystemVersionCombobox(section);
    containerFileSystemControl->setReadOnly(true);
    containerFileSystemControl->onChange = [this]() {
        this->currentContainerChanged = true;
        bool renderer = this->currentContainer->getWineVersionAsNumber(this->containerFileSystemControl->getSelectionStringValue()) > 500;
        containerGdiControl->setRowHidden(renderer);
        containerRendererControl->setRowHidden(!renderer);
        std::shared_ptr<FileSystemZip> fileSystem = GlobalSettings::getInstalledFileSystemFromName(this->containerFileSystemControl->getSelectionStringValue());
        appDirectDrawAutoRefreshControl->setReadOnly(atoi(fileSystem->fsVersion.c_str()) < 7 || !fileSystem->hasWine());
    };

    std::shared_ptr<LayoutRow> row = section->addRow(Msg::CONTAINER_VIEW_CONTAINER_LOCATION_LABEL, Msg::NONE);
    containerLocationControl = row->addTextInput(B(""), true);
    std::shared_ptr<LayoutButtonControl> showButtonControl = row->addButton(getTranslation(Msg::GENERIC_OPEN_BUTTON));
    showButtonControl->onChange = [this]() {
        Platform::openFileLocation(currentContainer->getDir());
        };

    section->addSeparator();

    containerWindowsVersionControl = createWindowsVersionCombobox(section);
    containerWindowsVersionControl->onChange = [this]() {        
        this->currentContainerChanged = true;        
    };


    containerGdiControl = section->addCheckbox(Msg::CONTAINER_VIEW_GDI_RENDERER_LABEL, Msg::CONTAINER_VIEW_GDI_RENDERER_HELP, false);
    containerGdiControl->onChange = [this]() {        
        this->currentContainerChanged = true;
    };

    std::vector<ComboboxItem> rendererOptions;
    rendererOptions.push_back(ComboboxItem(B("OpenGL"), B("gl")));
    rendererOptions.push_back(ComboboxItem(B("Vulkan"), B("vulkan")));
    rendererOptions.push_back(ComboboxItem(B("GDI"), B("gdi")));
    containerRendererControl = section->addComboboxRow(Msg::CONTAINER_VIEW_RENDERER_LABEL, Msg::CONTAINER_VIEW_RENDERER_HELP, rendererOptions);
    containerRendererControl->onChange = [this]() {
        this->currentContainerChanged = true;
    };
    containerRendererControl->setWidth((int)GlobalSettings::scaleFloatUIAndFont(150));

    std::vector<ComboboxItem> mouseOptions;
    mouseOptions.push_back(ComboboxItem(B("Enable"), B("enable")));
    mouseOptions.push_back(ComboboxItem(B("Disable"), B("disable")));
    mouseOptions.push_back(ComboboxItem(B("Force"), B("force")));
    containerMouseWarpControl = section->addComboboxRow(Msg::CONTAINER_VIEW_MOUSE_WARP_LABEL, Msg::CONTAINER_VIEW_MOUSE_WARP_HELP, mouseOptions);
    containerMouseWarpControl->onChange = [this]() {
        this->currentContainerChanged = true;
    };
    containerMouseWarpControl->setWidth((int)GlobalSettings::scaleFloatUIAndFont(150));

    std::vector<ComboboxItem> mountDrives;
    mountDrives.push_back(ComboboxItem(B(" ")));
    for (int i = 3; i < 26; i++) {
        mountDrives.push_back(ComboboxItem(BString((char)('A' + i)) + ":", BString((char)('a' + i))));
    }
    row = section->addRow(Msg::CONTAINER_VIEW_MOUNT_DIR_LABEL, Msg::CONTAINER_VIEW_MOUNT_DIR_HELP);
    containerMountDriveControl = row->addComboBox(mountDrives);
    containerMountDriveControl->setWidth((int)GlobalSettings::scaleFloatUIAndFont(50));
    containerMountDriveControl->onChange = [this]() {
        this->currentContainerChanged = true;
        this->currentContainerMountChanged = true;
    };
    containerMountPathControl = row->addTextInput();
    containerMountPathControl->setBrowseDirButton();
    containerMountPathControl->onChange = [this]() {
        this->currentContainerChanged = true;
        this->currentContainerMountChanged = true;
    };    

    std::shared_ptr<LayoutButtonControl> selectWineAppButton = section->addButton(Msg::CONTAINER_VIEW_PROGRAMS_LABEL, Msg::NONE, getTranslation(Msg::CONTAINER_VIEW_RUNE_APP_BUTTON_LABEL));
    selectWineAppButton->onChange = [this]() {
        if (saveChanges()) { // need to capture any changes to mount
            runOnMainUI([this] {
                std::vector<BoxedApp> apps;
                this->currentContainer->findApps(apps);
                AppChooserDlg* dlg = new AppChooserDlg(apps, [this](BoxedApp app) {
                    app.launch();                    
                    BString containerPath = this->currentContainer->getDir();
                    GlobalSettings::startUpArgs.runOnRestartUI = [containerPath]() {
                        gotoView(VIEW_CONTAINERS, containerPath);
                    };
                    runOnMainUI([app]() {
                        new WaitDlg(Msg::WAITDLG_LAUNCH_APP_TITLE, getTranslationWithFormat(Msg::WAITDLG_LAUNCH_APP_LABEL, true, app.getCmd()));
                        return false;
                        });
                    }, false, nullptr, Msg::CONTAINER_VIEW_SELECT_WINE_APP_DLG_TITLE);
                dlg->setLabelId(Msg::CONTAINER_VIEW_SELECT_WINE_APP_LABEL);
                return false;
                });
        }
    };

    if (GlobalSettings::getComponents().size()) {
        row = section->addRow(Msg::CONTAINER_VIEW_COMPONENTS_LABEL, Msg::NONE);

        std::vector<ComboboxItem> components;
        for (auto& component : GlobalSettings::getComponents()) {
            components.push_back(ComboboxItem(component.name));
        }
        componentsControl = row->addComboBox(components, 0);

        std::shared_ptr<LayoutButtonControl> installButton = row->addButton(getTranslation(Msg::INSTALLVIEW_INSTALL_BUTTON_LABEL));
        installButton->onChange = [this]() {
            if (this->saveChanges()) {
                AppFile& app = GlobalSettings::getComponents()[componentsControl->getSelection()];
                if (Fs::doesNativePathExist(app.localFilePath)) {
                    app.install(false, this->currentContainer);
                } else {
                    GlobalSettings::downloadFile(app.filePath, app.localFilePath, app.name, app.size, [&app, this](bool sucess) {
                        app.install(false, this->currentContainer);
                        });
                }
            }
        };
    }

    row = section->addRow(Msg::CONTAINER_VIEW_TINY_CORE_LABEL, Msg::NONE);
    packagesControl = row->addComboBox();

    std::shared_ptr<LayoutButtonControl> installButton = row->addButton(getTranslation(Msg::INSTALLVIEW_INSTALL_BUTTON_LABEL));
    installButton->onChange = [this]() {
        if (this->saveChanges()) {
            BString package = packagesControl->getSelectionStringValue();
            currentContainer->installTinyCorePackage(package);
        }
        };
    installButton->setHelpId(Msg::CONTAINER_OPTIONS_DOWNLOAD_PACKAGE);

    row = section->addRow(Msg::CONTAINER_VIEW_WINETRICKS_FONTS_LABEL, Msg::NONE);
    fontsControl = row->addComboBox();

    std::shared_ptr<LayoutButtonControl> fontInstallButton = row->addButton(getTranslation(Msg::INSTALLVIEW_INSTALL_BUTTON_LABEL));
    fontInstallButton->onChange = [this]() {
        if (this->saveChanges()) {
            BString verb = fontsControl->getSelectionStringValue();
            std::shared_ptr<FileSystemZip> wineTrickFileSystem = currentContainer->getFileSystem().lock();
            if (wineTrickFileSystem) {
                this->winetricks(wineTrickFileSystem, verb);
            }
        }
    };
    fontInstallButton->setHelpId(Msg::CONTAINER_OPTIONS_DOWNLOAD_WINETRICKS);

    row = section->addRow(Msg::CONTAINER_VIEW_WINETRICKS_DLLS_LABEL, Msg::NONE);
    dllsControl = row->addComboBox();

    std::shared_ptr<LayoutButtonControl> installButton2 = row->addButton(getTranslation(Msg::INSTALLVIEW_INSTALL_BUTTON_LABEL));
    installButton2->onChange = [this]() {
        if (this->saveChanges()) {
            BString verb = dllsControl->getSelectionStringValue();
            std::shared_ptr<FileSystemZip> wineTrickFileSystem = currentContainer->getFileSystem().lock();
            if (wineTrickFileSystem) {
                this->winetricks(wineTrickFileSystem, verb);
            }
        }
    };
    installButton2->setHelpId(Msg::CONTAINER_OPTIONS_DOWNLOAD_WINETRICKS);
    

    section->addSeparator();
    std::shared_ptr<LayoutButtonControl> selectAppButton = section->addButton(Msg::CONTAINER_OPTIONS_DLG_ADD_APP_LABEL, Msg::CONTAINER_OPTIONS_DLG_ADD_APP_HELP, getTranslation(Msg::CONTAINER_OPTIONS_DLG_ADD_APP_BUTTON_LABEL));
    selectAppButton->onChange = [this]() {
        if (saveChanges()) { // need to capture any changes to mount
            runOnMainUI([this] {
                std::vector<BoxedApp> items;
                std::vector<BoxedApp> wineApps;
                this->currentContainer->getNewApps(items);
                this->currentContainer->findApps(wineApps);
                new AppChooserDlg(items, wineApps, [this](BoxedApp app) {
                    BString iniPath = app.getIniFilePath();
                    this->setCurrentApp(app.getContainer()->getAppByIniFile(iniPath));
                    rebuildShortcutsCombobox();
                    showAppSection(true);
                    this->appPickerControl->setSelectionStringValue(iniPath);
                    });
                return false;
                });
        }
    };

    appSection = model->addSection();
    appSection->setIndent(true);

    row = section->addRow(Msg::CONTAINER_VIEW_SHORTCUT_LIST_LABEL, Msg::CONTAINER_VIEW_SHORTCUT_LIST_HELP);
    appPickerControl = row->addComboBox();
    appPickerControl->onChange = [this]() {
        if (this->saveChanges()) {
            this->setCurrentApp(this->currentContainer->getApps()[this->appPickerControl->getSelection()]);
        }
    };

    BString label;
    /*
    if (GlobalSettings::hasIconsFont()) {
        label += DELETE_ICON;
        label += " ";
    }
    */
    label += getTranslation(Msg::CONTAINER_VIEW_DELETE_SHORTCUT);
    std::shared_ptr<LayoutButtonControl> deleteButton = row->addButton(label);
    deleteButton->onChange = [this]() {
        BString label = getTranslationWithFormat(Msg::CONTAINER_VIEW_DELETE_SHORTCUT_CONFIRMATION, true, this->currentApp->getName());
        runOnMainUI([label, this]() {
            new YesNoDlg(Msg::GENERIC_DLG_CONFIRM_TITLE, label, [this](bool yes) {
                if (yes) {
                    runOnMainUI([this]() {
                        this->currentAppChanged = false;
                        this->currentApp->remove();
                        if (this->currentContainer->getApps().size()) {
                            this->setCurrentApp(this->currentContainer->getApps()[0]);
                        } else {
                            this->currentApp = nullptr;
                        }
                        showAppSection(this->currentContainer->getApps().size() != 0);
                        rebuildShortcutsCombobox();
                        GlobalSettings::reloadApps();
                        return false;
                        });
                }
                });
            return false;
            });
    };
    row = appSection->addRow(Msg::NONE, Msg::NONE);
    row->setTopMargin(0.0f);
    row->addCustomControl([this]() {
        if (this->currentApp && this->currentApp->getIconTexture()) {
            ImVec2 pos = ImGui::GetCursorPos();
            ImGui::SetCursorPosX(0);
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + GlobalSettings::extraVerticalSpacing);
            ImGui::Image(this->currentApp->getIconTexture()->texture->getTexture(), ImVec2((float)UiSettings::ICON_SIZE, (float)UiSettings::ICON_SIZE));
            ImGui::SetCursorPos(pos);
        }
        });

    appNameControl = appSection->addTextInputRow(Msg::CONTAINER_VIEW_NAME_LABEL, Msg::CONTAINER_VIEW_NAME_HELP);
    appNameControl->onChange = [this]() {
        this->currentApp->name = appNameControl->getText();
        this->currentAppChanged = true;
        rebuildShortcutsCombobox();
    };

    appPathControl = appSection->addTextInputRow(Msg::CONTAINER_VIEW_SHORTCUT_PATH_LABEL, Msg::CONTAINER_VIEW_SHORTCUT_PATH_HELP, B(""), true);
    appArgumentsControl = appSection->addTextInputRow(Msg::CONTAINER_VIEW_SHORTCUT_ARGUMENTS_LABEL, Msg::CONTAINER_VIEW_SHORTCUT_ARGUMENTS_HELP);
    appArgumentsControl->setNumberOfLines(1);
    appArgumentsControl->onChange = [this]() {
        this->currentAppChanged = true;
        std::vector<BString> args;
        appArgumentsControl->getText().split('\n', args);
        appArgumentsControl->setNumberOfLines((int)args.size() + 1);
    };

    std::vector<ComboboxItem> resolutions;
    resolutions.push_back(ComboboxItem(getTranslation(Msg::GENERIC_DEFAULT), B("")));
    for (auto& res : GlobalSettings::getAvailableResolutions()) {
        resolutions.push_back(ComboboxItem(res));
    }
    appResolutionControl = appSection->addComboboxRow(Msg::CONTAINER_VIEW_RESOLUTION_LABEL, Msg::CONTAINER_VIEW_RESOLUTION_HELP, resolutions);
    appResolutionControl->onChange = [this]() {
        this->currentAppChanged = true;
    };

    std::vector<ComboboxItem> bpp;
    bpp.push_back(ComboboxItem(B("32-bit (default)"), 32));
    bpp.push_back(ComboboxItem(B("16-bit"), 16));
    bpp.push_back(ComboboxItem(B("8-bit (256 colors))"), 8));
    appBppControl = appSection->addComboboxRow(Msg::CONTAINER_VIEW_BPP_LABEL, Msg::CONTAINER_VIEW_BPP_HELP, bpp);
    appBppControl->onChange = [this]() {
        this->currentAppChanged = true;
    };

    std::vector<ComboboxItem> fullscreen;
    fullscreen.push_back(ComboboxItem(B("Not Set"), FULLSCREEN_NOTSET));
    fullscreen.push_back(ComboboxItem(B("Stretch"), FULLSCREEN_STRETCH));
    fullscreen.push_back(ComboboxItem(B("Letterbox (maintain aspect ratio)"), FULLSCREEN_ASPECT));
    appFullScreenControl = appSection->addComboboxRow(Msg::CONTAINER_VIEW_FULL_SCREEN_LABEL, Msg::CONTAINER_VIEW_FULL_SCREEN_HELP, fullscreen);
    appFullScreenControl->onChange = [this]() {
        this->currentAppChanged = true;
    };

    std::vector<ComboboxItem> vsync;
    vsync.push_back(ComboboxItem(B("Not Set"), VSYNC_NOT_SET));
    vsync.push_back(ComboboxItem(B("Disabled"), VSYNC_DISABLED));
    vsync.push_back(ComboboxItem(B("Enabled"), VSYNC_ENABLED));
    vsync.push_back(ComboboxItem(B("Adaptive"), VSYNC_ADAPTIVE));
    appVSyncControl = appSection->addComboboxRow(Msg::CONTAINER_VIEW_VSYNC_LABEL, Msg::CONTAINER_VIEW_VSYNC_HELP, vsync);
    appVSyncControl->onChange = [this]() {
        this->currentAppChanged = true;
    };

    if (GlobalSettings::isDpiAware()) {
        appDpiAwareControl = appSection->addCheckbox(Msg::CONTAINER_VIEW_DPI_AWARE_LABEL, Msg::CONTAINER_VIEW_DPI_AWARE_HELP, false);
        appDpiAwareControl->onChange = [this]() {
            this->currentAppChanged = true;
        };
    }
    appPollRateControl = appSection->addTextInputRow(Msg::CONTAINER_VIEW_POLL_RATE_LABEL, Msg::CONTAINER_VIEW_POLL_RATE_HELP);
    appPollRateControl->onChange = [this]() {
        this->currentApp->pollRate = atoi(appPollRateControl->getText().c_str());
        this->currentAppChanged = true;
    };
    appSkipFramesControl = appSection->addTextInputRow(Msg::CONTAINER_VIEW_SKIP_FRAMES_LABEL, Msg::CONTAINER_VIEW_SKIP_FRAMES_HELP);
    appSkipFramesControl->onChange = [this]() {
        this->currentApp->skipFramesFPS = atoi(appSkipFramesControl->getText().c_str());
        this->currentAppChanged = true;
    };
#ifdef BOXEDWINE_MULTI_THREADED
    std::vector<ComboboxItem> affinity;
    affinity.push_back(ComboboxItem(getTranslation(Msg::GENERIC_COMBOBOX_ALL), 0));
#ifdef __MACH__
    // Platform::setCpuAffinityForThread
    affinity.push_back(ComboboxItem(B("1"), 1));
#else
    for (U32 i = 1; i < Platform::getCpuCount(); i++) {
        affinity.push_back(ComboboxItem(BString::valueOf(i), i));
    }
#endif
    appCpuAffinityControl = appSection->addComboboxRow(Msg::CONTAINER_VIEW_CPU_AFFINITY_LABEL, Msg::CONTAINER_VIEW_CPU_AFFINITY_HELP, affinity);
    appCpuAffinityControl->onChange = [this]() {
        this->currentAppChanged = true;
    };
#endif
    std::vector<ComboboxItem> scales;
    scales.push_back(ComboboxItem(getTranslation(Msg::GENERIC_DEFAULT), 0));
    scales.push_back(ComboboxItem(B("1/2x"), 50));
    scales.push_back(ComboboxItem(B("1x"), 100));
    scales.push_back(ComboboxItem(B("2x"), 200));
    scales.push_back(ComboboxItem(B("3x"), 300));
    appScaleControl = appSection->addComboboxRow(Msg::CONTAINER_VIEW_SCALE_LABEL, Msg::CONTAINER_VIEW_SCALE_HELP, scales);
    appScaleControl->onChange = [this]() {
        this->currentAppChanged = true;
    };

    std::vector<ComboboxItem> quality;
    quality.push_back(ComboboxItem(B("Nearest Pixel Sampling (default)")));
    quality.push_back(ComboboxItem(B("Linear Filtering")));
    appScaleQualityControl = appSection->addComboboxRow(Msg::CONTAINER_VIEW_SCALE_QUALITY_LABEL, Msg::CONTAINER_VIEW_SCALE_QUALITY_HELP, quality);
    appScaleQualityControl->onChange = [this]() {
        this->currentAppChanged = true;
    };

#if defined(BOXEDWINE_OPENGL_OSMESA) && defined(BOXEDWINE_OPENGL_SDL)
    std::vector<ComboboxItem> glOptions;
    glOptions.push_back(ComboboxItem(getTranslation(Msg::GENERIC_DEFAULT), OPENGL_TYPE_NOT_SET));
    glOptions.push_back(ComboboxItem(B("Native"), OPENGL_TYPE_SDL));
    glOptions.push_back(ComboboxItem(B("Mesa - OpenGL in Software"), OPENGL_TYPE_OSMESA));
    appOpenGlControl = appSection->addComboboxRow(Msg::OPTIONSVIEW_DEFAULT_OPENGL_LABEL, Msg::OPTIONSVIEW_DEFAULT_OPENGL_HELP, glOptions);
    appOpenGlControl->setWidth((int)GlobalSettings::scaleFloatUIAndFont(250));
    appOpenGlControl->onChange = [this]() {
        this->currentAppChanged = true;
    };
#endif

    row = appSection->addRow(Msg::CONTAINER_VIEW_GL_EXT_LABEL, Msg::CONTAINER_VIEW_GL_EXT_HELP);
    appGlExControl = row->addTextInput();
    appGlExControl->setNumberOfLines(3);
    appGlExControl->onChange = [this]() {
        this->currentAppChanged = true;
    };
    
    std::shared_ptr<LayoutButtonControl> setButtonControl = row->addButton(getTranslation(Msg::CONTAINER_VIEW_GL_EXT_SET_BUTTON_LABEL));
    setButtonControl->onChange = [this]() {
        appGlExControl->setText(B("GL_EXT_multi_draw_arrays GL_ARB_vertex_program\nGL_ARB_fragment_program GL_ARB_multitexture\nGL_EXT_secondary_color GL_EXT_texture_lod_bias\nGL_NV_texture_env_combine4 GL_ATI_texture_env_combine3\nGL_EXT_texture_filter_anisotropic GL_ARB_texture_env_combine\nGL_EXT_texture_env_combine GL_EXT_texture_compression_s3tc\nGL_ARB_texture_compression GL_EXT_paletted_texture"));
        this->currentAppChanged = true;
    };    

    appShowWindowImmediatelyControl = appSection->addCheckbox(Msg::CONTAINER_VIEW_SHOW_WINDOW_LABEL, Msg::CONTAINER_VIEW_SHOW_WINDOW_HELP, false);
    appShowWindowImmediatelyControl->onChange = [this]() {
        this->currentAppChanged = true;
    };

    appDirectDrawAutoRefreshControl = appSection->addCheckbox(Msg::CONTAINER_VIEW_AUTO_REFRESH_LABEL, Msg::CONTAINER_VIEW_AUTO_REFRESH_HELP, false);
    appDirectDrawAutoRefreshControl->onChange = [this]() {
        this->currentAppChanged = true;
    };

    for (auto& item : BoxedwineData::getContainers()) {
        addTab(item->getDir(), item->getName(), model, [this, item](bool buttonPressed, BaseViewTab& tab) {
            if (buttonPressed) {
                this->setCurrentContainer(item);
                if (gotoApp.length()) {                    
                    for (auto& app : item->getApps()) {
                        if (app->getIniFilePath() == gotoApp) {
                            setCurrentApp(app);
                            break;
                        }
                    }
                    gotoApp = B("");
                }
            }
            }, [item]() {
                if (item->getApps().size()) {
                    const BoxedAppIcon* icon = item->getApps()[0]->getIconTexture((int)ImGui::GetTextLineHeight());
                    if (icon) {
                        ImGui::Image(icon->texture->getTexture(), ImVec2(ImGui::GetTextLineHeight(), ImGui::GetTextLineHeight()));
                        ImGui::SameLine();
                        return;
                    }
                }
                ImGui::Dummy(ImVec2(ImGui::GetTextLineHeight(), 0.0f));
                ImGui::SameLine();                
            }, [this, item]() {
                runOnMainUI([]() {
                    ImGui::OpenPopup("ContainerOptionsPopup");
                    return false;
                    });

                runOnMainUI([this, item]() {
                    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(GlobalSettings::scaleFloatUI(8.0f), GlobalSettings::scaleFloatUI(8.0f)));
                    ImGui::PushStyleColor(ImGuiCol_PopupBg, ImGui::GetColorU32(ImGuiCol_ScrollbarGrab) | 0xFF000000);
                    bool result = false;
                    if (ImGui::BeginPopup("ContainerOptionsPopup")) {
                        if (ImGui::Selectable(c_getTranslation(Msg::CONTAINER_VIEW_DELETE_BUTTON_LABEL))) {
                            this->deleteContainer(item);
                        }
                        ImGui::EndPopup();
                        result = true;
                    }
                    ImGui::PopStyleColor();
                    ImGui::PopStyleVar();
                    return result;
                    });
            });
        if (item->getDir() == tab) {
            this->tabIndex = this->getTabCount() - 1;
            gotoApp = app;
        }
    }

    std::shared_ptr<LayoutSection> bottomSection = model->addSection();
    bottomSection->addSeparator();
    label = "";
    if (GlobalSettings::hasIconsFont()) {
        label += TRASH_ICON;
        label += " ";
    }
    label += getTranslation(Msg::CONTAINER_VIEW_DELETE_BUTTON_LABEL);
    std::shared_ptr<LayoutButtonControl> deleteContainerButton = bottomSection->addButton(Msg::NONE, Msg::CONTAINER_VIEW_DELETE_BUTTON_HELP, label);
    deleteContainerButton->onChange = [this]() {
        this->deleteContainer(this->currentContainer);
    };
}

void ContainersView::deleteContainer(BoxedContainer* container) {
    BString label;
    if (!container->getApps().size()) {
        label = getTranslationWithFormat(Msg::CONTAINER_VIEW_DELETE_CONFIRMATION, true, container->getName());
    } else {
        label = "";

        for (auto& app : container->getApps()) {
            if (label.length() != 0) {
                label += ", ";
            }
            label += app->getName();
        }
        label = getTranslationWithFormat(Msg::CONTAINER_VIEW_DELETE_CONFIRMATION_WITH_APPS, true, container->getName(), label);
    }
    runOnMainUI([label, this, container]() {
        new YesNoDlg(Msg::GENERIC_DLG_CONFIRM_TITLE, label, [this, container](bool yes) {
            if (yes) {
                runOnMainUI([this, container]() {
                    container->deleteContainerFromFilesystem();
                    BString containerDir;

                    if (currentContainer != container) {
                        containerDir = currentContainer->getDir();
                    }
                    BoxedwineData::reloadContainers();
                    gotoView(VIEW_CONTAINERS, containerDir);
                    return false;
                    });
            }
            });
        return false;
        });
}

bool ContainersView::saveChanges() {
    if (this->currentContainer && containerNameControl->getText().length() == 0) {
        this->errorMsg = getTranslation(Msg::CONTAINER_VIEW_ERROR_BLANK_NAME);
    } else if (this->containerMountDriveControl->getSelection() != 0 && containerMountPathControl->getText().length()==0) {
        this->errorMsg = getTranslation(Msg::CONTAINER_VIEW_ERROR_MISSING_MOUNT_LOCATION);
    } else if (this->containerMountDriveControl->getSelection() == 0 && containerMountPathControl->getText().length() != 0) {
        this->errorMsg = getTranslation(Msg::CONTAINER_VIEW_ERROR_MISSING_MOUNT_DRIVE);
    } else if (this->currentApp && appNameControl->getText().length()==0) {
        this->errorMsg = getTranslation(Msg::CONTAINER_VIEW_NAME_REQUIRED);
    }
    if (this->errorMsg.isEmpty()) {
        if (this->currentContainer && this->currentContainerChanged) {
            if (this->currentContainerMountChanged) {
                this->currentContainer->clearMounts();
                if (this->containerMountDriveControl->getSelection() != 0) {
                    this->currentContainer->addNewMount(MountInfo(this->containerMountDriveControl->getSelectionStringValue(), containerMountPathControl->getText(), true));
                }
            }
            std::shared_ptr<FileSystemZip> fs = GlobalSettings::getInstalledFileSystemFromName(this->containerFileSystemControl->getSelectionStringValue());
            this->currentContainer->setFileSystem(fs);
            this->currentContainer->setWindowsVersion(BoxedwineData::getWinVersions()[this->containerWindowsVersionControl->getSelection()]);
            if (this->currentContainer->getWineVersionAsNumber(fs->wineName) > 500) {
                this->currentContainer->setGDI(containerRendererControl->getSelectionStringValue() == "gdi");
                this->currentContainer->setRenderer(containerRendererControl->getSelectionStringValue());
            } else {
                this->currentContainer->setGDI(containerGdiControl->isChecked());
                this->currentContainer->setRenderer(B(containerGdiControl->isChecked()?"gdi":"gl"));
            }
            this->currentContainer->setMouseWarpOverride(containerMouseWarpControl->getSelectionStringValue());
            this->currentContainer->saveContainer();
            this->currentContainerChanged = false;            
        }
        if (this->currentApp && this->currentAppChanged) {
            BString ext = this->appGlExControl->getText();
            ext = ext.replace('\n', ' ');
            ext = ext.replace("  ", " ");
            ext = ext.replace("  ", " ");
            this->currentApp->glExt = ext;
            this->currentApp->name = appNameControl->getText();
            this->currentApp->args.clear();
            appArgumentsControl->getText().split('\n', this->currentApp->args);
            
            this->currentApp->resolution = appResolutionControl->getSelectionStringValue();
            this->currentApp->bpp = appBppControl->getSelectionIntValue();
            this->currentApp->scale = appScaleControl->getSelectionIntValue();
            this->currentApp->scaleQuality = this->appScaleQualityControl->getSelection();
            if (this->appOpenGlControl) {
                this->currentApp->openGlType = this->appOpenGlControl->getSelectionIntValue();
            }
            this->currentApp->fullScreen = this->appFullScreenControl->getSelection();
            this->currentApp->vsync = this->appVSyncControl->getSelectionIntValue();
            if (GlobalSettings::isDpiAware()) {
                this->currentApp->dpiAware = this->appDpiAwareControl->isChecked();
            }
            this->currentApp->showWindowImmediately = this->appShowWindowImmediatelyControl->isChecked();
            this->currentApp->autoRefresh = appDirectDrawAutoRefreshControl->isChecked();
#ifdef BOXEDWINE_MULTI_THREADED
            this->currentApp->cpuAffinity = this->appCpuAffinityControl->getSelectionIntValue();
#endif
            this->currentApp->saveApp();
            this->currentAppChanged = false;
            GlobalSettings::reloadApps();
        }
    }
    return this->errorMsg.isEmpty();
}

void ContainersView::setCurrentApp(BoxedApp* app) {
    this->currentApp = app;
    appNameControl->setText(app->getName());
    appGlExControl->setText(app->glExt);
    

    if (app->link.length()) {
        appPathControl->setText(app->link);
    } else {
        appPathControl->setText(app->path+"/"+app->cmd);
    }
    BString args;
    for (auto& arg : app->args) {
        if (args.length()) {
            args += "\n";
        }
        args += arg;
    }
    appArgumentsControl->setText(args);
    appArgumentsControl->setNumberOfLines((int)app->args.size() + 1);
    appResolutionControl->setSelectionByLabel(app->resolution);
    appBppControl->setSelectionIntValue(app->bpp);    

    if (app->fullScreen != FULLSCREEN_NOTSET) {
        appScaleControl->setSelection(0);
    } else {
        appScaleControl->setSelectionIntValue(app->scale);
    }
    appScaleControl->setReadOnly(app->fullScreen != FULLSCREEN_NOTSET);
    appScaleQualityControl->setSelection(app->scaleQuality);
    if (this->appOpenGlControl) {
        appOpenGlControl->setSelectionIntValue(app->openGlType);
    }
    appFullScreenControl->setSelectionIntValue(app->fullScreen);
    appVSyncControl->setSelectionIntValue(app->vsync);
    appDpiAwareControl->setCheck(app->dpiAware);
    appPollRateControl->setText(BString::valueOf(app->pollRate));
    appSkipFramesControl->setText(BString::valueOf(app->skipFramesFPS));
    appShowWindowImmediatelyControl->setCheck(app->showWindowImmediately);
    appDirectDrawAutoRefreshControl->setCheck(app->autoRefresh);
    std::shared_ptr<FileSystemZip> fileSystem = GlobalSettings::getInstalledFileSystemFromName(this->containerFileSystemControl->getSelectionStringValue());
    bool hasAutoRefresh = fileSystem && atoi(fileSystem->fsVersion.c_str()) >= 7 && fileSystem->hasWine();
    appDirectDrawAutoRefreshControl->setReadOnly(!hasAutoRefresh);
    appDirectDrawAutoRefreshControl->setHelpId(hasAutoRefresh ? Msg::CONTAINER_VIEW_AUTO_REFRESH_HELP : Msg::CONTAINER_VIEW_AUTO_REFRESH_MISSING_HELP);
#ifdef BOXEDWINE_MULTI_THREADED
    appCpuAffinityControl->setSelectionIntValue(app->cpuAffinity);
#endif
}

void ContainersView::rebuildShortcutsCombobox() {
    std::vector<ComboboxItem> apps;
    for (auto& app : this->currentContainer->getApps()) {
        apps.push_back(ComboboxItem(app->getName(), app->getIniFilePath()));
    }
    appPickerControl->setOptions(apps);
}

void ContainersView::setCurrentContainer(BoxedContainer* container) {
    this->currentContainer = container;
    this->currentContainerChanged = false;
    this->currentContainerMountChanged = false;
    section->setTitle(container->getName());
    containerFileSystemControl->setSelectionByLabel(container->getFileSystemName());
    containerWindowsVersionControl->setSelectionByLabel(container->getWindowsVersion());
    containerNameControl->setText(container->getName());
    containerLocationControl->setText(container->getDir());

    container->updateCachedSize();
    std::shared_ptr<FileSystemZip> fs = container->getFileSystem().lock();
    bool hasWine = fs && fs->hasWine();

    containerMountDriveControl->setRowHidden(!hasWine);
    containerGdiControl->setRowHidden(!hasWine);
    containerRendererControl->setRowHidden(!hasWine);
    containerMouseWarpControl->setRowHidden(!hasWine);
    componentsControl->setRowHidden(!hasWine);
    containerWindowsVersionControl->setRowHidden(!hasWine);

    if (hasWine) {
        if (!container->getMounts().size() || !container->getMounts()[0].wine || container->getMounts()[0].localPath.length() != 1) {
            containerMountDriveControl->setSelection(0);
            containerMountPathControl->setText(B(""));
        } else {
            const MountInfo& mount = container->getMounts()[0];
            BString lowerCase = mount.localPath.toLowerCase();
            containerMountDriveControl->setSelectionStringValue(lowerCase);
            containerMountPathControl->setText(mount.nativePath);
        }

        if (this->currentContainer->getWineVersionAsNumber(this->containerFileSystemControl->getSelectionStringValue()) > 500) {
            containerGdiControl->setCheck(false);
            containerRendererControl->setSelectionStringValue(container->getRenderer());

            containerGdiControl->setRowHidden(true);
            containerRendererControl->setRowHidden(false);
        } else {
            containerGdiControl->setCheck(container->isGDI());
            containerRendererControl->setSelectionStringValue(container->getRenderer());

            containerGdiControl->setRowHidden(false);
            containerRendererControl->setRowHidden(true);
        }
        containerMouseWarpControl->setSelectionStringValue(container->getMouseWarpOverride());
    }
    std::shared_ptr<FileSystemZip> fileSystem = currentContainer->getFileSystem().lock();
    if (fileSystem && fileSystem->tinyCorePackages.size()) {
        packagesControl->setRowHidden(false);
        std::vector<ComboboxItem> packages;
        for (auto& package : fileSystem->tinyCorePackages) {
            packages.push_back(ComboboxItem(package));
        }
        packagesControl->setOptions(packages);
    } else {
        packagesControl->setRowHidden(true);
    }

    if (fileSystem && fileSystem->hasWineTricks()) {
        fontsControl->setRowHidden(false);
        dllsControl->setRowHidden(false);

        std::vector<ComboboxItem> fonts;
        std::vector<BString> lines;
        fileSystem->wineTrickFonts.split('\n', lines);
        for (auto& line : lines) {
            BString verb = line.substr(0, line.indexOf(' '));
            fonts.push_back(ComboboxItem(line.replace("[downloadable]", ""), verb));
        }
        fontsControl->setOptions(fonts);

        std::vector<ComboboxItem> dlls;
        lines.clear();
        fileSystem->wineTrickDlls.split('\n', lines);
        for (auto& line : lines) {
            BString verb = line.substr(0, line.indexOf(' '));
            dlls.push_back(ComboboxItem(line.replace("[downloadable]", ""), verb));
        }
        dllsControl->setOptions(dlls);
    } else {
        fontsControl->setRowHidden(true);
        dllsControl->setRowHidden(true);
    }

    appPickerControl->setSelection(0);
    if (this->currentContainer->getApps().size()) {
        setCurrentApp(this->currentContainer->getApps()[0]);
        rebuildShortcutsCombobox();
    } else {
        this->currentApp = nullptr;
    }
    showAppSection(this->currentContainer->getApps().size() != 0);
}

void ContainersView::showAppSection(bool show) {
    appSection->setHidden(!show);
    appPickerControl->setRowHidden(!show);
}

void ContainersView::winetricks(const std::shared_ptr<FileSystemZip>& winetricks, BString verb) {
    GlobalSettings::startUpArgs = StartUpArgs();
    GlobalSettings::startUpArgs.addZip(winetricks->filePath);
    currentContainer->launch();     
    GlobalSettings::startUpArgs.title = "Winetricks " + verb;
    GlobalSettings::startUpArgs.addArg(B("/bin/sh"));
    GlobalSettings::startUpArgs.addArg(B("/usr/local/bin/winetricks"));
    GlobalSettings::startUpArgs.addArg(verb);
    GlobalSettings::startUpArgs.envValues.push_back(B("WINETRICKS_DOWNLOADER=curl"));
    GlobalSettings::startUpArgs.readyToLaunch = true;
#ifndef BOXEDWINE_UI_LAUNCH_IN_PROCESS
    GlobalSettings::startUpArgs.ttyPrepend = true;
#endif
    runOnMainUI([]() {
        WaitDlg* dlg = new WaitDlg(Msg::WAITDLG_LAUNCH_APP_TITLE, getTranslationWithFormat(Msg::WAITDLG_LAUNCH_APP_LABEL, true, GlobalSettings::startUpArgs.title));
        KSystem::watchTTY = [dlg](BString line) {
            if (!KThread::currentThread() || KThread::currentThread()->process->name != "curl") {
                dlg->addSubLabel(line, 5);
            }
        };
        dlg->onDone = []() {
            KSystem::watchTTY = nullptr;
        };
        return false;
        });
  
}
