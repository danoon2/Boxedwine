#ifndef __BOXED_APP_H__
#define __BOXED_APP_H__

class BoxedContainer;

class BoxedApp {
public:
    bool Load(BoxedContainer* container, const wxString& iniFilepath);

    const wxString& GetName() {return this->name;}
    const wxString& GetPath() {return this->path;}
    void Launch(bool showConsole);
    wxIcon* CreateIcon(int size);
    BoxedContainer* GetContainer() {return this->container;}
    bool IsLink() { return link.Length()>0;}
    bool SaveApp();

private:
    friend class BoxedContainer;
    wxString name;
    wxString path;
    wxString icon;
    wxString link;
    wxString cmd;
    BoxedContainer* container;
    wxString iniFilePath;
};

#endif