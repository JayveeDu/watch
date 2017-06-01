/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "window.h"

#ifndef QT_NO_SYSTEMTRAYICON

#include <QAction>
#include <QCheckBox>
#include <QComboBox>
#include <QCoreApplication>
#include <QCloseEvent>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QPushButton>
#include <QSpinBox>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QDebug>
#include <QTimer>
#include <QtNetwork>
#include <QRegularExpression>
#include <iostream>
#include <QPixmap>
#include <QPainter>

#ifdef Q_OS_OSX
#include <QtMac>
#endif

using namespace std;

//! [0]
Window::Window()
{
    flag = 0;
    timer = Q_NULLPTR;
    fetchingflag = false;
    currentIndex = -1;
    currentLeft = -1;
//    icons = new QList<QIcon>;
//    icons->append(QIcon(":/images/sad.png"));
//    icons->append(QIcon(":/images/happy.png"));
//    icons->append(QIcon(":/images/net.png"));
    maps = new QList<QPixmap>;
    maps->append(QPixmap(":/images/sad.png"));
    maps->append(QPixmap(":/images/happy.png"));
    maps->append(QPixmap(":/images/net.png"));
    
    
    config = new Config;
    
    manager = new QNetworkAccessManager(this);
    manager->setConfiguration(QNetworkConfiguration());
    manager->setProxy(QNetworkProxy::NoProxy);
    connect(manager, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(replyFinished(QNetworkReply*)));
    
    durationLabel = new QLabel(tr("Interval:"));
    durationSpinBox = new QSpinBox;
    durationSpinBox->setRange(1, 60);
    durationSpinBox->setValue(QString(config->interval).toInt());
   
    valveLabel = new QLabel(tr("Valve:"));
    valveEdit = new QLineEdit(tr(config->valve.toStdString().c_str()));
    
    cookieLabel = new QLabel(tr("Cookie:"));
    cookieEdit = new QLineEdit(tr(config->cookie.toStdString().c_str()));
    
    regexLabel = new QLabel(tr("Regex:"));
    regexEdit = new QLineEdit(tr(config->regex.toStdString().c_str()));
    
    urlLabel = new QLabel(tr("Url:"));
    urlEdit = new QLineEdit(tr(config->url.toStdString().c_str()));
    
    saveButton = new QPushButton(tr("Save"));
    saveButton->setDefault(true);
    connect(saveButton, &QAbstractButton::clicked, this, &Window::save);
    
    createActions();
    createTrayIcon();
//    connect(trayIcon, &QSystemTrayIcon::activated, this, &Window::iconActivated);
    setIcon(2,0);
    trayIcon->show();

    createGroupBox();
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(groupBox);
    setLayout(mainLayout);
    setWindowTitle(tr("Watch"));
    resize(400, 300);
    
    timeoutTimer = new QTimer(this);
    timeoutTimer->setInterval(1000);
    connect(timeoutTimer, SIGNAL(timeout()), this, SLOT(calc()));
    
    startTimer();
}

void Window::startTimer(){
    if (timer == Q_NULLPTR) {
        timer = new QTimer(this);
        connect(timer, SIGNAL(timeout()), this, SLOT(update()));
    }
    timer->setInterval(durationSpinBox->value() * 1000);
    if (!timer->isActive()) {
       timer->start();
    }
}

void Window::stopTimer(){
    if (timer != Q_NULLPTR && timer->isActive()) {
        timer->stop();
    }
    
}

void Window::timeout(){
    
    qDebug() << "time out";
    
}



void Window::save(){
    
    stopTimer();
    
    QString cookieStr = cookieEdit->text();
    QString urlStr = urlEdit->text();
    QString valveStr = valveEdit->text();
    QString regexStr = regexEdit->text();
    QString intervalStr = durationSpinBox->text();
    
    config->url = urlStr;
    config->cookie = cookieStr;
    config->valve = valveStr;
    config->regex = regexStr;
    config->interval = intervalStr;
    
//    cout << config.url << endl;
//    cout << config.cookie << endl;
//    cout << config.valve << endl;
//    cout << config.regex << endl;
//    cout << config.interval << endl;
   
    config->save();
    
    startTimer();
    
}

void Window::calc()
{
    
    qDebug() << "timeout";
    if (flag > 3) {
        flag = 0;
        setIcon(2,0);
        fetchingflag = false;
        if (timeoutTimer->isActive()) {
            timeoutTimer->stop();
            qDebug() << "stop timeout";
        }
    }else{
        flag++;
        qDebug() << "timeout++";
    }
    
}

void Window::update(){
    
    if (fetchingflag) {
        return;
    }
    fetchingflag = true;
    
    if (!timeoutTimer->isActive()) {
        qDebug() << "send";
        QString cki = cookieEdit->text();
        QNetworkCookie cookie("PHPSESSID",cki.toUtf8());
        manager->setCookieJar(new QNetworkCookieJar);
        QList<QNetworkCookie> list;
        list << cookie;
        manager->cookieJar()->setCookiesFromUrl(list,QUrl(urlEdit->text()));
        timeoutTimer->start();
        manager->get(QNetworkRequest(QUrl(urlEdit->text())));
    }
    
}

void Window::replyFinished(QNetworkReply* reply) {
    
    if (timeoutTimer->isActive()) {
        timeoutTimer->stop();
         qDebug() << "reply stop timeout";
    }
    flag = 0;
        
    if (reply->error()) {
        manager->setConfiguration(QNetworkConfiguration());
        manager->setNetworkAccessible(QNetworkAccessManager::Accessible);
        setIcon(2,0);
        qDebug() << "error : " << reply->error() << " readall : " << reply->readAll() << " string : " << reply->errorString();
        reply->deleteLater();
        reply = Q_NULLPTR;
        fetchingflag = false;
        return;
    }
    
    QByteArray array = reply->readAll();
    QString string = QString(array);
    
    reply->deleteLater();
    reply = Q_NULLPTR;
    
    QRegularExpression re(regexEdit->text());
    QRegularExpressionMatch match = re.match(string);
    bool hasMatch = match.hasMatch();
    if (string.length() > 0 && hasMatch) {
        QString numStr = match.captured(1);
        
        int left = numStr.toInt() - valveEdit->text().toInt();
        
        if (left > 0) {
              showMessage();
              setIcon(0,left);
#ifdef Q_OS_OSX
              QtMac::setBadgeLabelText(QString::number(left));
#endif
              
        }else{
              setIcon(1,0);
#ifdef Q_OS_OSX
              QtMac::setBadgeLabelText("");
#endif
              
        }
        
    }else{
        setIcon(2,0);
#ifdef Q_OS_OSX
        QtMac::setBadgeLabelText("");
#endif
        
    }
   
    fetchingflag = false;
}




//! [0]

//! [1]
void Window::setVisible(bool visible)
{
    minimizeAction->setEnabled(visible);
    maximizeAction->setEnabled(!isMaximized());
    restoreAction->setEnabled(isMaximized() || !visible);
    QDialog::setVisible(visible);
}
//! [1]

//! [2]
void Window::closeEvent(QCloseEvent *event)
{
#ifdef Q_OS_OSX
    if (!event->spontaneous() || !isVisible()) {
        return;
    }
#endif
    if (trayIcon->isVisible()) {
        hide();
        event->ignore();
    }
}
//! [2]

//! [3]
void Window::setIcon(int index, int count)
{
    if (currentIndex == index && currentLeft == count) {
        return;
    }
    currentIndex = index;
    currentLeft = count;
    QPixmap pmap = maps->at(index);
    
    QRect rect = pmap.rect();
    QPixmap pixmap = pmap.copy(rect);
    
    if (count > 0) {
        
        QPainter painter;
        painter.begin(&pixmap);
        
        QPen pen = painter.pen();
        pen.setColor(Qt::red);
        painter.setPen(pen);
        
        QFont font = painter.font();
        font.setBold(true);
        font.setPixelSize(90);
        painter.setFont(font);
        
        QString string;
        string = QString::number(count);
        rect.adjust(10,-10,0,0);
        painter.drawText(rect, Qt::AlignRight | Qt::AlignTop,string); 
        
        
        painter.end();
    }
    
    QIcon icon(pixmap);
    trayIcon->setIcon(icon);
    setWindowIcon(icon);
}
//! [3]

//! [4]
//void Window::iconActivated(QSystemTrayIcon::ActivationReason reason)
//{
////    switch (reason) {
////    case QSystemTrayIcon::Trigger:
////    case QSystemTrayIcon::DoubleClick:
////        break;
////    case QSystemTrayIcon::MiddleClick:
////        break;
////    default:
////        ;
////    }
//}
//! [4]

//! [5]
void Window::showMessage()
{
    
    if (currentIndex == 0) {
        return;
    }
    
    trayIcon->showMessage("Warning", "Bug's coming", QSystemTrayIcon::Information, 2000);
    
}
//! [5]



void Window::createGroupBox()
{
    groupBox = new QGroupBox(tr("Configuration"));

    QGridLayout *messageLayout = new QGridLayout;
    messageLayout->addWidget(durationLabel, 0, 0);
    messageLayout->addWidget(durationSpinBox, 0, 1, 1, 3);
    messageLayout->addWidget(valveLabel, 1, 0);
    messageLayout->addWidget(valveEdit, 1, 1, 1, 3);
    messageLayout->addWidget(cookieLabel, 2, 0);
    messageLayout->addWidget(cookieEdit, 2, 1,1,3);
    messageLayout->addWidget(regexLabel, 3, 0);
    messageLayout->addWidget(regexEdit, 3, 1,1,3);
    messageLayout->addWidget(urlLabel, 4, 0);
    messageLayout->addWidget(urlEdit, 4, 1,1,3);
    messageLayout->addWidget(saveButton, 5, 0,1,4);
    groupBox->setLayout(messageLayout);
}

void Window::createActions()
{
    minimizeAction = new QAction(tr("Mi&nimize"), this);
    connect(minimizeAction, &QAction::triggered, this, &QWidget::hide);

    maximizeAction = new QAction(tr("Ma&ximize"), this);
    connect(maximizeAction, &QAction::triggered, this, &QWidget::showMaximized);

    restoreAction = new QAction(tr("&Restore"), this);
    connect(restoreAction, &QAction::triggered, this, &QWidget::showNormal);

    quitAction = new QAction(tr("&Quit"), this);
    connect(quitAction, &QAction::triggered, qApp, &QCoreApplication::quit);
}

void Window::createTrayIcon()
{
    trayIconMenu = new QMenu(this);
    trayIconMenu->addAction(minimizeAction);
    trayIconMenu->addAction(maximizeAction);
    trayIconMenu->addAction(restoreAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(quitAction);

    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setContextMenu(trayIconMenu);
}

#endif
