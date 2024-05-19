#ifndef __BASE_DLG_H__
#define __BASE_DLG_H__

class BaseDlg {
public:
    BaseDlg(Msg title, int width, int height, ImFont* font = nullptr, BaseDlg* parent = nullptr);
    virtual ~BaseDlg() {}

    static void runDialogs();
    static void stopAllDialogs();
protected:
    virtual void run() = 0;
    virtual void onOk(bool buttonClicked);

    virtual void done();
    void toolTip(const char* desc);
    void addOkAndCancelButtons();
    void addCancelButton();
    float getReservedHeightForButtons();
    float getOuterFramePadding();

    int width;
    int height;
    float toolTipWidth;
    float extraVerticalSpacing;
private:
    static std::list<BaseDlg*> activeDialogs;

    float getButtonRowY();
    void runIfVisible();

    const char* title;
    bool isDone;
    BaseDlg* child;
    ImFont* font;
};
#endif