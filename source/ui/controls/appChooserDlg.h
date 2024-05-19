#ifndef __APP_CHOOSER_DLG_H__
#define __APP_CHOOSER_DLG_H__

class AppChooserDlg : public BaseDlg {
public:
    AppChooserDlg(std::vector<BoxedApp>& items, std::vector<BoxedApp>& wineApps, std::function<void(BoxedApp)> onSelected, bool saveApp = true, BaseDlg* parent=nullptr);
    AppChooserDlg(std::vector<BoxedApp>& items, std::function<void(BoxedApp)> onSelected, bool saveApp = true, BaseDlg* parent = nullptr, Msg titleId = Msg::APPCHOOSER_DLG_TITLE);

    void setLabelId(Msg id) {this->labelId = id;}
protected:
    // from BaseDlg
    void run() override;
    void onOk(bool buttonClicked) override;

private:
    void drawItems(std::vector<BoxedApp>& apps, int startingIndex);

    std::vector<BoxedApp> items;
    std::vector<BoxedApp> wineApps;
    std::function<void(BoxedApp app)> onSelected;
    Msg labelId;
    bool saveApp;
};

#endif