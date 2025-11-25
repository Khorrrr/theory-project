#include "MLTranslationBridge.h"
#include <QNetworkReply>
#include <QJsonParseError>
#include <QEventLoop>
#include <QTimer>
#include <QUrlQuery>

MLTranslationBridge::MLTranslationBridge(QObject *parent)
    : QObject(parent)
    , pythonServerUrl("http://localhost:5000")
    , networkManager(new QNetworkAccessManager(this))
    , isServerRunning(false)
    , requestTimeout(30000) // 30 seconds
{
}

MLTranslationBridge::~MLTranslationBridge() {
    delete networkManager;
}

void MLTranslationBridge::setServerUrl(const QString& url) {
    pythonServerUrl = url;
}

bool MLTranslationBridge::checkServerHealth() {
    QNetworkRequest request(QUrl(pythonServerUrl + "/health"));
    request.setRawHeader("Content-Type", "application/json");

    QNetworkReply* reply = networkManager->get(request);

    // Create event loop for synchronous request
    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);
    timer.setInterval(5000); // 5 second timeout for health check

    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);

    timer.start();
    loop.exec();

    bool isHealthy = false;
    if (timer.isActive() && reply->error() == QNetworkReply::NoError) {
        QByteArray responseData = reply->readAll();
        QJsonParseError parseError;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData, &parseError);

        if (parseError.error == QJsonParseError::NoError) {
            QJsonObject jsonObj = jsonDoc.object();
            isHealthy = jsonObj.value("status").toString() == "healthy";
        }
    }

    reply->deleteLater();
    isServerRunning = isHealthy;
    return isHealthy;
}

void MLTranslationBridge::translateCode(const QString& sourceCode,
                                       const QString& targetLanguage,
                                       const QVector<Token>& tokens) {
    showTranslationStatus("Checking ML server availability...");

    // Check if server is running first
    if (!checkServerHealth()) {
        emit translationError("ML server is not running. Please start the Python ML server.");
        return;
    }

    showTranslationStatus("Preparing translation request...");

    // Prepare request data
    QJsonObject requestData;
    requestData["source_code"] = sourceCode;
    requestData["target_language"] = targetLanguageToCode(targetLanguage);

    // Convert tokens to JSON array (optional context for ML model)
    QJsonArray tokenArray;
    for (const auto& token : tokens) {
        QJsonObject tokenObj;
        tokenObj["type"] = QString::fromStdString(token.typeToString());
        tokenObj["value"] = token.value;
        tokenObj["line"] = token.line;
        tokenObj["column"] = token.column;
        tokenArray.append(tokenObj);
    }
    requestData["tokens"] = tokenArray;

    QJsonDocument jsonDoc(requestData);

    // Create network request
    QNetworkRequest request(QUrl(pythonServerUrl + "/translate"));
    request.setRawHeader("Content-Type", "application/json");
    request.setRawHeader("Accept", "application/json");

    showTranslationStatus("Sending code to ML model...");

    // Send POST request
    QNetworkReply* reply = networkManager->post(request, jsonDoc.toJson());

    // Handle the response asynchronously
    connect(reply, &QNetworkReply::finished, this, &MLTranslationBridge::onNetworkReplyFinished);
    connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error),
            this, &MLTranslationBridge::onNetworkError);

    // Set up timeout
    QTimer* timeoutTimer = new QTimer(this);
    timeoutTimer->setSingleShot(true);
    timeoutTimer->setInterval(requestTimeout);

    connect(timeoutTimer, &QTimer::timeout, [reply, this]() {
        if (!reply->isFinished()) {
            reply->abort();
            emit translationError("Translation request timed out (30 seconds). Please try again.");
        }
    });

    timeoutTimer->start();
}

void MLTranslationBridge::onNetworkReplyFinished() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;

    // Clean up timeout timer if it exists
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        emit translationError(QString("Network error: %1").arg(reply->errorString()));
        return;
    }

    showTranslationStatus("Processing ML translation result...");

    QByteArray responseData = reply->readAll();
    QJsonParseError parseError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        emit translationError(QString("Invalid JSON response from ML server: %1").arg(parseError.errorString()));
        return;
    }

    QJsonObject jsonObj = jsonDoc.object();

    // Check for error response
    if (jsonObj.contains("error")) {
        QString errorMsg = jsonObj.value("error").toString();
        QString details = jsonObj.value("details").toString();
        if (!details.isEmpty()) {
            errorMsg += QString(": %1").arg(details);
        }
        emit translationError(QString("ML translation failed: %1").arg(errorMsg));
        return;
    }

    // Extract translated code
    if (!jsonObj.contains("translated_code")) {
        emit translationError("Invalid response format: missing translated_code field");
        return;
    }

    QString translatedCode = jsonObj.value("translated_code").toString();

    // Postprocess the result
    QString finalCode = postprocessResult(translatedCode);

    showTranslationStatus("ML translation completed successfully");
    emit translationCompleted(finalCode);
}

void MLTranslationBridge::onNetworkError(QNetworkReply::NetworkError error) {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;

    QString errorMsg;
    switch (error) {
    case QNetworkReply::ConnectionRefusedError:
        errorMsg = "Connection refused - ML server may not be running";
        break;
    case QNetworkReply::TimeoutError:
        errorMsg = "Request timeout - ML server took too long to respond";
        break;
    case QNetworkReply::HostNotFoundError:
        errorMsg = "ML server not found at specified address";
        break;
    default:
        errorMsg = QString("Network error: %1").arg(reply->errorString());
        break;
    }

    emit translationError(errorMsg);
}

QString MLTranslationBridge::preprocessCode(const QString& sourceCode, const QVector<Token>& tokens) {
    // Basic preprocessing - remove excessive whitespace, normalize formatting
    QString processed = sourceCode.trimmed();

    // Replace multiple consecutive newlines with single newline
    processed = processed.replace(QRegularExpression("\\n\\s*\\n\\s*\\n+"), "\n\n");

    return processed;
}

QString MLTranslationBridge::postprocessResult(const QString& mlResult) {
    QString result = mlResult;

    // Remove common ML model artifacts
    result = result.remove("```python").remove("```java").remove("```javascript");
    result = result.remove("```");

    // Clean up excessive whitespace
    result = result.trimmed();

    // Ensure proper formatting
    if (!result.endsWith("\n") && !result.isEmpty()) {
        result += "\n";
    }

    return result;
}

QString MLTranslationBridge::tokensToJson(const QVector<Token>& tokens) {
    QJsonArray tokenArray;
    for (const auto& token : tokens) {
        QJsonObject tokenObj;
        tokenObj["type"] = QString::fromStdString(token.typeToString());
        tokenObj["value"] = token.value;
        tokenObj["line"] = token.line;
        tokenObj["column"] = token.column;
        tokenArray.append(tokenObj);
    }

    QJsonDocument doc(tokenArray);
    return doc.toJson(QJsonDocument::Compact);
}

QString MLTranslationBridge::targetLanguageToCode(const QString& targetLanguage) {
    QString lowerLang = targetLanguage.toLower();

    if (lowerLang == "python" || lowerLang == "py") {
        return "python";
    } else if (lowerLang == "java") {
        return "java";
    } else if (lowerLang == "javascript" || lowerLang == "js") {
        return "javascript";
    } else if (lowerLang == "assembly" || lowerLang == "asm") {
        return "assembly";
    }

    // Default to python if unknown
    return "python";
}

bool MLTranslationBridge::startPythonServer() {
    // This would be used to automatically start the Python server
    // Implementation depends on your specific deployment strategy
    // For now, we'll assume the server needs to be started manually
    return false;
}

void MLTranslationBridge::showTranslationStatus(const QString& message) {
    // This can be connected to status updates in the UI
    qDebug() << "ML Translation Status:" << message;
}