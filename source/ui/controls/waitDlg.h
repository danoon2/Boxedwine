#ifndef __WAIT_DLG_H__
#define __WAIT_DLG_H__

class WaitDlg : public BaseDlg {
public:
    WaitDlg(int title, BString label);
    WaitDlg(int title, BString label, std::function<bool()> checkIfShouldContinue);
    virtual ~WaitDlg();

    void addSubLabel(BString subLabel, int max);

    std::function<void()> onDone;
protected:
    virtual void run();

private:
    BString label;
    std::deque<BString> subLabels;
    std::function<bool()> checkIfShouldContinue;
};

#endif