#ifndef __UNZIP_DLG_H__
#define __UNZIP_DLG_H__

class UnzipDlg : public BaseDlg {
public:
    UnzipDlg(int title, const std::string& label, const std::string& zipFilePath, const std::string& destDirPath, std::function<void(bool)> onCompleted);
    virtual ~UnzipDlg();

protected:
    virtual void run();

private:
    void unzipCompleted();
    void unzipFailed(const std::string& errorMsg);

    std::string label;
    std::string currentFile;
    U32 percentDone;
    std::function<void(bool)> onCompleted;
};

#endif