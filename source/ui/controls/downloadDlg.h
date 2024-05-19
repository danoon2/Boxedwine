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
    DownloadDlg(Msg title, const std::vector<DownloadItem>& items, std::function<void(bool)> onCompleted);
    virtual ~DownloadDlg();

protected:
    // from BaseDlg
    void run() override;

private:
    void downloadCompleted();
    void downloadFailed(BString errorMsg);
    void showErrorMsg(bool open);

    std::vector<DownloadItem> items;
    BString errorMsg;
    U32 percentDone = 0;
    std::function<void(bool)> onCompleted;
    bool cancelled = false;
    bool downloadDone = false;
    BString currentLabel;
    bool hasSize = false;
};

#endif