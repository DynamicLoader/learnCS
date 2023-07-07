import QtQuick 2.6
import QtQuick.Window 2.2
import QtQuick.Controls 2.5
import QtQuick.Controls.Material 2.12
import QtQuick.Layouts 1.15
import QtQuick.Dialogs /* Use Qt.labs.platform instead to get native implement of things */
//import Qt.labs.platform 1.1
import WrapperTNAM 1.0
import QImageMemory 1.0
import QUtils 1.0
import TNAM_Stat 1.0

import "handler/app.js" as App

Item{

    property int idx
    property bool busy: false
    anchors.fill: parent

    function terminate(){
        return tnam.terminate();
    }

    FileDialog {
        id: opfin
        fileMode: FileDialog.OpenFile
        nameFilters: (modeEnc.checked ? ["Image Files (*.bmp *.jpeg *.png *.jpg *.gif *.webp)","All Files (*)"] : ["Zipped Image File (*.zif)","All Files (*)"])
        onAccepted: {
            //tnam.inputFile = file;
            set_input_file.text = QUtils.url2LocalPath(selectedFile);
            //console.log("Opened " + file + "for input");

            tnam.displayTo(qim);
            qim.update();

            //console.log(thresh.value);
            binimg.DataMat = QUtils.binarizeImg(QUtils.url2LocalPath(selectedFile),thresh.value);
            binimg.update();
        }
        onRejected: console.log("Operation aborted by user");
    }

    FileDialog {
        id: opfout
        fileMode: FileDialog.SaveFile
        nameFilters: (!modeDec.checked ? ["Zipped Image File (*.zif)"]:["Image Files (*.bmp *.jpeg *.jpg *.png *.gif *.webp)"] )
        onAccepted: {
            //tnam.outputFile =  file;
            set_output_file.text = QUtils.url2LocalPath(selectedFile);
            //console.log("Opened " + file + "for output");
        }
        onRejected: console.log("Operation aborted by user");
    }

    Row{
        width: parent.width
        height: parent.height - op.height - encStat.height - 20
        //spacing: 10

        Column{
            id: col1
            height: parent.height
            width: parent.width / 3
            Text{
                color: Material.foreground
                text: "Original Image"
                anchors.horizontalCenter: parent.horizontalCenter
            }
            Image{
                //anchors.margins: 10
                id: orig
                source: tnam.inputFile
                visible: (modeDec.checked || chbIntSrc.checked) === false
                anchors.horizontalCenter: parent.horizontalCenter
                height: App.judgeDimension(parent.height, parent.width - 10, orig.sourceSize.height,orig.sourceSize.width, 0)
                width:  App.judgeDimension(parent.height, parent.width - 10, orig.sourceSize.height,orig.sourceSize.width, 1)
            }
        }
        Column{
            id: col2
            height: parent.height
            width: parent.width / 3
            Text{
                color: Material.foreground
                text: "Binarized Image"
                anchors.horizontalCenter: parent.horizontalCenter
            }
            QImageMemory{
                id: binimg
                anchors.horizontalCenter: parent.horizontalCenter
                height: App.judgeDimension(parent.height, parent.width - 10, binimg.ImgHeight, binimg.ImgWidth, 0)
                width:  App.judgeDimension(parent.height, parent.width - 10, binimg.ImgHeight, binimg.ImgWidth, 1)
            }

        }
        Column{
            id: col3
            height: parent.height
            width: parent.width / 3

            Text{
                color: Material.foreground
                text: "Decoded Image"
                anchors.horizontalCenter: parent.horizontalCenter
            }
            QImageMemory{
                id: qim
                anchors.horizontalCenter: parent.horizontalCenter
                height: App.judgeDimension(parent.height, parent.width - 10, qim.ImgHeight, qim.ImgWidth, 0)
                width:  App.judgeDimension(parent.height, parent.width - 10, qim.ImgHeight, qim.ImgWidth, 1)
            }
        }

    }
    Row{
        id: op
        anchors.bottom: encStat.top
        //height: parent.height * 0.2
        width:parent.width
        spacing: 30

        Column{
            id: colIO
            spacing: 5
            width:parent.width/2 - 20
            //Layout.alignment: Qt.AlignCenter

            CheckBox{
                height: 20
                id: chbIntSrc
                checked: false
                enabled: modeEnc.checked
                text: "Use Internal Source as image (to be encoded): "

                onCheckedChanged: {
                    tnam.reset();
                    tnam.displayTo(qim);

                    if(chbIntSrc.checked){
                        com.onDownChanged();
                    }else{
                        thresh.onMoved();
                    }
                }
            }
            ComboBox{
                //Layout.alignment: Qt.AlignCenter
                id: com
                enabled: chbIntSrc.checked && modeEnc.checked
                width: parent.width * 0.8
                anchors.horizontalCenter: parent.horizontalCenter

                model: QUtils.listInternalResource("/binimgs/")
                onHighlighted: (index) => {
                                 if(chbIntSrc.checked){
                                     binimg.DataMat = QUtils.getSourceMatFromQRC("/binimgs/" + com.valueAt(index));
                                     binimg.update();
                                 }
                             }

                onDownChanged: {
                    if(chbIntSrc.checked && !com.down){
                        binimg.DataMat = QUtils.getSourceMatFromQRC("/binimgs/" + com.valueAt(currentIndex));
                        binimg.update();
                    }
                }

            }


            Row{
                spacing: 5
                height: parent.height * 0.25
                width: parent.width
                enabled: !(modeEnc.checked && chbIntSrc.checked)

                Text{
                    color: Material.foreground
                    width: parent.width * 0.2
                    anchors.verticalCenter: parent.verticalCenter
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    text: "Input file:"
                }
                TextField{
                    id: set_input_file
                    color: Material.foreground
                    //text: QUtils.url2LocalPath(opfin.file)

                    width: parent.width * 0.8 - setInputFile.width
                    height: parent.height
                    //anchors.fill: parent
                    anchors.margins: 2
                    verticalAlignment: Text.AlignVCenter

                    onTextEdited: {
                        binimg.DataMat = QUtils.binarizeImg(set_input_file.text,thresh.value);
                        binimg.update();
                    }
                }
                Button{
                    id: setInputFile
                    width: parent.height
                    height: parent.height
                    text: "..."
                    onClicked: {
                        opfin.open();
                        thresh.value=0.5;
                    }
                }
            }
            Row{
                spacing: 5
                height: parent.height * 0.25
                width: parent.width

                Text{
                    color: Material.foreground
                    width: parent.width * 0.2
                    anchors.verticalCenter: parent.verticalCenter
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    text: "Output file:"
                }
                TextField{
                    color: Material.foreground
                    id: set_output_file
                    //text:""
                    //anchors.fill: parent
                    anchors.margins: 2
                    width: parent.width * 0.8 - setOutputFile.width
                    height: parent.height
                    verticalAlignment: Text.AlignVCenter
                }

                Button{
                    id: setOutputFile
                    width: parent.height
                    height: parent.height
                    text: "..."
                    onClicked: {
                        opfout.open();
                    }
                }
            }
        }

        Column{
            spacing: 5
            width:parent.width / 2 - 20

            Row{
                id: rowMode
                spacing: 20
                width: parent.width
                RadioButton{
                    id: modeEnc
                    text: "Encode mode"
                    checked: true
                    onToggled: {

                        set_input_file.text = "";
                        set_output_file.text = "";

                        binimg.setEmpty();
                        binimg.update();

                        tnam.reset();
                        tnam.displayTo(qim);
                    }
                }
                RadioButton{
                    id: modeDec
                    text: "Decode mode"

                    onToggled: {

                        set_input_file.text = "";
                        set_output_file.text = "";

                        binimg.setEmpty();
                        binimg.update();

                        tnam.reset();
                        tnam.displayTo(qim);
                    }
                }

            }

            Row{
                spacing: 5
                height: 20
                width: parent.width

                Text{
                    color: Material.foreground
                    text: "Thresh for binarizing: "
                }


                Slider {
                    id: thresh
                    width: parent.width * 0.5
                    enabled: modeEnc.checked & !chbIntSrc.checked & !busy
                    anchors.verticalCenter: parent.verticalCenter

                    from: 0
                    to: 1
                    stepSize: 0.01
                    value: 0.5
                    snapMode: Slider.SnapOnRelease


                    onMoved: {
                        //console.log(opfin.file)
                        binimg.DataMat = QUtils.binarizeImg(set_input_file.text,thresh.value);
                        binimg.update();
                    }
                }

                Text{
                    color: Material.foreground
                    id: thresh_text
                    //anchors.leftMargin: parent.width * 0.5
                    text: (thresh.value * 255).toFixed(0)
                }
            }

            Row{
                id: oprow
                width: parent.width
                RoundButton{

                    id: stEnc
                    text: "Encode"
                    width: parent.width / 3
                    height: this.width * 0.3
                    onClicked: {
                        //tnam.inputFile = "./test/TNAM_test/res.bmp"
                        //console.log("Start Encode...");
                        oprow.enabled = false;
                        rowMode.enabled = false;
                        colIO.enabled = false;
                        encStat.text = "Encoding..."
                        busy = true;
                        //stDec.enabled = false;
                        if(modeEnc.checked){
                            if(chbIntSrc.checked){
                                thresh.value = 0.5;
                                tnam.startEncode(false,QUtils.getSourceMatFromQRC("/binimgs/" + com.currentText,true));
                            }else{
                                //console.log(tnam.binRate);
                                tnam.startEncode();
                            }
                        }else{
                            tnam.startEncode(true);
                        }
                        tnamTimer.start();
                    }
                }
                RoundButton{
                    id: stDec
                    text: "Decode"
                    width: parent.width / 3
                    height: this.width * 0.3
                    onClicked: {
                        oprow.enabled = false;
                        rowMode.enabled = false;
                        colIO.enabled = false;
                        busy = true;
                        encStat.text = "Decoding..."
                        if(modeDec.checked){
                            tnam.startDecode();
                        }else{
                            tnam.startDecode(true);
                        }
                        tnamDecTimer.start();

                    }
                }
                RoundButton{
                    id: save
                    text: "Save"
                    width: parent.width / 3
                    height: this.width * 0.3
                    onClicked: {
                        var ret = 0;
                        busy = true;
                        oprow.enabled = false;
                        rowMode.enabled = false;
                        colIO.enabled = false;
                        if(modeEnc.checked){
                            encStat.text = "Saving encoded data..."
                            ret = tnam.saveEncoded();
                            if(!ret){
                                encStat.text = "Saved to " + set_output_file.text;
                            }else{
                                encStat.text = "Error while saving encoded data: " + QUtils.getErrorMsg(ret);
                            }
                        }else{
                            encStat.text = "Saving decoded image..."
                            ret = tnam.saveDecoded();
                            if(!ret){
                                encStat.text = "Saved to " + set_output_file.text;
                            }else{
                                encStat.text = "Error while saving decoded image!";
                            }

                        }
                        busy = false;
                        oprow.enabled = true;
                        rowMode.enabled = true;
                        colIO.enabled = true;
                    }
                }
            }
            Row{
                spacing: 10
                width: parent.width
                ProgressBar {
                    id: progress
                    from: 0
                    to: 100
                    value: 0

                    anchors.verticalCenter: parent.verticalCenter
                    width: parent.width * 0.8
                }
                Text{
                    color: Material.foreground
                    id: encProg
                    text: "0 %"
                }
            }

        }
    }
    Text{
        color: Material.foreground
        anchors.bottom: parent.bottom
        id:encStat
        height: 20
        text:qsTr("Ready.")
    }

    Timer{
        id:tnamTimer;
        interval: 50;
        repeat:true;
        triggeredOnStart: true;
        onTriggered: {
            encProg.text = (tnam.getEncodeProgress() * 100).toFixed(2) + "%";
            progress.value = (tnam.getEncodeProgress() * 100).toFixed(2);
        }
    }

    Timer{
        id: tnamDecTimer
        interval: 50
        repeat:true;
        triggeredOnStart: true;
        onTriggered: {
            encProg.text = (tnam.getDecodeProgress() * 100).toFixed(2) + "%";
            progress.value = (tnam.getDecodeProgress() * 100).toFixed(2);
        }
    }

    TNAM{
        id: tnam
        binRate: thresh.value
        inputFile: QUtils.localPath2Url(set_input_file.text)
        outputFile: QUtils.localPath2Url(set_output_file.text)
        onEncodeFinished: function(err){
            tnamTimer.stop();
            if(!err){
                progress.value = 100;
                encProg.text = "100 %";
                tnam.getEncodeStat(tnamencs);
                encStat.text = "Encode Result: Found " + tnamencs.TriNum + " Triangles, " + tnamencs.LineNum + " Lines and " + tnamencs.PointNum + " Points";
            }else{
                encStat.text = "Error while encoding: " + QUtils.getErrorMsg(err);
            }
            oprow.enabled = true;
            rowMode.enabled = true;
            colIO.enabled = true;
            busy = false;

        }

        onDecodeFinished: function(err){
            tnamDecTimer.stop();
            if(!err){
                progress.value = 100;
                encProg.text = "100 %"
                tnam.getDecodeStat(tnamdecs);
                encStat.text = "Decode Result: Decoded with " + tnamdecs.TriNum + " Triangles, " + tnamdecs.LineNum + " Lines and " + tnamdecs.PointNum + " Points";

                tnam.displayTo(qim);
                qim.update();
                console.log("Decode done!");
            }else{
                encStat.text = "Error while decoding: " + QUtils.getErrorMsg(err);
            }
            oprow.enabled = true;
            rowMode.enabled = true;
            colIO.enabled = true;
            busy = false;
        }
    }

    TNAM_Stat{
        id: tnamencs
    }
    TNAM_Stat{
        id: tnamdecs
    }
}

