#ifndef __WAIT_DLG_H__
#define __WAIT_DLG_H__

class WaitDlg : public BaseDlg {
public:
    WaitDlg(int title, const std::string& label);

protected:
    virtual void run();

private:
    std::string label;
};

#endif