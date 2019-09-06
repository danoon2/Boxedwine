#ifndef __BOXED_CONTAINER_LIST_H__
#define __BOXED_CONTAINER_LIST_H__

class BoxedContainerList: public wxListView
{
public:
    BoxedContainerList(wxWindow *parent, const wxWindowID id, const wxPoint& pos, const wxSize& size, long style) : wxListView(parent, id, pos, size, style) {}

    void OnItemRightClick(wxListEvent& event);
    void OnContextMenu(wxContextMenuEvent& event);
    void OnActivated(wxListEvent& event);
    void OnMenu(wxCommandEvent& event);
    void OnColClick(wxListEvent& event);

private:
    void ShowContextMenu(BoxedContainer* app);

    wxDECLARE_NO_COPY_CLASS(BoxedContainerList);
    wxDECLARE_EVENT_TABLE();
};

#endif