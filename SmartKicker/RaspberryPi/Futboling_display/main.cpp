#include "scoreboard.h"
#include <QApplication>
#include <QFileInfo>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QString executablePath(QCoreApplication::applicationDirPath());
    QSettings* configurationFile = new QSettings(executablePath + "/futbolin.ini", QSettings::IniFormat);

    configurationFile->beginGroup("Wallpaper");
    QString wallpaperPath = configurationFile->value("Path").toString();
    QFileInfo wallpaperPathInfo(wallpaperPath);
    if(wallpaperPathInfo.isRelative())
    {
        wallpaperPath = executablePath + "/" + wallpaperPath;
    }
    configurationFile->endGroup();

    configurationFile->beginGroup("Fonts");
    QString fontPath = configurationFile->value("Path").toString();
    QFileInfo fontPathInfo(fontPath);
    if(fontPathInfo.isRelative())
    {
        fontPath = executablePath + "/" + fontPath;
    }
    configurationFile->endGroup();

    position GreenPosition;
    position BluePosition;
    position GreenTeamPosition;
    position BlueTeamPosition;
    position ScoreGreenPosition;
    position ScoreBluePosition;
    position waitingPlayersPosition;

    configurationFile->beginGroup("GreenTeamTitle");
    GreenPosition.pX1 = configurationFile->value("X1").toInt();
    GreenPosition.pX2 = configurationFile->value("X2").toInt();
    GreenPosition.pY1 = configurationFile->value("Y1").toInt();
    GreenPosition.pY2 = configurationFile->value("Y2").toInt();
    GreenPosition.show = configurationFile->value("Show").toBool();
    configurationFile->endGroup();
    configurationFile->beginGroup("BlueTeamTitle");
    BluePosition.pX1 = configurationFile->value("X1").toInt();
    BluePosition.pX2 = configurationFile->value("X2").toInt();
    BluePosition.pY1 = configurationFile->value("Y1").toInt();
    BluePosition.pY2 = configurationFile->value("Y2").toInt();
    BluePosition.show = configurationFile->value("Show").toBool();
    configurationFile->endGroup();
    configurationFile->beginGroup("GreenTeamNames");
    GreenTeamPosition.pX1 = configurationFile->value("X1").toInt();
    GreenTeamPosition.pX2 = configurationFile->value("X2").toInt();
    GreenTeamPosition.pY1 = configurationFile->value("Y1").toInt();
    GreenTeamPosition.pY2 = configurationFile->value("Y2").toInt();
    GreenTeamPosition.show = configurationFile->value("Show").toBool();
    configurationFile->endGroup();
    configurationFile->beginGroup("BlueTeamNames");
    BlueTeamPosition.pX1 = configurationFile->value("X1").toInt();
    BlueTeamPosition.pX2 = configurationFile->value("X2").toInt();
    BlueTeamPosition.pY1 = configurationFile->value("Y1").toInt();
    BlueTeamPosition.pY2 = configurationFile->value("Y2").toInt();
    BlueTeamPosition.show = configurationFile->value("Show").toBool();
    configurationFile->endGroup();
    configurationFile->beginGroup("GreenScore");
    ScoreGreenPosition.pX1 = configurationFile->value("X1").toInt();
    ScoreGreenPosition.pX2 = configurationFile->value("X2").toInt();
    ScoreGreenPosition.pY1 = configurationFile->value("Y1").toInt();
    ScoreGreenPosition.pY2 = configurationFile->value("Y2").toInt();
    ScoreGreenPosition.show = configurationFile->value("Show").toBool();
    configurationFile->endGroup();
    configurationFile->beginGroup("BlueScore");
    ScoreBluePosition.pX1 = configurationFile->value("X1").toInt();
    ScoreBluePosition.pX2 = configurationFile->value("X2").toInt();
    ScoreBluePosition.pY1 = configurationFile->value("Y1").toInt();
    ScoreBluePosition.pY2 = configurationFile->value("Y2").toInt();
    ScoreGreenPosition.show = configurationFile->value("Show").toBool();
    configurationFile->endGroup();
    configurationFile->beginGroup("PlayersQueue");
    waitingPlayersPosition.pX1 = configurationFile->value("X1").toInt();
    waitingPlayersPosition.pX2 = configurationFile->value("X2").toInt();
    waitingPlayersPosition.pY1 = configurationFile->value("Y1").toInt();
    waitingPlayersPosition.pY2 = configurationFile->value("Y2").toInt();
    waitingPlayersPosition.show = configurationFile->value("Show").toBool();
    configurationFile->endGroup();

    configurationFile->~QSettings();
    wallpaperPathInfo.~QFileInfo();
    fontPathInfo.~QFileInfo();

    scoreboard w(wallpaperPath,fontPath,GreenPosition,BluePosition,GreenTeamPosition,BlueTeamPosition,ScoreGreenPosition,ScoreBluePosition,waitingPlayersPosition);
    w.show();

    return a.exec();
}
