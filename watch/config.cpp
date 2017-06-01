#include "config.h"
//#include <fstream>
#include <iostream>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QDataStream>
#include <QDir>
#include <QDebug>
//#include <QMessageBox>
#include <QApplication>

using namespace std;

Config::Config()
{

    cache_path = QApplication::applicationDirPath();
    cache_path = cache_path.append("/config.json");
    
    qDebug() << cache_path;
    
    QFile file(cache_path);
    if (file.open(QFile::ReadOnly)) {
        QString json_str;
        QDataStream ds(&file);
        ds >> json_str;
        file.close();
        
        QJsonParseError json_error;
        QJsonDocument parse_doucment = QJsonDocument::fromJson(json_str.toUtf8(), &json_error); 
        if(json_error.error == QJsonParseError::NoError) 
        {  
            if(parse_doucment.isObject())  
            {  
                QJsonObject obj = parse_doucment.object(); 
                url = obj.take("url").toString();
                cookie = obj.take("cookie").toString();
                valve = obj.take("valve").toString();
                interval = obj.take("interval").toString();
                regex = obj.take("regex").toString();
            }else{
//                QMessageBox::information(NULL,"url","url",QMessageBox::Yes);
            }
        }else{
//             QMessageBox::information(NULL,"d","d",QMessageBox::Yes);
        } 
        
    }else{
//        QMessageBox::information(NULL,cache_path,"a",QMessageBox::Yes);
    }
   
    
    if (url.length() == 0) {
//        QMessageBox::information(NULL,"default","123",QMessageBox::Yes);
        url = "http://172.16.1.135/bugfree/index.php/bug/list/9?query_id=-2";
        valve = "1";
        regex = "总数:<b>(\\d*)</b>";
        cookie = "95auqacc28enctjc5fpo7fnm06";
        interval = "3";
        save();
    }
    
    
//    cout << url << endl;
//    cout << valve << endl;
//    cout << regex << endl;
//    cout << cookie << endl;
//    cout << interval << endl;
}

void Config::save()
{

    QJsonObject json;
    json.insert("url", url);
    json.insert("cookie", cookie);
    json.insert("valve", valve);
    json.insert("interval", interval);
    json.insert("regex", regex);
    
    QJsonDocument document;
    document.setObject(json);
    QByteArray byte_array = document.toJson(QJsonDocument::Compact);
    QString json_str(byte_array);
    QFile file(cache_path);
    if (file.open(QFile::WriteOnly)) {
        QDataStream ds(&file);
        ds << json_str;
        file.close();
    }
}


