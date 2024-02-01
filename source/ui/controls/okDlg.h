#ifndef __OK_DLG_H__
#define __OK_DLG_H__

class OkDlg : public BaseDlg {
public:
    OkDlg(Msg title, BString label, std::function<void()> onDone, int width=400, int height=150);

protected:
    // from BaseDlg
    void run() override;

private:
    BString label;
    std::function<void()> onDone;
};

#endif