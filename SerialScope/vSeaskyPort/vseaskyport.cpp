#include "vseaskyport.h"
#include <QDateTime>


#include "./json/QJson.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

//使用前务必给的相关参数地址信息
const qint32 rxnamewidth = 64;
const qint32 rxunitwidth = 64;
const qint32 txnamewidth = 64;
const qint32 txunitwidth = 64;
const qint32 floatnum    = 6;  //小数点位数
vSeaskyPort::vSeaskyPort(QWidget *parent) : QObject(parent)
{
    vQTimerTxStop();
    connect(&this->vQTimerTx,&QTimer::timeout,
            this,&vSeaskyPort::vSeaskyTxSlot);
}


//串口连接的，其中串口接收到数据触发的槽函数，在槽函数中解析串口的数据
void vSeaskyPort::vConnectRx(void)
{
    void(vSeaskyPort:: * rxSlot)(void) =
            &vSeaskyPort::vSeaskyRxIRQ;
    connect(this->vSerial->qSerial,&QSerialPort::readyRead,
            this,rxSlot,Qt::QueuedConnection);  //串口准备好了可以进行数据的接收和协议处理
}
void vSeaskyPort::vDisConnectRx(void)
{
    void(vSeaskyPort:: * rxSlot)(void) = &vSeaskyPort::vSeaskyRxIRQ;
    disconnect(this->vSerial->qSerial,&QSerialPort::readyRead,
            this,rxSlot);
}
void vSeaskyPort::vConnectTx(void)
{
    connect(this,&vSeaskyPort::vSerialTx,
            this->vSerial,&vSerialPort::vWriteData
            ,Qt::QueuedConnection);
}
void vSeaskyPort::vDisConnectTx(void)
{
    disconnect(this,&vSeaskyPort::vSerialTx,
               this->vSerial,&vSerialPort::vWriteData);
}

//阿里云中的mqtt协议连接
void vSeaskyPort::vConnectAliyun()
{
    qDebug() << "运行vConnectAliyun函数";

    void(vSeaskyPort:: * rxSlot)(const QByteArray &message, const QMqttTopicName &topic) =
            &vSeaskyPort::vSeaskyAliyunIRQ;
    connect(this->client, &QMqttClient::messageReceived,
            this, rxSlot, Qt::QueuedConnection);
   qDebug() << "运行vConnectAliyun函数over";
}

void vSeaskyPort::vDisConnectAliyun()
{
    void(vSeaskyPort:: * rxSlot)(const QByteArray &message, const QMqttTopicName &topic) =
            &vSeaskyPort::vSeaskyAliyunIRQ;
    disconnect(this->client, &QMqttClient::messageReceived,
            this, rxSlot);
}



void vSeaskyPort::setQWidgetAddr(QWidget * addrTx,QWidget * addrRx)
{
   vTxEdit = addrTx;
   vRxEdit = addrRx;
}
void vSeaskyPort::setRxSeaskyAddr(QString * strF,QString * strN,QString * strU,float * addrF)
{
    this->vRxSeasky.vQString = strF;
    this->vRxSeasky.vName = strN;
    this->vRxSeasky.vUnit = strU;
    this->vRxSeasky.vFloat = addrF;
}
void vSeaskyPort::setTxSeaskyAddr(QString * strF,QString * strN,QString * strU,float * addrF)
{
    this->vTxSeasky.vQString = strF;
    this->vTxSeasky.vName = strN;
    this->vTxSeasky.vUnit = strU;
    this->vTxSeasky.vFloat = addrF;
}

//设置通信协议中的接受窗口的的设置 包括接收lineEdit的修改
void vSeaskyPort::configQWidgetRx(qint32 num)
{
    this->vRxNum = num;
    this->vRxNumUTF8 = num*4+10;
    QVBoxLayout * vQVBoxLayout = new QVBoxLayout(this->vRxEdit);
    QLineEdit   * vlineEdit1;
    QLineEdit   * vlineEdit2;
    QLineEdit   * vlineEdit3;
    for(qint16 i=0;i<this->vRxNum;i++)
    {
        QWidget     * vQWidget = new QWidget(this->vRxEdit);
        QHBoxLayout * vQHBoxLayout = new QHBoxLayout(vQWidget);
        vQHBoxLayout->setContentsMargins(0,0,0,0);
        vlineEdit1 = new QLineEdit(vQWidget);
        vlineEdit2 = new QLineEdit(vQWidget);
        vlineEdit3 = new QLineEdit(vQWidget);
        vlineEdit1->setMinimumSize(rxnamewidth,24);
        vlineEdit1->setMaximumSize(rxnamewidth,24);
        vlineEdit2->setMinimumSize(0,24);
        vlineEdit2->setMaximumSize(16777215,24);
        vlineEdit2->setAlignment(Qt::AlignLeft);
        vlineEdit2->setReadOnly(true);
        vlineEdit2->setValidator(new QDoubleValidator(vlineEdit2));
        vlineEdit3->setMinimumSize(rxunitwidth,24);
        vlineEdit3->setMaximumSize(rxunitwidth,24);
        vlineEdit1->setStyleSheet("background-color:#FFFFFF;");
        vlineEdit2->setStyleSheet("background-color:#CCFFFF;"
                                  "color:#FF0033;"
                                  "font-weight: bold;");
        vlineEdit3->setStyleSheet("background-color:#FFFFFF;");
        vQHBoxLayout->addWidget(vlineEdit1);
        vQHBoxLayout->addWidget(vlineEdit2);
        vQHBoxLayout->addWidget(vlineEdit3);
        vQWidget->setLayout(vQHBoxLayout);
        vQVBoxLayout->addWidget(vQWidget);
        //数据显示更新
        connect(&vQTimer,&QTimer::timeout,[=]()
        {
            //默认显示六位有效数字 数据放在vRxSeasky.vQString[i]中，定时到了就进行刷新显示（刷新速度比数据接受的速度要快）
            this->vRxSeasky.vQString[i] = QString::number((this->vRxSeasky.vFloat[i]),'f',floatnum);
            this->vRxSeasky.vQString[i].remove(QRegExp("0*$"));
            this->vRxSeasky.vQString[i].remove(QRegExp("[.]$"));
            vlineEdit2->setText(this->vRxSeasky.vQString[i]);  //将数据进行显示用Qstring类型进行显示
        });
        //编辑接受数据的参数名字（参数名字可以自己更改）
        connect(vlineEdit1,&QLineEdit::editingFinished,[=]()
        {
            this->vRxSeasky.vName[i] = vlineEdit1->text();
            emit vInfoChanged();
        });
        //编辑参数的单位
        connect(vlineEdit3,&QLineEdit::editingFinished,[=]()
        {
            this->vRxSeasky.vUnit[i] = vlineEdit3->text();
            emit vInfoChanged();
        });
        //最后显示三个信息，对于设置好后显示
        connect(this,&vSeaskyPort::vQWidgetRxShow,[=]()
        {
            vlineEdit1->setText(this->vRxSeasky.vName[i]);
            vlineEdit2->setText(this->vRxSeasky.vQString[i]);
            vlineEdit3->setText(this->vRxSeasky.vUnit[i]);
        });
    }
}
void vSeaskyPort::configQWidgetTx(qint32 num)
{
    this->vTxNum     = num;
    this->vTxNumUTF8 = num*4+10;
    QVBoxLayout * vQVBoxLayout = new QVBoxLayout(this->vTxEdit);
    QLineEdit   * vlineEdit1;
    QLineEdit   * vlineEdit2;
    QLineEdit   * vlineEdit3;
    for(qint16 i=0;i<this->vTxNum;i++)
    {
        QWidget     * vQWidget = new QWidget(this->vTxEdit);
        QHBoxLayout * vQHBoxLayout = new QHBoxLayout(vQWidget);
        vQHBoxLayout->setContentsMargins(0,0,0,0);
        vlineEdit1 = new QLineEdit(vQWidget);
        vlineEdit2 = new QLineEdit(vQWidget);
        vlineEdit3 = new QLineEdit(vQWidget);
        vlineEdit1->setMinimumSize(txnamewidth,24);
        vlineEdit1->setMaximumSize(txnamewidth,24);
        vlineEdit2->setMinimumSize(0,24);
        vlineEdit2->setMaximumSize(16777215,24);
        QRegExp txFloat("^(-?[0]|-?[1-9][0-9]{1,6})(?:\\.\\d{1,6})?$|(^\\t?$)");
        vlineEdit2->setValidator(new QRegExpValidator(txFloat,vlineEdit2));
        vlineEdit3->setMinimumSize(txunitwidth,24);
        vlineEdit3->setMaximumSize(txunitwidth,24);
        vlineEdit1->setStyleSheet("background-color:#FFFFFF;");
        vlineEdit2->setStyleSheet("background-color:#FFFFFF;");
        vlineEdit3->setStyleSheet("background-color:#FFFFFF;");

        vQHBoxLayout->addWidget(vlineEdit1);
        vQHBoxLayout->addWidget(vlineEdit2);
        vQHBoxLayout->addWidget(vlineEdit3);
        vQWidget->setLayout(vQHBoxLayout);
        vQVBoxLayout->addWidget(vQWidget);
        connect(vlineEdit1,&QLineEdit::editingFinished,[=]()
        {
            this->vTxSeasky.vName[i] = vlineEdit1->text();
            emit vInfoChanged();
        });
        connect(vlineEdit2,&QLineEdit::editingFinished,[=]()
        {
            this->vTxSeasky.vQString[i] = vlineEdit2->text();
            this->vTxSeasky.vFloat[i] = this->vTxSeasky.vQString[i].toDouble();
            emit vInfoChanged();
        });
        connect(vlineEdit3,&QLineEdit::editingFinished,[=]()
        {
            this->vTxSeasky.vUnit[i] = vlineEdit3->text();
            emit vInfoChanged();
        });
        connect(this,&vSeaskyPort::vQWidgetTxShow,[=]()
        {
            vlineEdit1->setText(this->vTxSeasky.vName[i]);
            vlineEdit2->setText(this->vTxSeasky.vQString[i]);
            vlineEdit3->setText(this->vTxSeasky.vUnit[i]);
        });
    }
}
void vSeaskyPort::timerStart(void)
{
    vQTimerEnable = true;
    if(!this->vQTimer.isActive())
    {
        this->vQTimer.start(this->timerCntSet);
    }
}
void vSeaskyPort::timerStop(void)
{
    vQTimerEnable = false;
    if(this->vQTimer.isActive())
    {
         this->vQTimer.stop();
    }
}

//由于追求极高的解析效率，嵌入式端尽可能打包发送，异常断帧情况将极大的影响程序效率，甚至于造成程序崩溃
void vSeaskyPort::vSeaskyRxIRQ(void)
{
    QByteArray vRxSerialBuff;
    if(this->vSerial!=nullptr)
    {
        if(this->vSerial->qSerial->isOpen())
        {
            vRxSerialBuff = this->vSerial->qSerial->readAll();
        }
        if(vRxSerialBuff.isEmpty())
        {
            return;
        }
        //检验数据帧头,帧头固定为(0XA5),同时务必确认帧头与上一帧数据有时差，协议容错
        if((!vRxSerialBuff.isEmpty())&&
            (vRxSerialBuff.at(0)==char(0XA5)))
        {
            vRxBuff = vRxSerialBuff;    //检验好后放到vRxBuff中
        }
        else if((!vRxBuff.isEmpty())&&
                 (vRxBuff.at(0)==char(0XA5)))
        {
            vRxBuff.append(vRxSerialBuff);
        }
        else
        {
            return;
        }
        //数据帧协议解析，以及判断，需要先具备10个字符，即包含完整的帧头，帧尾数据
        if((vRxBuff.at(0)==char(0XA5))
             &&(vRxBuff.length()>=10))
        {
            vHeadCheck();
            //协议处理，如果现有数据大于数据帧长度
            if(thisLength<=vRxBuff.length())//如果现在达到了读取数据量
            {
                if( this->vProtocol.get_protocol_info(    //得到相关的信息
                    this->vProtocol.rx_info.utf8_data,
                    &rx_pos,
                    &this->vProtocol.rx_info.flags_register,
                    this->vProtocol.rx_info.data))
                {
                    //删除已使用
                    vRxBuff.remove(0,thisLength);
                    //获取接收数据的时间戳
                    QString timeString;
                    timeString = QDateTime::currentDateTime().toString("[yyyy-MM-dd hh:mm:ss.zzz]\n");
                    /*获取CMDID*/ //数据是从协议中得到的在rx_pro相关的数据中
                    this->vProtocol.rx_info.cmd_id =
                    this->vProtocol.rx_pro.cmd_id;
                    /*获取数据长度*/
                    this->vProtocol.rx_info.float_len =
                    (this->vProtocol.rx_pro.header.data_length-2)/4;
                    /*获取字符长度*/
                    this->vProtocol.rx_info.utf8_data_len =
                    this->vProtocol.rx_date_length;
                    /*加入显示*/
                    vUpdateShowBuff(timeString);  //文档显示
                    /*加入示波器*/
                    this->vRxdata.clear();
                    for(qint8 i=0;i<this->vProtocol.rx_info.float_len;i++)
                    {
                       this->vRxdata.append(this->vProtocol.rx_info.data[i]);
                    }
                    ShowQVariant.setValue(this->vRxdata);    //这里是数据的上传
                    emit RxScope(ShowQVariant);
                    //待接收长度清零
                    thisLength = 0;
                    //如果有数据未处理
                    if(vRxBuff.length()>=10)
                    {
                        vSeaskyRxIRQ(vRxBuff);
                    }
                }
                else
                {
                    //解析失败判断
                    thisLength = 0;
                    //删除已使用
                    vRxBuff.remove(0,thisLength);
                    //如果有数据未处理
                    if(!vRxBuff.isEmpty())
                    {
                        //直接当新数据处理
                        vSeaskyRxIRQ(vRxBuff);
                    }
                }
            }
            return;
        }
    }
}

//由于追求极高的解析效率，嵌入式端尽可能打包发送，异常断帧情况将极大的影响程序效率，甚至于造成程序崩溃
void vSeaskyPort::vSeaskyRxIRQ(const QByteArray &str)
{
    //检验数据帧头,帧头固定为(0XA5),同时务必确认帧头与上一帧数据有时差
    if((str.at(0)==char(0XA5)))
    {
        vRxBuff = str;
    }
    else if(vRxBuff.at(0)==char(0XA5))
    {
        vRxBuff.append(str);
    }
    //数据帧协议解析，以及判断，需要先具备10个字符，即包含完整的帧头，帧尾数据
    if((vRxBuff.at(0)==char(0XA5))
         &&(vRxBuff.length()>=10))
    {
        vHeadCheck();
        //协议处理，如果现有数据大于数据帧长度
        if(thisLength<=vRxBuff.length())//如果现在达到了读取数据量
        {
            if(this->vProtocol.get_protocol_info(
               this->vProtocol.rx_info.utf8_data,
               &rx_pos,
               &this->vProtocol.rx_info.flags_register,
               this->vProtocol.rx_info.data))
            {
                //删除已使用
                vRxBuff.remove(0,thisLength);
                //获取接收数据的时间戳
                QString timeString;
                timeString = QDateTime::currentDateTime().toString("[yyyy-MM-dd hh:mm:ss.zzz]\n");
                /*获取CMDID*/
                this->vProtocol.rx_info.cmd_id =
                this->vProtocol.rx_pro.cmd_id;
                /*获取数据长度*/
                this->vProtocol.rx_info.float_len =
                (this->vProtocol.rx_pro.header.data_length-2)/4;
                /*获取字符长度*/
                this->vProtocol.rx_info.utf8_data_len =
                this->vProtocol.rx_date_length;
                /*加入显示*/
                vUpdateShowBuff(timeString);//文档显示和协议的头数据显示
                /*加入示波器*/
                this->vRxdata.clear();
                for(qint8 i=0;i<this->vProtocol.rx_info.float_len;i++)
                {
                   this->vRxdata.append(this->vProtocol.rx_info.data[i]);
                }
                ShowQVariant.setValue(this->vRxdata);
                //待接收长度清零
                thisLength = 0;
                //如果有数据未处理
                if(vRxBuff.length()>=10)
                {
                    vSeaskyRxIRQ(vRxBuff);
                }
            }
            else
            {
                //解析失败判断
                thisLength = 0;
                //删除已使用
                vRxBuff.remove(0,thisLength);
                //如果有数据未处理
                if(!vRxBuff.isEmpty())
                {
                    //直接当新数据处理
                    vSeaskyRxIRQ(vRxBuff);
                }
            }
        }
        return;
    }
}


void vSeaskyPort::vHeadCheck(void)
{
    /************此部分用于将数据连接起来************/
    this->vProtocol.rx_info.utf8_data
           = (uint8_t*)(vRxBuff.data());
    //判断帧头是否正确，并计算出数据总长度
    rx_pos = 0;
    //如果未获取过数据长度
    if(thisLength==0)
    {
        //解析帧头协议获取数据长度
        if(this->vProtocol.get_protocol_len(
               this->vProtocol.rx_info.utf8_data,
               &rx_pos,
               &thisLength))
        {
            //如果超出本软件支持的容量,认为接收错误
            if(thisLength>=this->vRxNumUTF8)
            {
                thisLength = 0;
                vRxBuff.clear();
            }
            return;
        }
        else
        {
            //帧头校验解析失败
            thisLength = 0;
            vRxBuff.clear();
            return;
        }
    }
}
void vSeaskyPort::setPlainEdit(vPlainTextEdit * edit)
{
    static bool hexEnable = false;
    this->vPlainEdit = edit;
    this->vPlainEdit->SetShowBuffAddr(&this->vRxShow);
    this->vPlainEdit->hexEnable = &hexEnable;
    this->vPlainEdit->TimerStart();
};

//更新文档显示
void vSeaskyPort::vUpdateShowBuff(const QString &currentTimer)
{
    this->vRxShow.append(currentTimer);
    for(qint16 i=0;i<this->vProtocol.rx_info.float_len;i++)
    {
          this->vRxShow.append(QString("\t%1\t:%2\t%3\n") .
                               arg(this->vRxSeasky.vName[i]).
                               arg(this->vRxSeasky.vQString[i],12).
                               arg(this->vRxSeasky.vUnit[i]));
    }
    this->vRxSeasky.vCmdId = this->vProtocol.rx_info.cmd_id;
    this->vRxSeasky.vReg = this->vProtocol.rx_info.flags_register;
    this->vRxSeasky.vDataLen = this->vProtocol.rx_info.float_len;
    emit showRxHead();
    emit textChanged();
}
void vSeaskyPort::vQTimerTxStart(void)
{
    if(!this->vQTimerTx.isActive())
    {
        this->vQTimerTx.start(this->vQtimerTxCnt);
    }
}
void vSeaskyPort::vQTimerTxStop(void)
{
    if(this->vQTimerTx.isActive())
    {
        this->vQTimerTx.stop();
    }
}
void vSeaskyPort::vSeaskyTxSlot(void)
{
    if(this->vTxSeasky.vDataLen>this->vTxNum)
    {
        return;//超出长度，错误
    }
    if(this->vProtocol.tx_info.data!=nullptr)
    {
        this->vProtocol.tx_info.cmd_id = this->vTxSeasky.vCmdId;
        this->vProtocol.tx_info.flags_register = this->vTxSeasky.vReg;
        this->vProtocol.tx_info.float_len =
                    this->vTxSeasky.vDataLen;
        this->vProtocol.get_protocol_send_data(
                    this->vProtocol.tx_info.cmd_id,
                    this->vProtocol.tx_info.flags_register,
                    this->vProtocol.tx_info.data,
                    this->vProtocol.tx_info.float_len,
                    this->vProtocol.tx_info.utf8_data,
                    &this->vProtocol.tx_info.utf8_data_len);
        vSeaskyTxBuff=QByteArray(reinterpret_cast<const char*>(this->vProtocol.tx_info.utf8_data),
                    this->vProtocol.tx_info.utf8_data_len);
        emit vSerialTx(vSeaskyTxBuff);
    }
}

void vSeaskyPort::vSeaskyAliyunIRQ(const QByteArray &message, const QMqttTopicName &topic)
{
    qDebug() << "进入阿里云json数据解析协议处理函数";

    QByteArray vRxSerialBuff;

    const QString content = message;

    /************************此代码是使用QT自带的QJson库*********************/
    //    QJsonDocument doc = QJsonDocument::fromJson(message);
    //    if(doc.isObject())
    //    {
    //        QJsonObject obj = doc.object();
    //        QJsonValue value = *(obj.find("items")); //在obj中找出key值为item项的QJsonValue的值
    //        if(value.isObject())
    //        {
    //            QJsonObject sub_obj = value.toObject();



    //            QJsonValue humidity = *(sub_obj.find("humidity")); //在item中寻找presure03项的QJsonValue的值
    //            if(humidity.isObject())
    //            {
    //                QJsonObject sub2_obj = humidity.toObject();
    //                QJsonValue sub3_value = *(sub2_obj.find("value"));  //在presure03中寻找value的值
    //                qDebug() << sub3_value.toInt();     //调试打印取出来的值
    //            }

    //            QJsonValue temperature = *(sub_obj.find("temperature")); //在item中寻找presure03项的QJsonValue的值
    //            if(temperature.isObject())
    //            {
    //                QJsonObject sub2_obj = temperature.toObject();
    //                QJsonValue sub3_value = *(sub2_obj.find("value"));  //在presure03中寻找value的值
    //                qDebug() << sub3_value.toInt();     //调试打印取出来的值
    //            }


    //            QJsonValue pressure = *(sub_obj.find("pressure")); //在item中寻找presure03项的QJsonValue的值
    //            if(pressure.isObject())
    //            {
    //                QJsonObject sub2_obj = pressure.toObject();
    //                QJsonValue sub3_value = *(sub2_obj.find("value"));  //在presure03中寻找value的值
    //                qDebug() << sub3_value.toInt();     //调试打印取出来的值
    //            }


    //            QJsonValue presure01 = *(sub_obj.find("presure01")); //在item中寻找presure03项的QJsonValue的值
    //            if(presure01.isObject())
    //            {
    //                QJsonObject sub2_obj = presure01.toObject();
    //                QJsonValue sub3_value = *(sub2_obj.find("value"));  //在presure03中寻找value的值
    //                qDebug() << sub3_value.toInt();     //调试打印取出来的值
    //            }

    //            QJsonValue presure02 = *(sub_obj.find("presure02")); //在item中寻找presure03项的QJsonValue的值
    //            if(presure02.isObject())
    //            {
    //                QJsonObject sub2_obj = presure02.toObject();
    //                QJsonValue sub3_value = *(sub2_obj.find("value"));  //在presure03中寻找value的值
    //                qDebug() << sub3_value.toInt();     //调试打印取出来的值
    //            }

    //            QJsonValue presure03 = *(sub_obj.find("presure03")); //在item中寻找presure03项的QJsonValue的值
    //            if(presure03.isObject())
    //            {
    //                QJsonObject sub2_obj = presure03.toObject();
    //                QJsonValue sub3_value = *(sub2_obj.find("value"));  //在presure03中寻找value的值
    //                qDebug() << sub3_value.toInt();     //调试打印取出来的值
    //            }

    //            QJsonValue presure10 = *(sub_obj.find("presure10")); //在item中寻找presure03项的QJsonValue的值
    //            if(presure10.isObject())
    //            {
    //                QJsonObject sub2_obj = presure10.toObject();
    //                QJsonValue sub3_value = *(sub2_obj.find("value"));  //在presure03中寻找value的值
    //                qDebug() << sub3_value.toInt();     //调试打印取出来的值
    //            }

    //            QJsonValue presure11 = *(sub_obj.find("presure11")); //在item中寻找presure03项的QJsonValue的值
    //            if(presure11.isObject())
    //            {
    //                QJsonObject sub2_obj = presure11.toObject();
    //                QJsonValue sub3_value = *(sub2_obj.find("value"));  //在presure03中寻找value的值
    //                qDebug() << sub3_value.toInt();     //调试打印取出来的值
    //            }

    //            QJsonValue presure12 = *(sub_obj.find("presure12")); //在item中寻找presure03项的QJsonValue的值
    //            if(presure12.isObject())
    //            {
    //                QJsonObject sub2_obj = presure12.toObject();
    //                QJsonValue sub3_value = *(sub2_obj.find("value"));  //在presure03中寻找value的值
    //                qDebug() << sub3_value.toInt();     //调试打印取出来的值
    //            }
    //        }

    //    }
    /*****************************************************************/

    /*************使用github上面的开源的接口（对QT的QJson相关类进行了封装，可以不通过QJsonDocument这个类来进行转换******************/
    json_object obj_json(message);
    json_object obj_items = obj_json.object("items");//取出其中的item 的json_object类型，里面主要是需要的数据
    qDebug() << obj_items.format_string();
    if(obj_items)
    {
        this->vRxdata.clear();//每次显示都要清除之前的

        float humidity = obj_items.object("humidity").value("value").toDouble();
        qDebug() << humidity;
        this->vProtocol.rx_info.data[0] = humidity;
        this->vRxdata.append(humidity);

        float temperature = obj_items.object("temperature").value("value").toDouble();
        qDebug() << temperature;
        this->vProtocol.rx_info.data[1] = temperature;
        this->vRxdata.append(temperature);

        float pressure = obj_items.object("pressure").value("value").toDouble();
        qDebug() << pressure;
        this->vProtocol.rx_info.data[2] = pressure;
        this->vRxdata.append(pressure);

        float presure01 = obj_items.object("presure01").value("value").toDouble();
        qDebug() << presure01;
        this->vProtocol.rx_info.data[3] = presure01;
        this->vRxdata.append(presure01);

        float presure02 = obj_items.object("presure02").value("value").toDouble();
        qDebug() << presure02;
        this->vProtocol.rx_info.data[4] = presure02;
        this->vRxdata.append(presure02);

        float presure03 = obj_items.object("presure03").value("value").toDouble();
        qDebug() << presure03;
        this->vProtocol.rx_info.data[5] = presure03;
        this->vRxdata.append(presure03);

        float presure10 = obj_items.object("presure10").value("value").toDouble();
        qDebug() << presure10;
        this->vProtocol.rx_info.data[6] = presure10;
        this->vRxdata.append(presure10);

        float presure11 = obj_items.object("presure11").value("value").toDouble();
        qDebug() << presure11;
        this->vProtocol.rx_info.data[7] = presure11;
        this->vRxdata.append(presure11);

        float presure12 = obj_items.object("presure12").value("value").toDouble();
        qDebug() << presure12;
        this->vProtocol.rx_info.data[8] = presure12;
        this->vRxdata.append(presure12);

        float presure13 = obj_items.object("presure13").value("value").toDouble();
        qDebug() << presure13;
        this->vProtocol.rx_info.data[9] = presure13;
        this->vRxdata.append(presure13);

        float presure20 = obj_items.object("presure20").value("value").toDouble();
        qDebug() << presure20;
        this->vProtocol.rx_info.data[10] = presure20;
        this->vRxdata.append(presure20);

        float presure21 = obj_items.object("presure21").value("value").toDouble();
        qDebug() << presure21;
        this->vProtocol.rx_info.data[11] = presure21;
        this->vRxdata.append(presure21);

        float presure22 = obj_items.object("presure22").value("value").toDouble();
        qDebug() << presure22;
        this->vProtocol.rx_info.data[12] = presure22;
        this->vRxdata.append(presure22);

        float presure23 = obj_items.object("presure23").value("value").toDouble();
        qDebug() << presure23;
        this->vProtocol.rx_info.data[13] = presure23;
        this->vRxdata.append(presure23);

        float presure30 = obj_items.object("presure30").value("value").toDouble();
        qDebug() << presure30;
        this->vProtocol.rx_info.data[14] = presure30;
        this->vRxdata.append(presure30);

        float presure31 = obj_items.object("presure31").value("value").toDouble();
        qDebug() << presure31;
        this->vProtocol.rx_info.data[15] = presure31;
        this->vRxdata.append(presure31);

        float presure32 = obj_items.object("presure32").value("value").toDouble();
        qDebug() << presure32;
        this->vProtocol.rx_info.data[16] = presure32;
        this->vRxdata.append(presure32);

        float presure33 = obj_items.object("presure33").value("value").toDouble();
        qDebug() << presure33;
        this->vProtocol.rx_info.data[17] = presure33;
        this->vRxdata.append(presure33);
    }
    if( enablePlot3DFlag == 1)
    {
            emit Rx3DScope();    //发送信号到3D矩阵显示
    }

    ShowQVariant.setValue(this->vRxdata);
    emit RxScope(ShowQVariant);
    qDebug() << content;


    //    QStringList strList = content.split(" ");
    //    ui->tempEdit->setText(strList[0]);
    //    ui->humiEdit->setText(strList[1]);

}
vSeaskyPortQThread::vSeaskyPortQThread(QObject *parent) : QThread(parent)
{

}
