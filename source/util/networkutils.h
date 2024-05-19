#ifndef __NETWORK_UTILS_H__
#define __NETWORK_UTILS_H__

class NetworkProxy {
public:
	BString host;
	U32 port;
	BString username;
	BString password;
};

bool downloadFile(BString url, BString filePath, std::function<void(U64 bytesCompleted)> f, NetworkProxy* proxy, BString& errorMsg, bool* cancel=nullptr);

#endif