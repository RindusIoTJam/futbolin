#include "scoreboard.h"
#include "ui_scoreboard.h"
#include <QImageReader>
#include <QFontDatabase>
#include <QHeaderView>

scoreboard::scoreboard(QString &wallpaperPath,QString &fontPath,position &GreenPosition, position &BluePosition, position &GreenTeamPosition,position &BlueTeamPosition,position &ScoreGreenPosition,position &ScoreBluePosition, position &waitingPlayersPosition,QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::scoreboard),
    futbolinPlayersTopic("rindus/futbolin"),
    futbolinEventTopic("rindus/futbolin/events"),
    bkgnd(wallpaperPath),
    palette(),
    fontPath(fontPath),
    GreenPosition(GreenPosition),
    BluePosition(BluePosition),
    GreenTeamPosition(GreenTeamPosition),
    BlueTeamPosition(BlueTeamPosition),
    ScoreGreenPosition(ScoreGreenPosition),
    ScoreBluePosition(ScoreBluePosition),
    TimePosition(294, 331, 190, 190),
    waitingPlayersPosition(waitingPlayersPosition),
    greenTeam(this),
    blueTeam(this),
    greenTeamLabel(this),
    blueTeamLabel(this),
    scoreGreenLabel(this),
    scoreBlueLabel(this),
    tablemodel(),
    waitingPlayers(this),
    teamsList(),
    scoreCounter(),
    lastEvent('c')
{

    m_client = new QMqttClient(this);
    connect(m_client, &QMqttClient::stateChanged, this, &scoreboard::onConnectionStatus);
    connect(m_client, &QMqttClient::messageReceived, this, &scoreboard::onReceived);
    connect(this, &scoreboard::newPlayerReceived, this, &scoreboard::displayPlayers);

    m_client->setHostname("mqtt.rindus.es");
    m_client->setPort(8883);
    m_client->setUsername("futbolin");
    m_client->setPassword("IqucaEquna785");

    bkgndSize = bkgnd.size();

    greenTeam.setGeometry(GreenPosition.pX1,GreenPosition.pY1,GreenPosition.pX2 - GreenPosition.pX1,GreenPosition.pY2 - GreenPosition.pY1);
    blueTeam.setGeometry(BluePosition.pX1,BluePosition.pY1,BluePosition.pX2 - BluePosition.pX1,BluePosition.pY2 - BluePosition.pY1);
    greenTeamLabel.setGeometry(GreenTeamPosition.pX1,GreenTeamPosition.pY1,GreenTeamPosition.pX2 - GreenTeamPosition.pX1,GreenTeamPosition.pY2 - GreenTeamPosition.pY1);
    blueTeamLabel.setGeometry(BlueTeamPosition.pX1,BlueTeamPosition.pY1,BlueTeamPosition.pX2 - BlueTeamPosition.pX1,BlueTeamPosition.pY2 - BlueTeamPosition.pY1);
    scoreGreenLabel.setGeometry(ScoreGreenPosition.pX1,ScoreGreenPosition.pY1,ScoreGreenPosition.pX2 - ScoreGreenPosition.pX1,ScoreGreenPosition.pY2 - ScoreGreenPosition.pY1);
    scoreBlueLabel.setGeometry(ScoreBluePosition.pX1,ScoreBluePosition.pY1,ScoreBluePosition.pX2 - ScoreBluePosition.pX1,ScoreBluePosition.pY2 - ScoreBluePosition.pY1);
    waitingPlayers.setGeometry(waitingPlayersPosition.pX1,waitingPlayersPosition.pY1,waitingPlayersPosition.pX2 - waitingPlayersPosition.pX1,waitingPlayersPosition.pY2 - waitingPlayersPosition.pY1);

    int fontid = QFontDatabase::addApplicationFont(fontPath);

    QStringList fontlist = QFontDatabase::applicationFontFamilies(fontid);

    QFont font(fontlist.front());




    ui->setupUi(this);

#ifndef QT_DEBUG
    this->setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    this->showFullScreen();
#endif

    if(GreenPosition.show)
    {
        greenTeam.setFont(font);
        greenTeam.setTextColor(QColor(Qt::white));
        greenTeam.setAlignment(Qt::AlignCenter);
        greenTeam.setText("GREEN TEAM");
        greenTeam.show();
    }
    if(BluePosition.show)
    {
        blueTeam.setFont(font);
        blueTeam.setTextColor(QColor(Qt::white));
        blueTeam.setAlignment(Qt::AlignCenter);
        blueTeam.setText("BLUE TEAM");
        blueTeam.show();
    }

    greenTeamLabel.setFont(font);
    greenTeamLabel.setTextColor(QColor(Qt::white));
    greenTeamLabel.setMargin(0);
    greenTeamLabel.setIndent(0);
    greenTeamLabel.show();

    blueTeamLabel.setFont(font);
    blueTeamLabel.setTextColor(QColor(Qt::white));
    blueTeamLabel.setMargin(0);
    blueTeamLabel.setIndent(0);
    blueTeamLabel.show();

    scoreGreenLabel.setFont(font);
    scoreGreenLabel.setTextColor(QColor(Qt::white));
    scoreGreenLabel.setText("0");
    scoreGreenLabel.setAlignment(Qt::AlignCenter);
    scoreGreenLabel.show();

    scoreBlueLabel.setFont(font);
    scoreBlueLabel.setTextColor(QColor(Qt::white));
    scoreBlueLabel.setText("0");
    scoreBlueLabel.setAlignment(Qt::AlignCenter);
    scoreBlueLabel.show();

    waitingPlayers.setModel(&tablemodel);
    waitingPlayers.setStyleSheet("background-color: transparent");
    waitingPlayers.setFont(font);
    waitingPlayers.setPalette(QPalette(QPalette::Background, Qt::transparent));
    waitingPlayers.setAttribute(Qt::WA_TranslucentBackground,true);
    waitingPlayers.verticalHeader()->setStyleSheet("color: white");
    waitingPlayers.horizontalHeader()->hide();
    waitingPlayers.setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    waitingPlayers.show();

    m_client->connectToHostEncrypted();

#ifdef QT_DEBUG
    greenTeamLabel.setText("first line\nsecond line");
    greenTeamLabel.setFrameShape(QFrame::Panel);
    greenTeamLabel.setFrameShadow(QFrame::Sunken);
    greenTeamLabel.setLineWidth(3);

    blueTeamLabel.setText("first line\nsecond line");
    blueTeamLabel.setFrameShape(QFrame::Panel);
    blueTeamLabel.setFrameShadow(QFrame::Sunken);
    blueTeamLabel.setLineWidth(3);

    scoreGreenLabel.setFrameShape(QFrame::Panel);
    scoreGreenLabel.setFrameShadow(QFrame::Sunken);
    scoreGreenLabel.setLineWidth(3);

    scoreBlueLabel.setFrameShape(QFrame::Panel);
    scoreBlueLabel.setFrameShadow(QFrame::Sunken);
    scoreBlueLabel.setLineWidth(3);
#endif
}

scoreboard::~scoreboard()
{
    m_client->~QMqttClient();
    delete ui;
}

void scoreboard::resizeEvent(QResizeEvent *event)
{
    QSize currentSize = this->size();
    resizeBackground(currentSize);
    resizeLabels(currentSize);
}

void scoreboard::onConnectionStatus(QMqttClient::ClientState state)
{
    switch (state)
    {
        case QMqttClient::ClientState::Connecting :
            statusBar()->showMessage(tr("Connecting to MQTT"), 2000);
            qDebug()<< "QMqttClient::ClientState::Connecting";
        break;
        case QMqttClient::ClientState::Connected :
            statusBar()->showMessage(tr("MQTT connected"), 2000);
            qDebug()<< "QMqttClient::ClientState::Connected";
            m_client->subscribe(futbolinPlayersTopic,0);
            m_client->subscribe(futbolinEventTopic,0);
        break;
        case QMqttClient::ClientState::Disconnected :
            statusBar()->showMessage(tr("MQTT disconnected"), 2000);
            qDebug()<< "QMqttClient::ClientState::Disconnected";
            m_client->connectToHostEncrypted();
        break;
        default:
        break;

    }
}

void scoreboard::onReceived(const QByteArray &message, const QMqttTopicName &topic)
{
    qDebug()<< "Received " << message << "from topic " << topic.name();

    if (futbolinPlayersTopic.match(topic.name()))
    {
        parseMQTTtoList(message);
    }
    else if (futbolinEventTopic.match(topic.name()))
    {
        parseMQTTEvent(message);
    }
}

void scoreboard::resizeBackground(QSize Size)
{
    QPixmap scaledbkgnd = bkgnd.scaled(Size, Qt::IgnoreAspectRatio);
    palette.setBrush(QPalette::Background, scaledbkgnd);
    this->setPalette(palette);
}

void scoreboard::resizeLabels(QSize Size)
{
 float originalSW = bkgndSize.width();
 float originalSH = bkgndSize.height();

 float currentSW = Size.width();
 float currentSH = Size.height();

 float scaleH = currentSH/originalSH;
 float scaleW = currentSW/originalSW;

 qDebug()<<"Windows H Original size "<<originalSH;
 qDebug()<<"Windows W Original size "<<originalSW;
 qDebug()<<"Windows H current size "<<currentSH;
 qDebug()<<"Windows W current size "<<currentSW;
 qDebug()<<"Windows H scale "<<scaleH;
 qDebug()<<"Windows W scale "<<scaleW;

 greenTeam.setGeometry(GreenPosition.pX1*scaleW,GreenPosition.pY1*scaleH,(GreenPosition.pX2 - GreenPosition.pX1)*scaleW,(GreenPosition.pY2 - GreenPosition.pY1)*scaleH);
 blueTeam.setGeometry(BluePosition.pX1*scaleW,BluePosition.pY1*scaleH,(BluePosition.pX2 - BluePosition.pX1)*scaleW,(BluePosition.pY2 - BluePosition.pY1)*scaleH);
 greenTeamLabel.setGeometry(GreenTeamPosition.pX1*scaleW,GreenTeamPosition.pY1*scaleH,(GreenTeamPosition.pX2 - GreenTeamPosition.pX1)*scaleW,(GreenTeamPosition.pY2 - GreenTeamPosition.pY1)*scaleH);
 blueTeamLabel.setGeometry(BlueTeamPosition.pX1*scaleW,BlueTeamPosition.pY1*scaleH,(BlueTeamPosition.pX2 - BlueTeamPosition.pX1)*scaleW,(BlueTeamPosition.pY2 - BlueTeamPosition.pY1)*scaleH);
 scoreGreenLabel.setGeometry(ScoreGreenPosition.pX1*scaleW,ScoreGreenPosition.pY1*scaleH,(ScoreGreenPosition.pX2 - ScoreGreenPosition.pX1)*scaleW,(ScoreGreenPosition.pY2 - ScoreGreenPosition.pY1)*scaleH);
 scoreBlueLabel.setGeometry(ScoreBluePosition.pX1*scaleW,ScoreBluePosition.pY1*scaleH,(ScoreBluePosition.pX2 - ScoreBluePosition.pX1)*scaleW,(ScoreBluePosition.pY2 - ScoreBluePosition.pY1)*scaleH);
 waitingPlayers.setGeometry(waitingPlayersPosition.pX1*scaleW,waitingPlayersPosition.pY1*scaleH,(waitingPlayersPosition.pX2 - waitingPlayersPosition.pX1)*scaleW,(waitingPlayersPosition.pY2 - waitingPlayersPosition.pY1)*scaleH);
}

void scoreboard::parseMQTTtoList(const QByteArray &msg)
{
    QString rawPlayers(msg);
    QString name1 = rawPlayers.left(rawPlayers.indexOf(' ',Qt::CaseInsensitive)).remove('+');
    QString name2 = rawPlayers.mid(rawPlayers.indexOf('/')).remove('/');
    name2 = name2.left(name2.indexOf(' ',Qt::CaseInsensitive));

    qDebug() << "Name 1: " << name1 << "\n" << "Name 2: " << name2;

    Team team;
    team.playerUp = name1;
    team.playerDown = name2;

    teamsList.append(team);
    updateWaitingPlayers();
    emit newPlayerReceived();
}

void scoreboard::displayPlayers(void)
{
    if(teamsList.size() < 2 )
    {
         statusBar()->showMessage(tr("Waiting for other team"), 60000);
    }
    else
    {
         statusBar()->clearMessage();
         Team team = teamsList.takeFirst();
         greenTeamLabel.setText(team.playerUp + "\n" + team.playerDown);
         team = teamsList.takeFirst();
         blueTeamLabel.setText(team.playerUp + "\n" + team.playerDown);
         updateWaitingPlayers();
         disconnect(this, &scoreboard::newPlayerReceived, this, &scoreboard::displayPlayers);
    }
}

void scoreboard::parseMQTTEvent(const QByteArray &msg)
{
    if(msg.front() == 'r')
    {
        scoreCounter.blue++;
    }
    else if (msg.front() == 'l')
    {
        scoreCounter.green++;
    }
    else if (msg.front() == 'c')
    {
        if (!teamsList.isEmpty())
        {

            if (scoreCounter.blue < scoreCounter.green)
            {
                Team team = teamsList.takeFirst();
                QStringList lastPlayers = blueTeamLabel.text().split('\n');
                blueTeamLabel.setText(team.playerUp + "\n" + team.playerDown);
                team.playerUp = lastPlayers.front();
                team.playerDown = lastPlayers.back();
                teamsList.push_back(team);
                updateWaitingPlayers();
            }
            else if (scoreCounter.blue > scoreCounter.green)
            {
                Team team = teamsList.takeFirst();
                QStringList lastPlayers = greenTeamLabel.text().split('\n');
                greenTeamLabel.setText(team.playerUp + "\n" + team.playerDown);
                team.playerUp = lastPlayers.front();
                team.playerDown = lastPlayers.back();
                teamsList.push_back(team);
                updateWaitingPlayers();
            }

        }

        scoreCounter = score();
    }
    else if (msg.front() == 'n')
    {
        if(lastEvent == 'r')
        {
            scoreCounter.blue--;
        }
        else if (lastEvent == 'l')
        {
            scoreCounter.green--;
        }
    }
    else if ((msg.front() == 'd'))
    {
        QByteArray msgindex = msg;
        msgindex.remove(0,1);
        bool ok;
        int index = msgindex.toInt(&ok);
        if(ok)
        {
            if(index <= teamsList.size() && index !=0)
            {
                teamsList.remove(--index);
                updateWaitingPlayers();
            }
        }
        else
        {
            if(msg.front() == 'd')
            {
                teamsList.clear();
                blueTeamLabel.clear();
                greenTeamLabel.clear();
                scoreCounter = score();
                connect(this, &scoreboard::newPlayerReceived, this, &scoreboard::displayPlayers);
                updateWaitingPlayers();
            }
        }
    }


    lastEvent = msg.front();
    scoreBlueLabel.setText(QString::number(scoreCounter.blue));
    scoreGreenLabel.setText(QString::number(scoreCounter.green));
    qDebug() << "Blue team counter " << scoreCounter.blue;
    qDebug() << "Green team counter " << scoreCounter.green;
}

void scoreboard::updateWaitingPlayers()
{
    tablemodel.clear();
    for (int i = 0;i < teamsList.size(); i++)
    {
        Team team = teamsList.at(i);
        QStandardItem *NameA = new QStandardItem(team.playerUp + "\n" + team.playerDown);
        NameA->setTextAlignment(Qt::AlignVCenter | Qt::AlignJustify);
        tablemodel.setItem(i,0,NameA);
    }
    waitingPlayers.setModel(&tablemodel);
    waitingPlayers.resizeRowsToContents();
    waitingPlayers.resizeColumnsToContents();
    waitingPlayers.update();
}
