#ifndef __DOWNLOAD_DLG_H__
#define __DOWNLOAD_DLG_H__

class DownloadDlg : public BaseDlg {
public:
    DownloadDlg(int title, const std::string& label, const std::string& url, const std::string& filePath, std::function<void(bool)> onCompleted, U64 expectedFileSize);

protected:
    virtual void run();

private:
    void downloadCompleted();
    void downloadFailed(const std::string& errorMsg);
    void showErrorMsg(bool open);

    std::string label;
    std::string url;
    std::string filePath;
    std::string errorMsg;
    U32 percentDone;
    U64 expectedFileSize;
    std::function<void(bool)> onCompleted;
};

#endif