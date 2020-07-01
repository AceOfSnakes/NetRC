/*
 * AVRPioRemote
 * Copyright (C) 2013  Andreas Müller, Ulrich Mensfeld
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef PLAYERINTERFACE_H
#define PLAYERINTERFACE_H


#include <QSettings>
#include <QRegExp>
#include <QVariant>
#include <QVariantMap>
#include <QtNetwork/QTcpSocket>
#include <string>

using namespace std;

class PlayerInterface : public QObject
{
    Q_OBJECT
public:
    PlayerInterface();

    bool IsConnected();
    QVariantList   m_ping_commands = {"?P"};
    QVariantList   m_ping_cmds = {"?P","?L","?A"};

    QVariantMap m_PlayerSettings;/* = {
        {"pingCommands",m_ping_cmds},
        {"pingResponseOk","P0"},
        {"pingResponseErr","E0"},
        {"timeRegExp", "^([0-9]{3})?([0-9]{2,3})([0-9]{2})([0-9]{2})([0-9]{2})$"},
        {"BdPowerOnButton","PN"},
        {"BdPowerOffButton","PF"},
        {"BdContinuedButton","/A181AFAA/RU"},
        {"BdOpen_CloseButton","/A181AFB6/RU"},
        {"BdAudioButton","/A181AFBE/RU"},
        {"BdSubtitleButton","/A181AF36/RU"},
        {"BdAngleButton","/A181AFB5/RU"},
        {"BdFlDimmerButton","/A181AFF9/RU"},
        {"BdCD_DVDButton","/A181AF2A/RU"},
        {"BdHDMIButton","/A181AFF8/RU"},
        {"BdTopMenuButton","/A181AFB4/RU"},
        {"BdFunctionButton","/A181AFB3/RU"},
        {"BdExitButton","/A181AF20/RU"},
        {"BdMediaGalleryButton","/A181AFF7/RU"},
        {"BdPopUpMenuButton","/A181AFB9/RU"},
        {"CursorUpButton","/A184FFFF/RU"},
        {"CursorLeftButton","/A187FFFF/RU"},
        {"CursorEnterButton","/A181AFEF/RU"},
        {"CursorRightButton","/A186FFFF/RU"},
        {"CursorDownButton","/A185FFFF/RU"},
        {"BdHomeMenuButton","/A181AFB0/RU"},
        {"BdReturnButton","/A181AFF4/RU"},
        {"BdProgramButton","/A181AF60/RU"},
        {"BdBookmarkButton","/A181AF61/RU"},
        {"BdZoomButton","/A181AF62/RU"},
        {"BdIndexButton","/A181AF63/RU"},
        {"BdPrevButton","/A181AFEA/RU"},
        {"BdPlayButton","/A181AF39/RU"},
        {"BdNextButton","/A181AFE9/RU"},
        {"BdRevButton","/A181AF3E/RU"},
        {"BdPauseButton","/A181AF3A/RU"},
        {"BdStopButton","/A181AF38/RU"},
        {"BdFwdButton","/A181AF3D/RU"},
        {"Bd0Button","/A181AFA0/RU"},
        {"Bd1Button","/A181AFA1/RU"},
        {"Bd2Button","/A181AFA2/RU"},
        {"Bd3Button","/A181AFA3/RU"},
        {"Bd4Button","/A181AFA2/RU"},
        {"Bd5Button","/A181AFA5/RU"},
        {"Bd6Button","/A181AFA6/RU"},
        {"Bd7Button","/A181AFA7/RU"},
        {"Bd8Button","/A181AFA8/RU"},
        {"Bd9Button","/A181AFA9/RU"},
        {"Bd2ndVideoButton","/A181AFBF/RU"},
        {"Bd2AudioButton","/A181AFBD/RU"},
        {"BdA_BButton","/A181AFE4/RU"},
        {"BdClearButton","/A181AFE5/RU"},
        {"BdEnterButton","/A181AFEF/RU"},
        {"BdRepeatButton","/A181AFE8/RU"},
        {"BdDisplayButton","/A181AFE3/RU"},
        {"BdKeylockButton","/A181AF22/RU"},
        {"BdReplayButton","/A181AF24/RU"},
        {"BdSkipSearchButton","/A181AF25/RU"},
        {"prefferedPort",8102},
        {"crlf",true}
    };
*/
private:
    QTcpSocket      m_Socket;
    string          m_ReceivedString;
    bool            m_Connected;
    bool            crlf;
    int             m_error_count;
    QRegExp         rxBD;
    void InterpretString(const QString& data);

private slots:

    void TcpError(QAbstractSocket::SocketError socketError);
    void TcpConnected();
    void TcpDisconnected();
    void ReadString();
public slots:
    void ConnectToPlayer(const QString &PlayerIpAddress, const int PlayerIpPort);
    void Disconnect();
    bool SendCmd(const QString& cmd);
    void reloadPlayerSettings(QVariantMap  settings);

signals:
    void SettingsChanged();
    void PlayerOffline(bool);
    bool CmdToBeSend(const QString& cmd); // for logging
    void CommError(QString error);
    void Connected();
    void Disconnected();
    void DataReceived(QString);
    void UpdateDisplayInfo(QRegExp&);
};

#endif // PLAYERINTERFACE_H
