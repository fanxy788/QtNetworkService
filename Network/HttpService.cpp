#include "HttpService.h"
#include <QUuid>
#include <QJsonObject>
#include <QJsonDocument>

HttpService::HttpService()
{
    connect(&m_networkAccessManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(onResponse(QNetworkReply*)));
}

HttpService::~HttpService()
{
    m_networkAccessManager.disconnect();
}

HttpServiceMethod HttpService::get(const QString url)
{
    HttpServiceMethod httpServiceMethod(QNetworkAccessManager::GetOperation, this);
    return httpServiceMethod.url(url);
}

HttpServiceMethod HttpService::post(const QString url)
{
    HttpServiceMethod httpServiceMethod(QNetworkAccessManager::PostOperation, this);
    return httpServiceMethod.url(url);
}

bool HttpService::sendRequest(QNetworkAccessManager::Operation op, QNetworkRequest &request, QVariant data, const QObject *respReceiver, const char *slot)
{
    Q_ASSERT(respReceiver);
    Q_ASSERT(slot);

    QString requestId = "Request-" + QUuid::createUuid().toString();
    request.setAttribute(QNetworkRequest::User, requestId);

    HttpRequest* requestForResp = new HttpRequest(this);
    requestForResp->setObjectName(requestId);

    connect(requestForResp, SIGNAL(finished(QVariant)), respReceiver, slot);

    if (op == QNetworkAccessManager::GetOperation) {
        m_networkAccessManager.get(request);
    }
    else if (op == QNetworkAccessManager::PostOperation) {
        m_networkAccessManager.post(request, QJsonDocument(data.toJsonObject()).toBinaryData());
    }

    return true;
}

void HttpService::onResponse(QNetworkReply *reply)
{
    QString requestId = reply->request().attribute(QNetworkRequest::User).toString();

    HttpRequest *httpRequest = findChild<HttpRequest *>(requestId);
    if (httpRequest != NULL) {
        httpRequest->onResponse(QVariant::fromValue(reply));
        httpRequest->deleteLater();
    }

    reply->deleteLater();
}
