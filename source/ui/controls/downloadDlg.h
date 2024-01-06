#ifndef __DOWNLOAD_DLG_H__
#define __DOWNLOAD_DLG_H__

class DownloadItem {
public:
    DownloadItem(BString label, BString url, BString urlBackup, const BString filePath, U64 size) : label(label), url(url), urlBackup(urlBackup), filePath(filePath), size(size) {}
    BString label;
    BString url;
    BString urlBackup;
    BString filePath;
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
    void downloadFailed(BString errorMsg);
    void showErrorMsg(bool open);

    std::vector<DownloadItem> items;
    U32 currentItem;

    BString errorMsg;
    U32 percentDone;
    std::function<void(bool)> onCompleted;
    bool cancelled;
    bool downloadDone;
    BString currentLabel;
    bool hasSize;
};

#endif