#ifndef __YESNO_DLG_H__
#define __YESNO_DLG_H__

class YesNoDlg : public BaseDlg {
public:
    YesNoDlg(int title, BString label, std::function<void(bool)> onCompleted);

protected:
    virtual void run();

private:
    BString label;
    std::function<void(bool)> onCompleted;
};

#endif