#ifndef __BOXED_APP_LIST_H__
#define __BOXED_APP_LIST_H__

class BoxedFrame;

class BoxedAppList: public wxListView
{
public:
    BoxedAppList(BoxedFrame* mainFrame, wxWindow *parent, const wxWindowID id, const wxPoint& pos, const wxSize& size, long style) : wxListView(parent, id, pos, size, style), mainFrame(mainFrame) {}

    void OnItemRightClick(wxListEvent& event);
    void OnContextMenu(wxContextMenuEvent& event);
    void OnActivated(wxListEvent& event);
    void OnMenu(wxCommandEvent& event);

private:
    void ShowContextMenu(BoxedApp* app);

    BoxedFrame* mainFrame;

    wxDECLARE_NO_COPY_CLASS(BoxedAppList);
    wxDECLARE_EVENT_TABLE();
};

#endif