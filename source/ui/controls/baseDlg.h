#ifndef __BASE_DLG_H__
#define __BASE_DLG_H__

#define COLUMN_PADDING 10

class ComboboxData {
public:
    ComboboxData() : dataForCombobox(0), currentSelectedIndex(0) {}
    char* dataForCombobox;
    std::vector<std::string> data;
    int currentSelectedIndex;

    void dataChanged();
};

class BaseDlg {
public:
    BaseDlg(int title, int width, int height);

    static void runDialogs();    
protected:
    virtual void run() = 0;
    virtual void onOk(bool buttonClicked);

    virtual void done();
    void toolTip(const char* desc);
    void addOkAndCancelButtons();

    int width;
    int height;
    float toolTipWidth;
    float extraVerticalSpacing;
private:
    static std::list<BaseDlg*> activeDialogs;

    void runIfVisible();

    const char* title;
    bool isDone;
};
#endif