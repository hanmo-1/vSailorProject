/**
  ******************************************************************************
  * @file    AliyunMqttClient.cpp
  * @author  Junxin Zheng
  * @version V1.0.0
  * @date    7-January-2019
  * @brief   ...
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "AliyunMqttClient.h"

/* Variables -----------------------------------------------------------------*/
/* Function ------------------------------------------------------------------*/
/*******************************************************************************
  * @brief  Construct function.
  * @param  QString producKkey   - The Product Key of Aliyun IoT Platform.
  *         QString deviceName   - The Device Name of Aliyun IoT Platform.
  *         QString deviceSecret - The Device Secret of Aliyun IoT Platform.
  *         QObject *parent      - Pointer of parent object.
  * @retval None.
  *****************************************************************************/
AliyunMqttClient::AliyunMqttClient(const QString &producKkey, const QString &deviceName, const QString &deviceSecret, QObject *parent) :
                                   QMqttClient(parent), PRODUCT_KEY(producKkey), DEVICE_NAME(deviceName), DEVICE_SECRET(deviceSecret)
{
    // Common Configurtion.
    setPort(1883);
    setKeepAlive(100);
    setCleanSession(true);

    // Signal-Slot Connection.
    connect(this, &AliyunMqttClient::stateChanged, this, &AliyunMqttClient::printClientState);

#ifndef ALIYUN_MQTT_CLIENT_NO_DEBUG
    qDebug() << "This is a Aliyun MQTT Client.";
#endif
}


/*******************************************************************************
  * @brief  Destructor function.
  * @param  None.
  * @retval None.
  *****************************************************************************/
AliyunMqttClient::~AliyunMqttClient()
{
    if(state() != Disconnected)
    {
        disconnectFromHost();
    }
}


/*******************************************************************************
  * @brief  Connect to the MQTT Host.
  * @param  None.
  * @retval None.
  *****************************************************************************/
void AliyunMqttClient::connectToHost()
{
    setProtocolVersion(MQTT_3_1_1);
    setHostname(getHostName(PRODUCT_KEY));
    setClientId(getClientId());
    setUsername(getUserName(PRODUCT_KEY, DEVICE_NAME));
    setPassword(getUserPassword(PRODUCT_KEY, DEVICE_NAME, DEVICE_SECRET));
    QMqttClient::connectToHost();
}


/*******************************************************************************
  * @brief  Print the state of MQTT Client.
  * @param  None.
  * @retval None.
  *****************************************************************************/
void AliyunMqttClient::printClientState()
{
    qDebug() << "MQTT Client State: " << state();
    qDebug() << "MQTT Client Error: " << error();
}


/*******************************************************************************
  * @brief  Get MQTT Host Name according to Product Key.
  * @param  QString producKkey - The Product Key of Aliyun IoT Platform.
  * @retval None.
  *****************************************************************************/
QString AliyunMqttClient::getHostName(const QString &productKey)
{
    return productKey + ".iot-as-mqtt.cn-shanghai.aliyuncs.com";
}


/*******************************************************************************
  * @brief  Get MQTT Client ID according to Product Key.
  * @param  QString Random(necessary).
  *         SecureMode secureMode - MQTT Connection Secure Mode.
  *             @param TLS, TCP
  *         SignMethod signMethod - MQTT Signture Method.
  *             @param Md5, Sha1, Sha256
  * @retval None.
  *****************************************************************************/
QString AliyunMqttClient::getClientId(const QString &Random, SecureMode secureMode, SignMethod signMethod)
{
    // ID (MAC Address)
    QString clientId = Random;

    // Secure Mode
    clientId.append("|securemode=").append(QString::number(secureMode));

    // Signature Method
    clientId.append(",signmethod=");
    switch(signMethod)
    {
        case Md5:    clientId.append("hmacmd5");    break;
        case Sha1:   clientId.append("hmacsha1");   break;
        case Sha256: clientId.append("hmacsha256"); break;
    }
    clientId.append("|");

    qDebug() << "Client ID: " << clientId;
    return clientId;
}


/*******************************************************************************
  * @brief  Get MQTT Host Name according to Product Key.
  * @param  QString producKkey - The Product Key of Aliyun IoT Platform.
  *         QString deviceName - The Device Name of Aliyun IoT Platform.
  * @retval None.
  *****************************************************************************/
QString AliyunMqttClient::getUserName(const QString &productKey, const QString &deviceName)
{
    return deviceName + "&" + productKey;
}


/*******************************************************************************
  * @brief  Get MQTT Host Name according to Product Key.
  * @param  QString producKkey - The Product Key of Aliyun IoT Platform.
  *         QString deviceName - The Device Name of Aliyun IoT Platform.
  *         QString deviceSecret - The Device Secret of Aliyun IoT Platform.
  *         QString Random(necessary).
  * @retval None.
  *****************************************************************************/
QString AliyunMqttClient::getUserPassword(const QString &productKey, const QString &deviceName, const QString &deviceSecret, const QString &Random)
{
    return getHash(QCryptographicHash::Sha1, "clientId" + Random + "deviceName" + deviceName + "productKey" + productKey, deviceSecret).toHex();
}


/*******************************************************************************
  * @brief  Get Mac address of this machine.
  * @param  None.
  * @retval None.
  *****************************************************************************/
QString AliyunMqttClient::getHostMac()
{
    QList<QNetworkInterface> netInterface = QNetworkInterface::allInterfaces();
    int nCnt = netInterface.count();

    QString hostMac = "";
    for(int i = 0; i < nCnt; i ++)
    {
        if(netInterface[i].flags().testFlag(QNetworkInterface::IsUp) &&
           netInterface[i].flags().testFlag(QNetworkInterface::IsRunning) &&
          !netInterface[i].flags().testFlag(QNetworkInterface::IsLoopBack))
        {
            hostMac = netInterface[i].hardwareAddress(); break;
        }
    }
    return hostMac;
}

/**************** (C) COPYRIGHT 2019 Junxin Zheng ******** END OF FILE ********/
