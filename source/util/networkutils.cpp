#if !defined (__EMSCRIPTEN__) && !defined (__TEST)
#include "Poco/URIStreamOpener.h"
#include "Poco/StreamCopier.h"
#include "Poco/Path.h"
#include "Poco/URI.h"
#include "Poco/Exception.h"
#include "Poco/Net/HTTPStreamFactory.h"
#include "Poco/Net/HTTPSStreamFactory.h"
#include "Poco/Net/FTPStreamFactory.h"
#include "Poco/Net/AcceptCertificateHandler.h"
#include "Poco/Net/SSLManager.h"
#include "Poco/Net/Context.h"

#ifdef U64
#undef U64
#endif

#include "boxedwine.h"
#include <iostream>
#include <fstream>
#include "networkutils.h"

using Poco::URIStreamOpener;
using Poco::StreamCopier;
using Poco::Path;
using Poco::URI;
using Poco::Exception;
using Poco::Net::HTTPStreamFactory;
using Poco::Net::HTTPSStreamFactory;
using Poco::Net::FTPStreamFactory;
using Poco::Net::AcceptCertificateHandler;
using Poco::Net::SSLManager;
using Poco::Net::Context;

static bool sslInitialized;

bool downloadFile(const std::string& url, const std::string& filePath, std::function<void(U64 bytesCompleted)> f, NetworkProxy* proxy, std::string& errorMsg) {
    try
    {
        std::string complete_page_url = "";
        std::ofstream file_stream;
        std::unique_ptr<std::istream> pStr = nullptr;

        // Create the URI from the URL to the file.
        URI uri(url);

        //std::auto_ptr<std::istream>pStr(URIStreamOpener::defaultOpener().open(uri);
        //StreamCopier::copyStream(*pStr.get(), std::cout);

        if (stringStartsWith(url, "https", true)) {
            std::unique_ptr<HTTPSStreamFactory> https_stream_factory = nullptr;

            if (!sslInitialized) {
                sslInitialized = true;
                Poco::Net::initializeSSL();

                static Poco::SharedPtr<AcceptCertificateHandler> pCertHandler;
                pCertHandler = new AcceptCertificateHandler(false); // ask the user via console
                Context::Ptr pContext = new Context(Context::CLIENT_USE, "");
                SSLManager::instance().initializeClient(0, pCertHandler, pContext);
            }
            if (proxy) {
                https_stream_factory = std::unique_ptr<HTTPSStreamFactory>(new HTTPSStreamFactory(proxy->host, proxy->port, proxy->username, proxy->password));
            } else {
                https_stream_factory = std::unique_ptr<HTTPSStreamFactory>(new HTTPSStreamFactory());
            }

            if (https_stream_factory) {
                pStr = std::unique_ptr<std::istream>(https_stream_factory->open(uri));
            }
        } else
        {
            std::unique_ptr<HTTPStreamFactory> http_stream_factory = nullptr;

            if (proxy) {
                http_stream_factory = std::unique_ptr<HTTPStreamFactory>(new HTTPStreamFactory(proxy->host, proxy->port, proxy->username, proxy->password));
            } else {
                http_stream_factory = std::unique_ptr<HTTPStreamFactory>(new HTTPStreamFactory());
            }

            if (http_stream_factory) {
                pStr = std::unique_ptr<std::istream>(http_stream_factory->open(uri));
            }
        }

        // :TODO: why can't I get the length from pStr?
        if (pStr) {

            file_stream.open(filePath, std::ios::out | std::ios::trunc | std::ios::binary);
            //StreamCopier::copyStream(*pStr.get(), file_stream);

            std::istream& is = *pStr.get();

            Poco::Buffer<char> buffer(8192);
            std::streamsize len = 0;
            is.read(buffer.begin(), 8192);
            std::streamsize n = is.gcount();
            while (n > 0)
            {
                len += n;
                file_stream.write(buffer.begin(), n);
                f(len);
                if (is && file_stream)
                {
                    is.read(buffer.begin(), 8192);
                    n = is.gcount();
                } else {
                    n = 0;
                }
            }

            file_stream.close();
            return true;
        }
        return false;
    } catch (Exception& exc) {
        klog("HttpClient:: Exception in DownloadFile , error code: %d", exc.code());
        errorMsg = exc.displayText();
    }

    return false;
}
#endif
