#ifndef __CONTAINER_OPTIONS_H__
#define __CONTAINER_OPTIONS_H__

class BoxedContainer;

class ContainerOptionsDlg : public BaseDlg {
public:
    ContainerOptionsDlg(BoxedContainer* container);

protected:
    virtual void run();
    virtual void onOk(bool buttonClicked);

private:
    BoxedContainer* container;
    const char* wineVersionLabel;
    const char* wineVersionHelp;
    const char* addAppLabel;
    const char* addAppHelp;
    const char* addAppButtonLabel;
    ImVec2 leftColumnWidth;
    ComboboxData wineVersionComboboxData;
};

#endif