#ifndef __BOXED_CONTAINER_LIST_H__
#define __BOXED_CONTAINER_LIST_H__

class BoxedFrame;

class BoxedContainerList: public wxListView
{
public:
    BoxedContainerList(BoxedFrame* mainFrame, wxWindow *parent, const wxWindowID id, const wxPoint& pos, const wxSize& size, long style) : wxListView(parent, id, pos, size, style), mainFrame(mainFrame) {}

    void OnItemRightClick(wxListEvent& event);
    void OnContextMenu(wxContextMenuEvent& event);
    void OnActivated(wxListEvent& event);
    void OnMenu(wxCommandEvent& event);
    void OnColClick(wxListEvent& event);

private:
    void ShowContextMenu(BoxedContainer* app);

    BoxedFrame* mainFrame;

    wxDECLARE_NO_COPY_CLASS(BoxedContainerList);
    wxDECLARE_EVENT_TABLE();
};

#endif