#ifndef __UNZIP_DLG_H__
#define __UNZIP_DLG_H__

class UnzipDlg : public BaseDlg {
public:
    UnzipDlg(Msg title, BString label, BString zipFilePath, BString destDirPath, std::function<void(bool)> onCompleted);
    virtual ~UnzipDlg();

protected:
    // from BaseDlg
    void run() override;

private:
    void unzipCompleted();
    void unzipFailed(BString errorMsg);

    BString label;
    BString currentFile;
    U32 percentDone = 0;
    std::function<void(bool)> onCompleted;
};

#endif