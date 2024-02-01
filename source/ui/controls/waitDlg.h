#ifndef __WAIT_DLG_H__
#define __WAIT_DLG_H__

class WaitDlg : public BaseDlg {
public:
    WaitDlg(Msg title, BString label);
    WaitDlg(Msg title, BString label, std::function<bool()> checkIfShouldContinue);
    virtual ~WaitDlg();

protected:
    // from BaseDlg
    void run() override;

public:
    void addSubLabel(BString subLabel, int max);

    std::function<void()> onDone;

private:
    BString label;
    std::deque<BString> subLabels;
    std::function<bool()> checkIfShouldContinue;
};

#endif