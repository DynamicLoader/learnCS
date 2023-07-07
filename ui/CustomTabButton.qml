import QtQuick 2.6
import QtQuick.Controls 2.5
import QtQuick.Controls.Material 2.2
import QtQuick.Layouts 1.15
import QtQuick.Dialogs /* Use Qt.labs.platform instead to get native implement of things */
//import Qt.labs.platform 1.1
import "handler/taskMgr.js" as Taskmgr

TabButton {
    id: tb
    enabled: true
    text: qsTr("Untitled tab")
    width: implicitWidth

    icon.source: (tnamObj? "qrc:/ui/img/tnam.png" : "qrc:/ui/img/rnam.png")

    property ComponentTNAM tnamObj;
    property ComponentRNAM rnamObj;
    //property icon icon;

    onDoubleClicked:{
        ptx.visible = false;
        ted.visible = true;
    }

    MessageDialog{
        id: msgClose
        title: "Close on busy"
        text: "The tab \"" + ptx.text + "\" which is to be closed has unfinished job! Do you want to force stop the job and close it anyway?"
        informativeText: "Warning: All undone data will lost and this may cause the program to crash or leak memory!".toUpperCase()
        buttons: MessageDialog.Yes | MessageDialog.No

        modality: Qt.ApplicationModal
        onButtonClicked: function (button, role) {
            switch (button) {
            case MessageDialog.Yes:
                if(tnamObj){
                    tnamObj.terminate();
                    Taskmgr.endTaskTNAM(tb);
                }

                if(rnamObj){
                    rnamObj.terminate();
                    Taskmgr.endTaskRNAM(tb);
                }
                break;
            }
        }
    }

    contentItem: Row{
        spacing: 5
        height: tb.height

        Image{
            height: parent.height
            width: parent.height
            source:tb.icon.source
        }
        Item{
            property string text: tb.text
            width: ptx.width
            height: parent.height
            anchors.verticalCenter: parent.verticalCenter
            //anchors.horizontalCenter: parent.horizontalCenter

            Text{
                id: ptx
                height: parent.height
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                text: parent.text
                color: Material.foreground
            }

            TextInput{
                id: ted
                text: parent.text
                width: parent.width
                height: parent.height
                color: Material.foreground

                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                visible: false
                font.italic: true

                onAccepted: {
                    console.log("Edit done");
                    parent.text = ted.text
                    ptx.visible = true;
                    ted.visible = false;
                }
            }

        }

        ToolButton{
            text: "\u00d7"
            height: parent.height
            width: parent.height
            flat: true

            onClicked: {
                if(tnamObj){
                    if(tnamObj.busy){
                        msgClose.open();
                    }else{
                        Taskmgr.endTaskTNAM(tb);
                    }
                }

                if(rnamObj){
                    if(rnamObj.busy){
                        msgClose.open();
                    }else{
                        Taskmgr.endTaskRNAM(tb);
                    }
                }
            }
        }
    }
}
