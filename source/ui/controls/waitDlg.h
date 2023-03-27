#ifndef __WAIT_DLG_H__
#define __WAIT_DLG_H__

class WaitDlg : public BaseDlg {
public:
    WaitDlg(int title, const std::string& label);
    WaitDlg(int title, const std::string& label, std::function<bool()> checkIfShouldContinue);
    virtual ~WaitDlg();

    void addSubLabel(const std::string& subLabel, int max);

    std::function<void()> onDone;
protected:
    virtual void run();

private:
    std::string label;
    std::deque<std::string> subLabels;
    std::function<bool()> checkIfShouldContinue;
};

#endif