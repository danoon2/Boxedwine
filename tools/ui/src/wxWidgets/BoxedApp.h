#ifndef __BOXED_APP_H__
#define __BOXED_APP_H__

class BoxedContainer;
class BoxedApp {
public:
    bool Load(BoxedContainer* container, const wxString& root, const wxString& iniFilepath);

    const wxString& GetName() {return this->name;}
    const wxString& GetPath() {return this->path;}
    void Launch(bool showConsole);
    wxIcon* CreateIcon(int size);
    BoxedContainer* GetContainer() {return this->container;}
private:
    wxString name;
    wxString path;
    wxString icon;
    wxString link;
    wxString cmd;
    wxString root;
    BoxedContainer* container;
};

#endif