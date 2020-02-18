/*
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
 *
 */

#include "kylin-dbus-interface.h"
#include "mainwindow.h"


KylinDBus::KylinDBus(MainWindow *mainWindow, QObject *parent) :QObject(parent)
{
    this->mw = mainWindow;

    getObjectPath(); //获取dbus中 lan 与 WiFi 的device路径
    getPhysicalCarrierState(0); //初始化获取网线插入状态
    getLanHwAddressState(); //获取有线网Mac地址
    getWiredCardName();

    QDBusConnection::systemBus().connect(QString("org.freedesktop.NetworkManager"),
                                         QString("/org/freedesktop/NetworkManager"),
                                         QString("org.freedesktop.NetworkManager"),
                                         QString("DeviceAdded"), mw, SLOT(onWirelessDeviceAdded(QDBusObjectPath) ) );

    QDBusConnection::systemBus().connect(QString("org.freedesktop.NetworkManager"),
                                         QString("/org/freedesktop/NetworkManager"),
                                         QString("org.freedesktop.NetworkManager"),
                                         QString("DeviceRemoved"), mw, SLOT(onWirelessDeviceRemoved(QDBusObjectPath) ) );

    QDBusConnection::systemBus().connect(QString("org.freedesktop.NetworkManager"),
                                         QString(wiredPath.path()),
                                         QString("org.freedesktop.NetworkManager.Device.Wired"),
                                         QString("PropertiesChanged"), this, SLOT(onLanPropertyChanged(QVariantMap) ) );

    if (wirelessPath.path() != ""){
        QDBusConnection::systemBus().connect(QString("org.freedesktop.NetworkManager"),
                                             QString(wirelessPath.path()),
                                             QString("org.freedesktop.NetworkManager.Device.Wireless"),
                                             QString("PropertiesChanged"), this, SLOT(onWifiPropertyChanged(QVariantMap) ) );

        QDBusConnection::systemBus().connect(QString("org.freedesktop.NetworkManager"),
                                             QString(wirelessPath.path()),
                                             QString("org.freedesktop.NetworkManager.Device.Wireless"),
                                             QString("AccessPointAdded"), this, SLOT(onAccessPointAdded(QDBusObjectPath) ) );
        getWirelessCardName();//获取无线网卡名称
    }

    time = new QTimer(this);
    time->setTimerType(Qt::PreciseTimer);
    QObject::connect(time, SIGNAL(timeout()), this, SLOT(slot_timeout()));
}

void KylinDBus::getObjectPath()
{
    QDBusInterface m_interface( "org.freedesktop.NetworkManager",
                              "/org/freedesktop/NetworkManager",
                              "org.freedesktop.NetworkManager",
                              QDBusConnection::systemBus() );

   QDBusReply<QList<QDBusObjectPath>> obj_reply = m_interface.call("GetAllDevices");
   QList<QDBusObjectPath> obj_paths = obj_reply.value();

   foreach (QDBusObjectPath obj_path, obj_paths){
       QDBusInterface interface( "org.freedesktop.NetworkManager",
                                 obj_path.path(),
                                 "org.freedesktop.DBus.Introspectable",
                                 QDBusConnection::systemBus() );

       QDBusReply<QString> reply = interface.call("Introspect");
       if(reply.value().indexOf("org.freedesktop.NetworkManager.Device.Wired") != -1){
           wiredPath = obj_path;
       } else if (reply.value().indexOf("org.freedesktop.NetworkManager.Device.Wireless") != -1){
           wirelessPath = obj_path;
           isWirelessCardOn = true;
       }
   }
}

void KylinDBus::getPhysicalCarrierState(int n)
{
    QDBusInterface interface( "org.freedesktop.NetworkManager",
                              wiredPath.path(),
                              "org.freedesktop.DBus.Properties",
                              QDBusConnection::systemBus() );

    QDBusReply<QVariant> reply = interface.call("Get", "org.freedesktop.NetworkManager.Device.Wired", "Carrier");

    if (reply.value().toString() == "true"){
        isWiredCableOn = true;
    } else if (reply.value().toString() == "false"){
        isWiredCableOn = false;
    } else {
        syslog(LOG_ERR, "Error occured when get the property 'Carrier' of Wired");
        qDebug()<<"Error occured when get the property 'Carrier' of Wired";
    }

    if (n == 1){ this->mw->onPhysicalCarrierChanged(isWiredCableOn);}
}

void KylinDBus::getLanHwAddressState()
{
    QDBusInterface lanInterface( "org.freedesktop.NetworkManager",
                              wiredPath.path(),
                              "org.freedesktop.DBus.Properties",
                              QDBusConnection::systemBus() );

    QDBusReply<QVariant> lanReply = lanInterface.call("Get", "org.freedesktop.NetworkManager.Device.Wired", "HwAddress");
    dbusLanMac = lanReply.value().toString();
}

void KylinDBus::getWiredCardName()
{
    QDBusInterface lanInterface( "org.freedesktop.NetworkManager",
                              wiredPath.path(),
                              "org.freedesktop.DBus.Properties",
                              QDBusConnection::systemBus() );

    QDBusReply<QVariant> lanReply = lanInterface.call("Get", "org.freedesktop.NetworkManager.Device", "Interface");
    dbusLanCardName = lanReply.value().toString();
}

void KylinDBus::getWirelessCardName()
{
    QDBusInterface lanInterface( "org.freedesktop.NetworkManager",
                              wirelessPath.path(),
                              "org.freedesktop.DBus.Properties",
                              QDBusConnection::systemBus() );

    QDBusReply<QVariant> lanReply = lanInterface.call("Get", "org.freedesktop.NetworkManager.Device", "Interface");
    dbusWiFiCardName = lanReply.value().toString();
}

void KylinDBus::getLanIp(QString netName)
{
    QDBusInterface m_interface("org.freedesktop.NetworkManager",
                                      "/org/freedesktop/NetworkManager/Settings",
                                      "org.freedesktop.NetworkManager.Settings",
                                      QDBusConnection::systemBus() );
    QDBusReply<QList<QDBusObjectPath>> m_reply = m_interface.call("ListConnections");

    QList<QDBusObjectPath> m_objNets = m_reply.value();
    foreach (QDBusObjectPath objNet, m_objNets){
        QDBusInterface m_interface("org.freedesktop.NetworkManager",
                                  objNet.path(),
                                  "org.freedesktop.NetworkManager.Settings.Connection",
                                  QDBusConnection::systemBus());
        QDBusMessage reply = m_interface.call("GetSettings");
        const QDBusArgument &dbusArg = reply.arguments().at( 0 ).value<QDBusArgument>();
        //DBus type : a{sa{sv}}, a map with a key of QString, which maps to another map of QString,QVariant
        QMap<QString,QMap<QString,QVariant>> map;
        dbusArg >> map;

        //方法一
        for (int i =0;i<2;i++){
            if(map.values().at(1).values().at(i).toString() == netName){
                dbusLanIpv4 = map.values().at(2).values().at(4).toString();
                dbusLanIpv6 = map.values().at(3).values().at(5).toString();
                break;
            }
        }

//        //方法二,仅供参考，暂不能用
//        for(QString key : map.keys() ){
//            QMap<QString,QVariant> innerMap = map.value(key);
//            qDebug() << "Key: " << key;
//            if (key == "ipv4") {
//                for (QString inner_key : innerMap.keys()){
//                    qDebug() << "       " <<  inner_key << ":" << innerMap.value(inner_key);
//                }
//            }
//        }
    }
}

void KylinDBus::getWifiMac(QString netName, int num)
{
    dbusWifiMac = "";

    QDBusInterface interface( "org.freedesktop.NetworkManager",
                              wirelessPath.path(),
                              "org.freedesktop.NetworkManager.Device.Wireless",
                              QDBusConnection::systemBus() );

    QDBusReply<QList<QDBusObjectPath>> reply = interface.call("GetAllAccessPoints");
    QList<QDBusObjectPath> objPaths = reply.value();
    int n = 0;
    foreach (QDBusObjectPath objPath, objPaths){
        if (n == num){
            QDBusInterface path_interface( "org.freedesktop.NetworkManager",
                                      objPath.path(),
                                      "org.freedesktop.DBus.Properties",
                                      QDBusConnection::systemBus() );

            QDBusReply<QVariant> path_reply = path_interface.call("Get", "org.freedesktop.NetworkManager.AccessPoint", "HwAddress");
            dbusWifiMac = path_reply.value().toString();
        }
        n += 1;
    }
}

void KylinDBus::getActWifiMac(QString netName)
{
    dbusWifiMac = "";

    QDBusInterface m_interface("org.freedesktop.NetworkManager",
                                      "/org/freedesktop/NetworkManager/Settings",
                                      "org.freedesktop.NetworkManager.Settings",
                                      QDBusConnection::systemBus() );
    QDBusReply<QList<QDBusObjectPath>> m_reply = m_interface.call("ListConnections");

    QList<QDBusObjectPath> m_objNets = m_reply.value();
    foreach (QDBusObjectPath objNet, m_objNets){
        QDBusInterface m_interface("org.freedesktop.NetworkManager",
                                  objNet.path(),
                                  "org.freedesktop.NetworkManager.Settings.Connection",
                                  QDBusConnection::systemBus());
        QDBusMessage reply = m_interface.call("GetSettings");
        const QDBusArgument &dbusArg = reply.arguments().at( 0 ).value<QDBusArgument>();
        QMap<QString,QMap<QString,QVariant>> map;
        dbusArg >> map;
        if (map.values().at(0).values().size() >= 5){
            if(map.values().at(2).values().at(0).toString() == netName){
                dbusWifiMac = map.values().at(0).values().at(3).toString();
            }
        }
    }
}

int KylinDBus::getAccessPointsNumber()
{
    QDBusInterface interface( "org.freedesktop.NetworkManager",
                              wirelessPath.path(),
                              "org.freedesktop.NetworkManager.Device.Wireless",
                              QDBusConnection::systemBus() );

    QDBusReply<QList<QDBusObjectPath>> reply = interface.call("GetAllAccessPoints");
    QList<QDBusObjectPath> objPaths = reply.value();

//    foreach (QDBusObjectPath objPath, objPaths){
//        qDebug()<<"debug: *****path is: "<<objPath.path(); //列出每一个objectPath
//    }

    return objPaths.size();
}

int KylinDBus::getLanConnState()
{
    QDBusInterface interface( "org.freedesktop.NetworkManager",
                              "/org/freedesktop/NetworkManager/Devices/2",
                              "org.freedesktop.DBus.Properties",
                              QDBusConnection::systemBus() );

    QDBusReply<QVariant> reply = interface.call("Get", "org.freedesktop.NetworkManager.Device", "ActiveConnection");

    return 0;
}

void KylinDBus::onLanPropertyChanged(QVariantMap qvm)
{
    if (!isRunningFunction) {
        isRunningFunction = true;
        time->start(3000);

        QString str = qvm.value("Carrier").toString();
        if (str == "false" || str == "true"){
            getPhysicalCarrierState(1);
        }
    } else { a = 0; }
}
void KylinDBus::slot_timeout()
{
    isRunningFunction = false;
    time->stop();
}

void KylinDBus::onWifiPropertyChanged(QVariantMap qvm)
{
    //qDebug()<<"debug: *************"<<qvm.values();
    //uint ii = qvm.value("Bitrate").toUInt();
    //QString ss = QString::number(ii);
    //qDebug()<<"debug: ============="<<ss;
}

void KylinDBus::onAccessPointAdded(QDBusObjectPath objPath)
{
    //qDebug()<<"debug: &&&&&&&&&&&&&"<<objPath.path();
}