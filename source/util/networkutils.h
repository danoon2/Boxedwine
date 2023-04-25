#ifndef __NETWORK_UTILS_H__
#define __NETWORK_UTILS_H__

class NetworkProxy {
public:
	std::string host;
	U32 port;
	std::string username;
	std::string password;
};

bool downloadFile(const std::string& url, const std::string& filePath, std::function<void(U64 bytesCompleted)> f, NetworkProxy* proxy, std::string& errorMsg, bool* cancel=NULL);

#endif