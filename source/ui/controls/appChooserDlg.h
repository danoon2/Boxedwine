#ifndef __APP_CHOOSER_DLG_H__
#define __APP_CHOOSER_DLG_H__

class AppChooserDlg : public BaseDlg {
public:
    AppChooserDlg(std::vector<BoxedApp>& items, std::vector<BoxedApp>& wineApps, std::function<void(BoxedApp*)> onSelected, BaseDlg* parent=NULL);
    AppChooserDlg(std::vector<BoxedApp>& items, std::function<void(BoxedApp*)> onSelected, BaseDlg* parent = NULL, int titleId= APPCHOOSER_DLG_TITLE);

    void setLabelId(int id) {this->labelId = id;}
protected:
    virtual void run();
    virtual void onOk(bool buttonClicked);

private:
    void drawItems(std::vector<BoxedApp>& apps, int startingIndex);

    std::vector<BoxedApp> items;
    std::vector<BoxedApp> wineApps;
    std::function<void(BoxedApp*)> onSelected;
    int labelId;
};

#endif