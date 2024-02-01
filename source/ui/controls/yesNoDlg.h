#ifndef __YESNO_DLG_H__
#define __YESNO_DLG_H__

class YesNoDlg : public BaseDlg {
public:
    YesNoDlg(Msg title, BString label, std::function<void(bool)> onCompleted);

protected:
    // from BaseDlg
    void run() override;

private:
    BString label;
    std::function<void(bool)> onCompleted;
};

#endif