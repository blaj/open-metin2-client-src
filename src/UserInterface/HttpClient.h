#ifndef HTTPCLIENT_H
#define HTTPCLIENT_H

#include "EterBase/Singleton.h"
#include <curl/curl.h>
#include <functional>
#include <list>
#include <string>

using HttpCallback = std::function<void(bool bSuccess, long lStatusCode, const std::string &strBody)>;

class CHttpClient : public CSingleton<CHttpClient> {
public:
  CHttpClient();
  ~CHttpClient();

  void Get(const std::string &strUrl, const std::string &strBearerToken, HttpCallback callback);

  void Post(
    const std::string &strUrl,
    const std::string &strJsonBody,
    const std::string &strBearerToken,
    HttpCallback callback
  );

  void Delete(const std::string &strUrl, const std::string &strBearerToken, HttpCallback callback);

  void Process();

private:
  struct SRequest {
    CURL *pEasyHandle;
    curl_slist *pHeaders;
    std::string strResponseBuffer;
    HttpCallback callback;
  };

  void __StartRequest(
    const std::string &strUrl,
    const char *szMethod,
    const std::string *pstrBody,
    const std::string &strBearerToken,
    HttpCallback callback
  );

  CURLM *m_pMultiHandle;
  std::list<SRequest> m_requests;
};

#endif
