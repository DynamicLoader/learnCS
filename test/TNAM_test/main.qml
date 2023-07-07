import QtQuick 2.6
import QtQuick.Window 2.2
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.15
//import QtQuick.Dialogs /* Use Qt.labs.platform instead to get native implement of things */
import Qt.labs.platform 1.1
import WrapperTNAM 1.0
import QImageMemory 1.0
import QUtils 1.0
import TNAM_Stat 1.0

ApplicationWindow {
    id: window
    visible: true
    width: 800
    height: 600
    maximumHeight: 1080
    maximumWidth: 1920
    minimumHeight: 480
    minimumWidth: 640
    //flags: Window
    title: qsTr("Hello World")

    MenuBar {
        Menu {
            title: qsTr("&File")
            MenuItem { 
                text:"Set &Input File" 
                onTriggered: opfin.open();
            }
            MenuSeparator {}
            MenuItem { 
                text:"Set &Output File" 
                onTriggered: opfout.open();
            }
        }

        Menu {
            title: qsTr("&Help")
            MenuItem {
                text: qsTr("&Help")
                onTriggered: console.debug(qsTr("Open action triggered"))
            }
            MenuItem {
                text:"&About..."
                onTriggered: messageDialog.open();
            }
        }
    }

    FileDialog {
        id: opfin
        fileMode: FileDialog.OpenFile
        onAccepted: {
            tnam.inputFile =  file;
            console.log("Opened " + file + "for input");
        }
        onRejected: console.log("Operation aborted by user");
    }

    FileDialog {
        id: opfout
        fileMode: FileDialog.SaveFile
        onAccepted: {
            tnam.outputFile =  file;
            console.log("Opened " + file + "for output");
        }
        onRejected: console.log("Operation aborted by user");
    }

    StackLayout {
        width: parent.width
        anchors.fill: parent
        currentIndex: 0
        Item {
            id: homeTab
            //anchors.fill: parent
            //color: red
            Row{
                anchors.fill: parent
                Column{
                    //anchors.fill: parent
                    CheckBox{
                        id: chbFileI
                        checked: true
                        text: "Use File-based Encode"
                    }
                    CheckBox{
                        id: chbFileO
                        checked: false
                        text: "Use File-based Decode"
                    }
                    Text{
                        id: encProg
                        width: 100
                        height: 25
                        text: "Not encoded"
                    }
                    Button{
                        id: stEnc
                        text: "Load image"
                        onClicked: {
                            //tnam.inputFile = "./test/TNAM_test/res.bmp"
                            console.log("Start Encode...");
                            stEnc.enabled = false;
                            tnam.startEncode(!chbFileI.checked);
                            tnamTimer.start();
                        }
                    }
                    Button{
                        id: stDec
                        text: "Decode"
                        onClicked: {
                            console.log("Start Decode...");
                            stDec.enabled = false;
                            tnam.startDecode(!chbFileO.checked);
                        }
                    }
                    Button{
                        id: saveEnc
                        text: "Save Encoded"
                        onClicked: tnam.saveEncoded();
                    }
                    Button{
                        id: saveDec
                        text: "Save Decoded"
                        onClicked: tnam.saveDecoded();
                    }
                    Button{
                        text: "Display decoded"
                        onClicked: tnam.displayTo(qim)
                    }
                    ComboBox{
                        id: com
                        model: QUtils.listInternalResource("/binimgs/")
                        onActivated: {
                            qim.DataMat = QUtils.getSourceMatFromQRC("/binimgs/" + com.currentText);
                            qim.update();
                        }
                        onHighlighted: function(index){
                            qim.DataMat = QUtils.getSourceMatFromQRC("/binimgs/" + com.valueAt(index));
                            qim.update();
                        }
                    }
                    Text{
                        id: encStat
                    }
                }
                Column{
                    Slider {
                        id: sizeCh
                        
                        from: 0
                        to: 2
                        stepSize: 0.01
                        value: 1
                        snapMode: Slider.SnapOnRelease

                        onMoved: {
                            console.log("reset qim size to",sizeCh.value);
                        }
                    }
                
                    QImageMemory{
                        id: qim
                        height: ImgHeight * sizeCh.value
                        width: ImgWidth * sizeCh.value
                        //DataMat: QUtils.getSourceMatFromQRC("qrc:/embed/binimgs/baboonzyp256_binary.bmp")
                    }

                    TNAM_Stat{
                        id: tnamencs
                    }
                }
            }
        }
    }

    Timer{
        id:tnamTimer;
        interval: 50;
        repeat:true;    
        triggeredOnStart: true;
        onTriggered: {
           encProg.text = (tnam.getEncodeProgress() * 100).toFixed(2) + "%";
        }
    }

     TNAM{
        id: tnam
        onEncodeFinished: {
            console.log("Encode Finished");
            stEnc.enabled = true;
            tnamTimer.stop();
            encProg.text = "100%";
            tnam.getEncodeStat(tnamencs);
            encStat.text = "Encode Result: Found " + tnamencs.TriNum + " Triangles, " + tnamencs.LineNum + " Lines and " + tnamencs.PointNum + " Points";
        }

        onDecodeFinished: {
            tnam.displayTo(qim);
            qim.update();
            console.log("Decode done!");
            stDec.enabled = true;
        }
    }

    MessageDialog {
       
        id: messageDialog
        buttons: MessageDialog.Ok
        informativeText: "Test"
        detailedText: "TNAM: " + tnam.getName()
        title: "TNAM Test"
        text: "Only for test"
        modality: Qt.ApplicationModal
    }

}
