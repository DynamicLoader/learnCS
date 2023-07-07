import QtQuick 2.6
import QtQuick.Window 2.2
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.15
import Qt.labs.platform 1.1
import QtQuick.Dialogs
import WrapperRNAM 1.0
import QImageMemory 1.0
import QUtils 1.0
import RNAM_Stat 1.0
import QtQuick.Controls.Material 2.2
import "handler/app.js" as App

Item {
    id: name

    property int idx
    property bool busy: false
    property int time_elapsed: 0
    ;

    function terminate() {
        terminate.clicked();
        return;
    }

    Item {
        FileDialog {
            id: load_image_dia
            fileMode: FileDialog.OpenFile

            onAccepted: {
                terminate.clicked();

                if ((QUtils.imageSuffixChecker(selectedFile) === 1)) {
                    image.setSourceImagePath(selectedFile);
                    image.loadImage();
                    console.log(baseimage.width, baseimage.height);
                    var statusStr = "Loaded " + selectedFile + " as source [" + image.getWidth() + "x" + image.getHeight() + "]";
                    status.text = qsTr(statusStr);
                    imageStack.height = App.judgeDimension(container.height, container.width, image.height, image.width, 0);
                    imageStack.width = App.judgeDimension(container.height, container.width, image.height, image.width, 1);
                    image.displaySource(baseimage);
                    console.log(imageStack.width, imageStack.height);
                } else if ((QUtils.imageSuffixChecker(selectedFile) === -1)) {
                    image.setZifInputPath(selectedFile);
                    var statusStr = "Loaded " + selectedFile + " as ZIF";
                    image.readZif();
                    status.text = qsTr(statusStr);
                    imageStack.height = App.judgeDimension(container.height, container.width, image.height, image.width, 0);
                    imageStack.width = App.judgeDimension(container.height, container.width, image.height, image.width, 1);
                    start_decode.clicked();
                }

                butt_save_zif.enabled = false;
                chanSelect.enabled = false;
                clip.enabled = padd.enabled = grey.enabled = true;
            }
            onRejected: status.text = qsTr("Open discarded")
        }
        FileDialog {
            id: save_image_dia
            fileMode: FileDialog.SaveFile

            onAccepted: {
                image.setResultSavePath(selectedFile);
                var statusStr = "Output image setted to " + selectedFile;
                status.text = qsTr(statusStr);
                image.saveDecodeResult();
            }
            onRejected: status.text = qsTr("Save discarded")
        }
        FileDialog {
            id: zif_save_dia
            fileMode: FileDialog.SaveFile

            onAccepted: {
                image.setZifOutPath(selectedFile);
                var statusStr = "Output ZIF setted to " + selectedFile;
                image.writeZif();
                status.text = qsTr(statusStr);
            }
        }
        ColorDialog {
            id: vis_color_1
            onAccepted: {
                image.visualColor1 = selectedColor;
                colPreview1.Material.background = selectedColor;
                image.displaySegment(layover, chanSelect.currentIndex);
                layover.visible = true;
                layover.opacity = opaqueSelecter.value;
            }
        }
        ColorDialog {
            id: vis_color_2
            onAccepted: {
                image.visualColor2 = selectedColor;
                colPreview2.Material.background = selectedColor;
                image.displaySegment(layover, chanSelect.currentIndex);
                layover.visible = true;
                layover.opacity = opaqueSelecter.value;
            }
        }
        Timer {
            id: progressTimer
            interval: 20
            repeat: true
            running: false

            onTriggered: {
                progress.value = image.getProgress();
                progressText.text = image.getProgress().toString() + "%";
                busy = true;

                time_elapsed += 20;
            }
        }
        RNAM_Stat {
            id: imageStat
        }
    }
    ColumnLayout {
        anchors.fill: parent
        height: parent.height
        width: parent.width

        RowLayout {
            Layout.fillHeight: true
            Layout.fillWidth: true

            ColumnLayout {
                Layout.fillHeight: true
                Layout.fillWidth: false
                Layout.preferredWidth: 330
                Layout.alignment: Qt.AlignTop

                Pane {
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignHCenter
                    Material.elevation: 6
                    Material.background: Material.color(Material.Teal)
                    Label {
                        text: qsTr("File operation")
                    }
                }
                RoundButton {
                    id: butt_load_image
                    text: qsTr("Load Image")
                    Layout.fillWidth: true

                    onClicked: {
                        load_image_dia.open();
                        status.text = qsTr("Loading image");
                    }
                }
                RowLayout {
                    Layout.fillWidth: true
                    RoundButton {
                        id: butt_save_image
                        text: qsTr("Save image")
                        Layout.fillWidth: true

                        onClicked: {
                            save_image_dia.open();
                            status.text = qsTr("Setting save path");
                        }
                    }
                    RoundButton {
                        id: butt_save_zif
                        text: qsTr("Save ZIF")
                        Layout.fillWidth: true
                        enabled: false;

                        onClicked: {
                            zif_save_dia.open();
                            status.text = qsTr("Setting save path");
                        }
                    }
                }
                Pane {
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignHCenter
                    Material.elevation: 6
                    Material.background: Material.color(Material.Teal)
                    Label {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Preprocess options")
                    }
                }
                RowLayout {
                    Layout.fillWidth: true

                    RoundButton {
                        id: clip
                        text: qsTr("Clip")
                        Layout.fillWidth: true

                        onClicked: {
                            image.trimSource();
                            image.displaySource(baseimage);
                            var statusStr = "Image clipped to " + image.getHeight().toString() + "x" + image.getWidth().toString();
                            status.text = qsTr(statusStr);
                            imageStack.height = App.judgeDimension(container.height, container.width, image.height, image.width, 0);
                            imageStack.width = App.judgeDimension(container.height, container.width, image.height, image.width, 1);
                        }
                    }
                    RoundButton {
                        id: padd
                        text: qsTr("Add padding")
                        Layout.fillWidth: true

                        onClicked: {
                            image.addPadding();
                            image.displaySource(baseimage);
                            imageStack.height = App.judgeDimension(container.height, container.width, image.height, image.width, 0);
                            imageStack.width = App.judgeDimension(container.height, container.width, image.height, image.width, 1);
                        }
                    }
                    RoundButton {
                        id: grey
                        text: "To grey"
                        Layout.fillWidth: true

                        onClicked: {
                            image.convertSourceToGray();
                            image.displaySource(baseimage);
                            imageStack.height = App.judgeDimension(container.height, container.width, image.height, image.width, 0);
                            imageStack.width = App.judgeDimension(container.height, container.width, image.height, image.width, 1);
                        }
                    }
                }
                Pane {
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignHCenter
                    Material.elevation: 6
                    Material.background: Material.color(Material.Teal)
                    Label {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Codec options")
                    }
                }


                Label {
                    text: "Rect. build algorithm:"
                }
                ComboBox {
                    id: methodSelect
                    Layout.fillWidth: true
                    currentIndex: 0
                    model: ["Horizonal Priority", "Diagonal Priority"]
                }
                Label {
                    Layout.alignment: Qt.AlignLeft
                    text: qsTr("Sigma: ")
                }
                Slider {
                    id: sigmaChooser
                    Layout.fillWidth: true
                    from: 1
                    snapMode: Slider.SnapOnRelease
                    stepSize: 1
                    to: 100
                    value: 24

                    onMoved: {
                        var statusStr = "_sigma changed to " + sigmaChooser.value.toString();
                        status.text = qsTr(statusStr);
                    }
                }
                RowLayout {
                    Layout.fillWidth: true

                    RoundButton {
                        id: start_encode
                        text: qsTr("Encode")
                        Layout.fillWidth: true

                        onClicked: {
                            clip.enabled = padd.enabled = grey.enabled = false;
                            time_elapsed = 0;
                            progressTimer.start();

                            layover.visible = false;
                            start_encode.enabled = false;
                            butt_save_zif.enabled = false;
                            deSBencode.restart();

                            image.encode(sigmaChooser.value, methodSelect.currentIndex + 1);
                        }

                        Timer {
                            id: deSBencode
                            interval: 500
                            repeat: false

                            onTriggered: {
                                start_encode.enabled = true;
                            }
                        }
                    }
                    RoundButton {
                        id: start_decode
                        text: qsTr("Decode")
                        Layout.fillWidth: true

                        onClicked: {
                            clip.enabled = padd.enabled = grey.enabled = false;
                            start_decode.enabled = false;


                            deSBdecode.restart();
                            image.decode();
                        }

                        Timer {
                            id: deSBdecode
                            interval: 500
                            repeat: false

                            onTriggered: {
                                start_decode.enabled = true;
                            }
                        }
                    }

                    RoundButton {
                        id: terminate
                        text: qsTr("\u00d7")
                        Layout.fillWidth: false

                        Material.foreground: Material.White
                        Material.background: Material.Red

                        onClicked: {
                            image.terminate();
                            progressTimer.stop();
                            clip.enabled = padd.enabled = grey.enabled = true;
                            busy = false;
                            progress.value = 0;
                            status.text = "Job aborted.";
                            progressText.text = "Idle"
                        }
                    }
                }
                CheckBox {
                    id: livepre
                    text: qsTr("Live preview mode")
                }
                Pane {
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignHCenter
                    Material.elevation: 6
                    Material.background: Material.color(Material.Teal)
                    Label {
                        Layout.alignment: Qt.AlignHCenter
                        text: "Info overlay"
                    }
                }
                RowLayout {
                    RadioButton {
                        id: showdiff
                        text: qsTr("Difference")
                        checked: false

                        onClicked: {
                            image.displayDiff(layover);
                            layover.visible = true;
                            layover.opacity = opaqueSelecter.value;
                            showseg.checked = false;
                            nolayover.checked = false;
                        }
                    }
                    RadioButton {
                        id: nolayover
                        text: qsTr("None")
                        checked: true

                        onClicked: {
                            layover.visible = false;
                            showseg.checked = false;
                            showdiff.checked = false;
                        }
                    }
                }
                // Text {
                //     text: qsTr("Display Segmentation of Channel: ")
                // }
                RowLayout {
                    RadioButton {
                        id: showseg
                        text: "Segment of Channel"

                        onClicked: {
                            showdiff.checked = false;
                            nolayover.checked = false;
                            image.displaySegment(layover, 2 - chanSelect.currentIndex);
                            layover.visible = true;
                            layover.opacity = opaqueSelecter.value;
                            image.loadStat(2 - chanSelect.currentIndex, imageStat);
                            if (imageStat.BlockCount !== 0) {
                                var statusStr = "Channel " + chanSelect.currentText.toString() + " have " + imageStat.BlockCount.toString() + " rectangles. Stats:" + "  BPP: " + imageStat.BPP.toString() + "  CR:  " + imageStat.CR.toString() + "  PSNR:  " + imageStat.PSNR.toString();
                                status.text = statusStr;
                            }
                        }
                    }
                    ComboBox {
                        id: chanSelect
                        currentIndex: 0
                        enabled: false
                        model: ["R", "G", "B"]

                        onActivated: {
                            showseg.checked = true;
                            nolayover.checked = false;
                            showdiff.checked = false;
                            image.displaySegment(layover, 2 - chanSelect.currentIndex);
                            layover.visible = true;
                            layover.opacity = opaqueSelecter.value;
                            image.loadStat(2 - chanSelect.currentIndex, imageStat);
                            if (imageStat.BlockCount !== 0) {
                                var statusStr = "Channel " + chanSelect.currentText.toString() + " have " + imageStat.BlockCount.toString() + " rectangles. Stats:" + "  BPP: " + imageStat.BPP.toString() + "  CR:  " + imageStat.CR.toString() + "  PSNR:  " + imageStat.PSNR.toString();
                                status.text = statusStr;
                            }
                        }
                    }
                }
            }
            Rectangle {
                id: container
                Layout.fillHeight: true
                Layout.fillWidth: true
                color: "grey"
                visible: true

                onHeightChanged: {
                    imageStack.height = App.judgeDimension(container.height, container.width, image.height, image.width, 0);
                    imageStack.width = App.judgeDimension(container.height, container.width, image.height, image.width, 1);
                }
                onWidthChanged: {
                    imageStack.height = App.judgeDimension(container.height, container.width, image.height, image.width, 0);
                    imageStack.width = App.judgeDimension(container.height, container.width, image.height, image.width, 1);
                }

                DropArea {
                    id: imageDrop
                    anchors.fill: parent

                    onDropped: function (_drop) {
                        console.log("Dropped", _drop.urls);
                        if ((QUtils.imageSuffixChecker(_drop.urls) === 1)) {
                            image.setSourceImagePath(_drop.urls);
                            image.loadImage();
                            console.log(baseimage.width, baseimage.height);
                            var statusStr = "Loaded " + _drop.urls + " as source [" + image.getWidth() + "x" + image.getHeight() + "]";
                            status.text = qsTr(statusStr);
                            imageStack.height = App.judgeDimension(container.height, container.width, image.height, image.width, 0);
                            imageStack.width = App.judgeDimension(container.height, container.width, image.height, image.width, 1);
                            image.displaySource(baseimage);
                            console.log(imageStack.width, imageStack.height);
                        } else if ((QUtils.imageSuffixChecker(_drop.urls) === -1)) {
                            image.setZifInputPath(_drop.urls);
                            var statusStr = "Loaded " + _drop.urls + " as ZIF";
                            image.readZif();
                            status.text = qsTr(statusStr);
                            imageStack.height = App.judgeDimension(container.height, container.width, image.height, image.width, 0);
                            imageStack.width = App.judgeDimension(container.height, container.width, image.height, image.width, 1);
                            start_decode.clicked();
                        }
                        chanSelect.enabled = false;
                        baseimage.opacity = 1;
                        dropIndic.opacity = 0;

                        clip.enabled = padd.enabled = grey.enabled = true;
                        butt_save_zif.enabled = false;
                    }
                    onEntered: {
                        layover.visible = false;
                        dropIndic.opacity = 0.5;
                    }
                    onExited: {
                        dropIndic.opacity = 0;
                    }
                }
                StackLayout {
                    id: imageStack
                    anchors.centerIn: parent

                    QImageMemory {
                        id: baseimage
                        anchors.fill: parent
                        visible: true
                        z: 10
                    }
                    QImageMemory {
                        id: layover
                        anchors.fill: parent
                        visible: false
                        z: 20
                    }
                }

                Rectangle {
                    id: dropIndic
                    anchors.fill: parent
                    color: "white"
                    opacity: 0
                    visible: true
                    z: 21

                    TextArea {
                        color: "black"
                        id: dropIndicText
                        readOnly: true
                        enabled: false
                        anchors.centerIn: parent
                        background: Rectangle {
                            color: "transparent"
                        }
                        text: qsTr("Drop image here")
                        font.pixelSize: 40
                    }
                }
            }
        }
        RowLayout {
            anchors.left: parent.left
            anchors.right: parent.right

            Label {
                Layout.alignment: Qt.AlignLeft
                text: qsTr("Overlay opacity ")
            }
            Slider {
                id: opaqueSelecter
                from: 0
                snapMode: Slider.SnapOnRelease
                stepSize: 1
                to: 100
                value: 50

                onMoved: {
                    layover.opacity = opaqueSelecter.value / 100;
                }
            }
            RowLayout {
                id: colorSelect
                anchors.right: parent.right

                Label {
                    Layout.alignment: Qt.AlignLeft
                    text: qsTr("Segment color scheme ")
                }

                RoundButton {
                    id: colPreview1
                    Material.background: image.visualColor1
                    height: colcho.height
                    width: colcho.height

                    onClicked: {
                        vis_color_1.open();
                    }
                }
                RoundButton {
                    id: colPreview2
                    Material.background: image.visualColor2
                    height: colcho.height
                    width: colcho.height

                    onClicked: {
                        vis_color_2.open();
                    }
                }

            }
        }
        Label {
            id: status
            Layout.fillWidth: true
            text: "Tips: use drag and drop to load image."
            wrapMode: Text.WordWrap
        }
        RowLayout {
            Layout.alignment: Qt.AlignBottom
            Layout.fillWidth: true

            ProgressBar {
                id: progress
                Layout.fillWidth: true
                Layout.preferredHeight: 35

                from: 0
                to: 100
                value: 0
                z: 0
            }

            Label {
                id: progressText
                Layout.alignment: Qt.AlignRight
                text: "Idle"
            }
        }
    }

    RNAM {
        id: image
        onDecodeFinished: {
            progress.value = 100;
            progressTimer.stop();
            image.displayResult(baseimage);
            baseimage.update();
            status.text = "Decode Finished";
            layover.visible = false;
            busy = false;
            butt_save_zif.enabled = true;
            clip.enabled = padd.enabled = grey.enabled = true;
        }
        onEncodeFinished: {
            progressTimer.stop();
            var statusStr = "Encode finished in " + time_elapsed.toString() + " ms.";
            progress.value = 100;
            status.text = qsTr(statusStr);
            progressText.text = "Done";
            if (livepre.checked) {
                start_decode.clicked();
            }
            chanSelect.enabled = true;
            clip.enabled = padd.enabled = grey.enabled = true;
            busy = false;
        }
        onError: function (num) {
            console.log("Err ", num)
            status.text = QUtils.getErrorMsg(num);
            progress.value = 0;
            progressText.text = "Error";

            if (num !== -221) {
                busy = false;
                clip.enabled = padd.enabled = grey.enabled = true;
            }
        }
    }
}
