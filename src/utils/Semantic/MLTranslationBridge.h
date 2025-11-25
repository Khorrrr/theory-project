#ifndef MLTRANSLATIONBRIDGE_H
#define MLTRANSLATIONBRIDGE_H

#include <QObject>
#include <QString>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTimer>
#include <QEventLoop>
#include <QDebug>
#include "../LexicalAnalysis/Token.h"

class MLTranslationBridge : public QObject {
    Q_OBJECT

private:
    QString pythonServerUrl;
    QNetworkAccessManager* networkManager;
    bool isServerRunning;
    int requestTimeout;

public:
    explicit MLTranslationBridge(QObject *parent = nullptr);
    ~MLTranslationBridge();

    void setServerUrl(const QString& url);
    bool checkServerHealth();
    void translateCode(const QString& sourceCode,
                      const QString& targetLanguage,
                      const QVector<Token>& tokens);

signals:
    void translationCompleted(const QString& translatedCode);
    void translationError(const QString& error);

private slots:
    void onNetworkReplyFinished();
    void onNetworkError(QNetworkReply::NetworkError error);

private:
    QString preprocessCode(const QString& sourceCode, const QVector<Token>& tokens);
    QString postprocessResult(const QString& mlResult);
    QString tokensToJson(const QVector<Token>& tokens);
    QString targetLanguageToCode(const QString& targetLanguage);
    bool startPythonServer();
    void showTranslationStatus(const QString& message);
};

#endif // MLTRANSLATIONBRIDGE_H