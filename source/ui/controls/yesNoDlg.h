#ifndef __YESNO_DLG_H__
#define __YESNO_DLG_H__

class YesNoDlg : public BaseDlg {
public:
    YesNoDlg(int title, const std::string& label, std::function<void(bool)> onCompleted);

protected:
    virtual void run();

private:
    std::string label;
    std::function<void(bool)> onCompleted;
};

#endif