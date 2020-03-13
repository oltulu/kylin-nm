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

#include "utils.h"
#include <stdio.h>

NetworkSpeed::NetworkSpeed(QObject *parent) :QObject(parent){}

int NetworkSpeed::getCurrentDownloadRates(char *netname, long *save_rate, long *tx_rate)
{
    FILE * net_dev_file; //文件指针
    char buffer[1024]; //文件中的内容暂存在字符缓冲区里
    //size_t bytes_read; //实际读取的内容大小
    char * match; //用以保存所匹配字符串及之后的内容
    int counter = 0;
    //int i = 0;
    char tmp_value[128];

    if((NULL == netname)||(NULL == save_rate)||(NULL == tx_rate)){
        printf("bad param\n");
        return -1;
    }

    if ( (net_dev_file=fopen("/proc/net/dev", "r")) == NULL ){ //打开文件/pro/net/dev/，从中读取流量数据
        printf("open file /proc/net/dev/ error!\n");
        return -1;
    }
    memset(buffer,0,sizeof(buffer));

    while(fgets(buffer,sizeof(buffer),net_dev_file) != NULL){
        match = strstr(buffer,netname);

        if(NULL == match){
            //printf("no eth0 keyword to find!\n");
            continue;
        }else{
            //printf("%s\n",buffer);
            match = match + strlen(netname) + strlen(":"); //地址偏移到冒号
            sscanf(match,"%ld ",save_rate);
            memset(tmp_value,0,sizeof(tmp_value));
            sscanf(match,"%s ",tmp_value);
            match = match + strlen(tmp_value);
            for(size_t i=0;i<strlen(buffer);i++){
                if(0x20 == *match){
                    match ++;
                } else{
                    if(8 == counter){
                        sscanf(match,"%ld ",tx_rate);
                    }
                    memset(tmp_value,0,sizeof(tmp_value));
                    sscanf(match,"%s ",tmp_value);
                    match = match + strlen(tmp_value);
                    counter ++;
                }
            }
            //printf("%s save_rate:%ld tx_rate:%ld\n",netname,*save_rate,*tx_rate);
        }
    }

    return 0; //返回成功
}




CustomStyle::CustomStyle(const QString &proxyStyleName, QObject *parent) : QProxyStyle (proxyStyleName)
{

}

void CustomStyle::drawComplexControl(QStyle::ComplexControl control, const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const
{
    return QProxyStyle::drawComplexControl(control, option, painter, widget);
}

void CustomStyle::drawControl(QStyle::ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    return QProxyStyle::drawControl(element, option, painter, widget);
}

void CustomStyle::drawItemPixmap(QPainter *painter, const QRect &rectangle, int alignment, const QPixmap &pixmap) const
{
    return QProxyStyle::drawItemPixmap(painter, rectangle, alignment, pixmap);
}

void CustomStyle::drawItemText(QPainter *painter, const QRect &rectangle, int alignment, const QPalette &palette, bool enabled, const QString &text, QPalette::ColorRole textRole) const
{
    return QProxyStyle::drawItemText(painter, rectangle, alignment, palette, enabled, text, textRole);
}

void CustomStyle::drawPrimitive(QStyle::PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    return QProxyStyle::drawPrimitive(element, option, painter, widget);
}

QPixmap CustomStyle::generatedIconPixmap(QIcon::Mode iconMode, const QPixmap &pixmap, const QStyleOption *option) const
{
    return QProxyStyle::generatedIconPixmap(iconMode, pixmap, option);
}

QStyle::SubControl CustomStyle::hitTestComplexControl(QStyle::ComplexControl control, const QStyleOptionComplex *option, const QPoint &position, const QWidget *widget) const
{
    return QProxyStyle::hitTestComplexControl(control, option, position, widget);
}

QRect CustomStyle::itemPixmapRect(const QRect &rectangle, int alignment, const QPixmap &pixmap) const
{
    return QProxyStyle::itemPixmapRect(rectangle, alignment, pixmap);
}

QRect CustomStyle::itemTextRect(const QFontMetrics &metrics, const QRect &rectangle, int alignment, bool enabled, const QString &text) const
{
    return QProxyStyle::itemTextRect(metrics, rectangle, alignment, enabled, text);
}

int CustomStyle::pixelMetric(QStyle::PixelMetric metric, const QStyleOption *option, const QWidget *widget) const
{
    return QProxyStyle::pixelMetric(metric, option, widget);
}

void CustomStyle::polish(QWidget *widget)
{
    if (widget) {
        if (widget->inherits("QTipLabel")){
            widget->setAttribute(Qt::WA_TranslucentBackground);

            QPainterPath path;
            auto rect = widget->rect();
            rect.adjust(0, 0, -0, -0);
            path.addRoundedRect(rect, 6, 6);

            widget->setProperty("blurRegion", QRegion(path.toFillPolygon().toPolygon()));
        }
    }

    return QProxyStyle::polish(widget);
}

void CustomStyle::polish(QApplication *application)
{
    return QProxyStyle::polish(application);
}

void CustomStyle::polish(QPalette &palette)
{
    return QProxyStyle::polish(palette);
}

void CustomStyle::unpolish(QWidget *widget)
{
    return QProxyStyle::unpolish(widget);
}

void CustomStyle::unpolish(QApplication *application)
{
    return QProxyStyle::unpolish(application);
}

QSize CustomStyle::sizeFromContents(QStyle::ContentsType type, const QStyleOption *option, const QSize &contentsSize, const QWidget *widget) const
{
    return QProxyStyle::sizeFromContents(type, option, contentsSize, widget);
}

QIcon CustomStyle::standardIcon(QStyle::StandardPixmap standardIcon, const QStyleOption *option, const QWidget *widget) const
{
    return QProxyStyle::standardIcon(standardIcon, option, widget);
}

QPalette CustomStyle::standardPalette() const
{
    return QProxyStyle::standardPalette();
}

int CustomStyle::styleHint(QStyle::StyleHint hint, const QStyleOption *option, const QWidget *widget, QStyleHintReturn *returnData) const
{
    return QProxyStyle::styleHint(hint, option, widget, returnData);
}

QRect CustomStyle::subControlRect(QStyle::ComplexControl control, const QStyleOptionComplex *option, QStyle::SubControl subControl, const QWidget *widget) const
{
    return QProxyStyle::subControlRect(control, option, subControl, widget);
}

QRect CustomStyle::subElementRect(QStyle::SubElement element, const QStyleOption *option, const QWidget *widget) const
{
    return QProxyStyle::subElementRect(element, option, widget);
}