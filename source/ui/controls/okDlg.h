#ifndef __OK_DLG_H__
#define __OK_DLG_H__

class OkDlg : public BaseDlg {
public:
    OkDlg(int title, const std::string& label, std::function<void()> onDone);

protected:
    virtual void run();

private:
    std::string label;
    std::function<void()> onDone;
};

#endif