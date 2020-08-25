#ifndef __DOWNLOAD_DLG_H__
#define __DOWNLOAD_DLG_H__

class DownloadItem {
public:
    DownloadItem(const std::string& label, const std::string& url, const std::string& urlBackup, const std::string filePath, U64 size) : label(label), url(url), urlBackup(urlBackup), filePath(filePath), size(size) {}
    std::string label;
    std::string url;
    std::string urlBackup;
    std::string filePath;
    U64 size;
};

class DownloadDlg : public BaseDlg {
public:
    DownloadDlg(int title, const std::vector<DownloadItem>& items, std::function<void(bool)> onCompleted);
    virtual ~DownloadDlg();

protected:
    virtual void run();

private:
    void downloadCompleted();
    void downloadFailed(const std::string& errorMsg);
    void showErrorMsg(bool open);

    std::vector<DownloadItem> items;
    U32 currentItem;

    std::string errorMsg;
    U32 percentDone;
    std::function<void(bool)> onCompleted;
    bool cancelled;
    U64 socketfd;
    bool downloadDone;
    std::string currentLabel;
    bool hasSize;
};

#endif