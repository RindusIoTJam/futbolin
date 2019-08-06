#ifndef SCOREBOARD_H
#define SCOREBOARD_H

#include <QMainWindow>
#include <QtMqtt/QtMqtt>
#include <QDebug>
#include "./DinamicFontSize/dynamicfontsizelabel.h"


struct position
{
    int pX1;
    int pX2;
    int pY1;
    int pY2;
    bool show;

    position():
        pX1(0),
        pX2(0),
        pY1(0),
        pY2(0),
        show(true)
    {}

    position(int pX1,int pX2,int pY1, int pY2,bool show=true):
        pX1(pX1),
        pX2(pX2),
        pY1(pY1),
        pY2(pY2),
        show(show)
    {}
};

namespace Ui {
class scoreboard;
}

class scoreboard : public QMainWindow
{
    Q_OBJECT

public:
    explicit scoreboard(QWidget *parent = nullptr);
    scoreboard(QString &wallpaperPath,QString &fontPath,position &GreenPosition, position &BluePosition, position &GreenTeamPosition,position &BlueTeamPosition,position &ScoreGreenPosition,position &ScoreBluePosition,QWidget *parent = nullptr);
    ~scoreboard();

private:

    struct Team
    {
        QString playerUp;
        QString playerDown;
        Team():
            playerUp(""),
            playerDown("")
        {}
    };

    struct score
    {
        int blue;
        int green;
        score():
            blue(0),
            green(0)
        {}
    };

    Ui::scoreboard *ui;
    QMqttClient *m_client;
    const QMqttTopicFilter futbolinPlayersTopic;
    const QMqttTopicFilter futbolinEventTopic;
    void resizeEvent(QResizeEvent *event);
    QPixmap bkgnd;
    QSize bkgndSize;
    QPalette palette;
    QString fontPath;

    const position GreenPosition;
    const position BluePosition;
    const position GreenTeamPosition;
    const position BlueTeamPosition;
    const position ScoreGreenPosition;
    const position ScoreBluePosition;
    const position TimePosition;

    DynamicFontSizeLabel greenTeam;
    DynamicFontSizeLabel blueTeam;
    DynamicFontSizeLabel greenTeamLabel;
    DynamicFontSizeLabel blueTeamLabel;
    DynamicFontSizeLabel scoreGreenLabel;
    DynamicFontSizeLabel scoreBlueLabel;
    DynamicFontSizeLabel timeLabel;

    QVector<Team> teamsList;
    score scoreCounter;
    char lastEvent;

    void resizeLabels(QSize Size);
    void resizeBackground(QSize Size);

    void parseMQTTtoList(const QByteArray &msg);
    void parseMQTTEvent(const QByteArray &msg);


public slots:
    void onConnectionStatus(QMqttClient::ClientState state);
    void onReceived(const QByteArray &message, const QMqttTopicName &topic);
    void displayPlayers(void);

signals:
    void newPlayerReceived(void);
};

#endif // SCOREBOARD_H
