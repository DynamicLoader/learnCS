import QtQuick 2.6
import QtQuick.Window 2.2
import QtQuick.Controls 2.5
import QtQuick.Controls.Material 2.12
import QtQuick.Layouts 1.15
import QtQuick.Dialogs /* Use Qt.labs.platform instead to get native implement of things */
//import Qt.labs.platform 1.1
import WrapperTNAM 1.0
import WrapperRNAM 1.0

import "handler/app.js" as App
import "handler/taskMgr.js" as Taskmgr

ApplicationWindow {
    id: window
    visible: true
    width: 1080
    height: 840
    //    maximumHeight: 1080
    //    maximumWidth: 1920
    minimumHeight: 800
    minimumWidth: 840
    title: qsTr("SuperNAM-Z V1.0.0")
    //color: "white"

    QtObject {
        id: themeConfig
        property int theme: Material.System
        property color primary: Material.color(Material.BlueGrey)
        property color accent: Material.color(Material.Teal)
    }

    Material.primary: themeConfig.primary
    Material.theme: themeConfig.theme
    Material.accent:  themeConfig.accent

    onClosing: (c)=>{

                   msgExit.open();
                   c.accepted = false;
               }

    Menu {
        id: menu

        MenuItem {
            icon.source: "qrc:/ui/img/tnam.png"
            icon.color: "transparent"
            text:"New &TNAM task"
            onTriggered: {
                Taskmgr.addTaskTNAM();
            }
        }
        MenuItem {
            icon.source: "qrc:/ui/img/rnam.png"
            icon.color: "transparent"
            text:"New &RNAM task"
            onTriggered: {
                Taskmgr.addTaskRNAM();
            }
        }
        MenuSeparator{}

        Menu{
            id: menuTheme
            title: "Theme..."

            MenuItem{
                id: themeL
                text: "Light"
                checkable: true

                onTriggered: {
                    themeL.checked = true;
                    themeD.checked = false;
                    themeS.checked = false;
                    themeConfig.theme = Material.Light;
                }
            }
            MenuItem{
                id: themeD
                text: "Dark"
                checkable: true

                onTriggered: {
                    themeL.checked = false;
                    themeD.checked = true;
                    themeS.checked = false;
                    themeConfig.theme = Material.Dark;
                }
            }
            MenuItem{
                id: themeS
                text: "System"
                checkable: true
                checked: true

                onTriggered: {
                    themeL.checked = false;
                    themeD.checked = false;
                    themeS.checked = true;
                    themeConfig.theme = Material.System;
                }
            }
        }

        MenuSeparator{}

        MenuItem{
            icon.source: "qrc:/ui/img/about.logo.png"
            icon.color: "transparent"
            text: "&About..."
            onTriggered:{
                aboutDialog.open();
            }
        }
    }

    MessageDialog{
        id: msgExit
        title: "About to exit"
        text: "Do you really want to exit the program?"
        informativeText: "All undone job will be terminated at once and unsaved data will lost!"
        buttons: MessageDialog.Yes | MessageDialog.No
        modality: Qt.ApplicationModal
        onButtonClicked: function (button, role) {
            switch (button) {
            case MessageDialog.Yes:
                Qt.exit(0);
                break;
            }
        }
    }

    TabBar {

        id: bar
        visible: sl.count
        width: parent.width - rmenu.width
        TabButton{ // To fix layout error
            id: __plh
            visible: false
            enabled: false
            width: 0
        }

        // onCurrentIndexChanged: {

        //     if(bar.currentIndex ==0 && bar.count >1 ){
        //         console.log(bar.currentIndex);
        //         bar.setCurrentIndex(1);
        //     }
        // }
    }

    Rectangle{
        visible: bar.visible
        width: bar.height
        height: bar.height
        anchors.right: parent.right
        color: Material.backgroundColor

        RoundButton{
            id: rmenu
            width: bar.height
            height: bar.height
            flat: false
            Material.accent: Material.darkBlue
            text: "+"

            onClicked: {
                menu.popup(rmenu,0,rmenu.height - 5);
            }
        }
    }


    StackLayout {
        id: sl
        visible: bar.visible
        width: parent.width
        height: parent.height - bar.height

        anchors.top: bar.bottom
        currentIndex: bar.currentIndex - 1
    }

    Item{ // Welcome Page here!
        anchors.fill: parent

        visible: !sl.count

        ColumnLayout{
            spacing: 5
            anchors.fill: parent
            anchors.leftMargin: 20
            anchors.topMargin: 20
            anchors.bottomMargin: 20

            RowLayout {
                anchors.horizontalCenter: parent.horizontalCenter
                Image {
                    anchors.top: parent.top
                    source: "img/icon.png"
                }

                Text {
                    color: Material.foreground
                    font.pixelSize: 40
                    text: "Welcome to SuperNAM-Z!\nCreate your first task."
                }
            }

            RowLayout{
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width
                spacing: 5

                RoundButton{
                    icon.source: "qrc:/ui/img/tnam.png"
                    icon.color: "transparent"
                    text: "Create TNAM task"
                    onClicked: {
                        Taskmgr.addTaskTNAM();
                    }
                }

                RoundButton{
                    icon.source: "qrc:/ui/img/rnam.png"
                    icon.color: "transparent"
                    text: "Create RNAM task"
                    onClicked: {
                        Taskmgr.addTaskRNAM();
                    }
                }

                RoundButton{
                    icon.source: "qrc:/ui/img/about.logo.png"
                    icon.color: "transparent"
                    text: "About"
                    onClicked: aboutDialog.open();
                }

                RoundButton{
                    id: btnTheme
                    icon.source: themeConfig.theme == Material.System? "qrc:/ui/img/system.png" : (themeConfig.theme === Material.Light ? "qrc:/ui/img/sun.png" : "qrc:/ui/img/moon.png")
                    icon.color: "transparent"
                    text: "Theme"
                    onClicked: menuTheme.popup(btnTheme,0,0)
                }
            }

        }


    }

    TNAM{
        id: ext
    }

    RNAM{
        id: exr
    }

    MessageDialog {
        id: aboutDialog
        buttons: MessageDialog.Ok
        title: "About SuperNAM-Z"
        text: "  SuperNAM-Z provides a simple GUI for testing Triangular NAM and Rectangular NAM codec.\n"

        informativeText: "Contributors: \n\n"
                         + "  Lu Yi (DynamicLoader)\n"
                         + "  Lin HongJie (Alex Lin)\n"
                         + "  Zeng YiSen (easonT)\n"
                         + "  Luo QiHang (llqqhh)\n\n"
                         + "  Core code of RNAM and TNAM algorithm was written by Yunping Zheng et al. We refined"
                         + " the original code, improved its redability and boosted the performance of TNAM.\n\n"
                         + "Internal Component used: \n"+ ext.getName() + "\n" + exr.getName() + "\n\n" +
                                                              "The program used Qt,Qml and OpenCV. Opensourced under LGPL-3.\n" +
                                                              "Please visit [https://docs.dyldr.top/] for source code after 09/01/2023."


        modality: Qt.ApplicationModal
    }

}
