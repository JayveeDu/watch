#ifndef CONFIG_H
#define CONFIG_H

#include <QObject>

//pojo for persistent
class Config : public QObject 
{
public:
    Config();
    void save();
    
    QString url;//url to fetch html text 
    QString regex;//regex to get watched number
    QString valve;//warning value, exceed this value will warn
    QString cookie;//cookie for login
    QString interval;//interval for fetch html text
    
private:
    QString cache_path;
};

#endif // CONFIG_H
