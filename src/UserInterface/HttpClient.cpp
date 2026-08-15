#include "HttpClient.h"

static size_t __WriteCallback(char *ptr, size_t size, size_t nmemb, void *userdata) {
  static_cast<std::string *>(userdata)->append(ptr, size * nmemb);
  return size * nmemb;
}

CHttpClient::CHttpClient() {
  curl_global_init(CURL_GLOBAL_DEFAULT);
  m_pMultiHandle = curl_multi_init();
}

CHttpClient::~CHttpClient() {
  curl_multi_cleanup(m_pMultiHandle);
  curl_global_cleanup();
}

void CHttpClient::__StartRequest(
  const std::string &strUrl,
  const char *szMethod,
  const std::string *pstrBody,
  const std::string &strBearerToken,
  HttpCallback callback
) {
  m_requests.emplace_back();
  SRequest &req = m_requests.back();

  req.pEasyHandle = curl_easy_init();
  req.pHeaders = curl_slist_append(nullptr, "Content-Type: application/json");
  if (!strBearerToken.empty()) {
    req.pHeaders = curl_slist_append(req.pHeaders, ("Authorization: Bearer " + strBearerToken).c_str());
  }

  curl_easy_setopt(req.pEasyHandle, CURLOPT_URL, strUrl.c_str());
  curl_easy_setopt(req.pEasyHandle, CURLOPT_HTTPHEADER, req.pHeaders);
  curl_easy_setopt(req.pEasyHandle, CURLOPT_CUSTOMREQUEST, szMethod);
  curl_easy_setopt(req.pEasyHandle, CURLOPT_WRITEFUNCTION, __WriteCallback);
  curl_easy_setopt(req.pEasyHandle, CURLOPT_WRITEDATA, &req.strResponseBuffer);
  curl_easy_setopt(req.pEasyHandle, CURLOPT_TIMEOUT_MS, 8000L);
  curl_easy_setopt(req.pEasyHandle, CURLOPT_PRIVATE, &req);

  if (pstrBody) {
    curl_easy_setopt(req.pEasyHandle, CURLOPT_COPYPOSTFIELDS, pstrBody->c_str());
  }

  req.callback = std::move(callback);
  curl_multi_add_handle(m_pMultiHandle, req.pEasyHandle);
}

void CHttpClient::Get(const std::string &strUrl, const std::string &strBearerToken, HttpCallback callback) {
  __StartRequest(strUrl, "GET", nullptr, strBearerToken, std::move(callback));
}

void CHttpClient::Post(
  const std::string &strUrl,
  const std::string &strJsonBody,
  const std::string &strBearerToken,
  HttpCallback callback
) {
  __StartRequest(strUrl, "POST", &strJsonBody, strBearerToken, std::move(callback));
}

void CHttpClient::Delete(const std::string &strUrl, const std::string &strBearerToken, HttpCallback callback) {
  __StartRequest(strUrl, "DELETE", nullptr, strBearerToken, std::move(callback));
}

void CHttpClient::Process() {
  int nRunning = 0;
  curl_multi_perform(m_pMultiHandle, &nRunning);

  int nMsgsLeft = 0;
  CURLMsg *pMsg;
  while ((pMsg = curl_multi_info_read(m_pMultiHandle, &nMsgsLeft)) != nullptr) {
    if (pMsg->msg != CURLMSG_DONE) {
      continue;
    }

    SRequest *pReq = nullptr;
    curl_easy_getinfo(pMsg->easy_handle, CURLINFO_PRIVATE, &pReq);

    long lHttpStatus = 0;
    curl_easy_getinfo(pMsg->easy_handle, CURLINFO_RESPONSE_CODE, &lHttpStatus);
    pReq->callback(pMsg->data.result == CURLE_OK, lHttpStatus, pReq->strResponseBuffer);

    curl_multi_remove_handle(m_pMultiHandle, pReq->pEasyHandle);
    curl_slist_free_all(pReq->pHeaders);
    curl_easy_cleanup(pReq->pEasyHandle);

    m_requests.remove_if([&](const SRequest &r) { return &r == pReq; });
  }
}
