#include "mainwindow.h"
#include <QApplication>
const QString SerialScopeVersion = "Sailor Project V0.0.3";
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
/******************通用解决中文乱码问题****************/
//    //设置中文字体
//    a.setFont(QFont("Microsoft Yahei", 9));

//    //设置中文编码
//#if (QT_VERSION <= QT_VERSION_CHECK(5,0,0))
//#if _MSC_VER
//    QTextCodec *codec = QTextCodec::codecForName("gbk");
//#else
//    QTextCodec *codec = QTextCodec::codecForName("utf-8");
//#endif
//    QTextCodec::setCodecForLocale(codec);
//    QTextCodec::setCodecForCStrings(codec);
//    QTextCodec::setCodecForTr(codec);
//#else
//    QTextCodec *codec = QTextCodec::codecForName("utf-8");
//    QTextCodec::setCodecForLocale(codec);
//#endif


    MainWindow w;
  QFile file(":/qss/vQss/vQss.css");
// QFile file(":/qss/vQss/vQss_black.css");
    if (file.open(QFile::ReadOnly))
    {
        QTextStream filetext(&file);
        QString stylesheet = filetext.readAll();
        qApp->setStyleSheet(stylesheet);
    }
    a.setWindowIcon(QIcon(":/image/image/main.ico"));
    w.setWindowTitle(SerialScopeVersion);
    w.show();
    return a.exec();
}
