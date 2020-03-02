#ifndef __APP_CHOOSER_DLG_H__
#define __APP_CHOOSER_DLG_H__

class AppChooserDlg : BaseDlg {
public:
    AppChooserDlg(BoxedContainer* container, BaseDlg* parent=NULL);

protected:
    virtual void run();
    virtual void onOk(bool buttonClicked);

private:
    std::vector<BoxedApp> items;
};

#endif