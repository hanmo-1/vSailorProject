/**
  ******************************************************************************
  * @file    AliyunMqttClient.h
  * @author  Junxin Zheng
  * @version V1.0.0
  * @date    7-January-2019
  * @brief   ...
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef ALIYUNMQTTCLIENT_H
#define ALIYUNMQTTCLIENT_H

/* Includes ------------------------------------------------------------------*/
#include <QDebug>
#include <QObject>
#include <QtNetwork>
#include <QtMqtt/qmqttclient.h>
#include "DateUtil.h"
#include "Authorization.h"

/* Macro Definition ----------------------------------------------------------*/
//#define ALIYUN_MQTT_CLIENT_NO_DEBUG

/* Variables -----------------------------------------------------------------*/
/* Function Declaration ------------------------------------------------------*/
/* Class Declaration ---------------------------------------------------------*/
class AliyunMqttClient : public QMqttClient
{
    Q_OBJECT
public:
    enum SecureMode
    {
        TLS = 2,
        TCP
    };

    enum SignMethod
    {
        Md5,
        Sha1,
        Sha256
    };

    explicit AliyunMqttClient(const QString &productKey,   const QString &deviceName,
                              const QString &deviceSecret, QObject *parent = nullptr);

    ~AliyunMqttClient();
    static QString getClientId(const QString &Random = getHostMac(), SecureMode secureMode = TCP, SignMethod signMethod = Sha1);
    static QString getHostName(const QString &productKey);
    static QString getUserName(const QString &productKey, const QString &deviceName);
    static QString getUserPassword(const QString &productKey, const QString &deviceName, const QString &deviceSecret, const QString &Random = getHostMac());

    void connectToHost();

private:
    static QString getHostMac();
    const  QString PRODUCT_KEY;
    const  QString DEVICE_NAME;
    const  QString DEVICE_SECRET;

signals:

public slots:
    void printClientState();
};

#endif // ALIYUNMQTTCLIENT_H

/**************** (C) COPYRIGHT 2019 Junxin Zheng ******** END OF FILE ********/
