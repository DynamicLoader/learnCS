import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts
import Qt.labs.platform
import QtQuick.Dialogs
import WrapperRNAM
import QImageMemory
import RNAM_Stat

ApplicationWindow {
    id: window
    height: 800
    maximumHeight: 1080
    maximumWidth: 1920
    minimumHeight: 480
    minimumWidth: 640
    title: qsTr("RNAM Testing tool")
    visible: true
    width: 600

    RowLayout {
        StackLayout {
            height: image.getHeight()
            width: image.getWidth()

            QImageMemory {
                id: baseimage
                Layout.fillHeight: false
                Layout.fillWidth: false
                Layout.preferredHeight: image.height
                Layout.preferredWidth: image.height
                height: image.height
                visible: true
                width: image.width
                z: 0
            }
            QImageMemory {
                id: layover
                Layout.preferredHeight: image.height
                Layout.preferredWidth: image.width
                height: image.height
                visible: false
                width: image.width
                z: 1
            }
        }
        ColumnLayout {
            Layout.alignment: Qt.AlignRight

            ColumnLayout {
                width: parent

                Text {
                    id: caption
                    Layout.alignment: Qt.AlignCenter
                    text: qsTr("RNAM testing program")
                }
                ProgressBar {
                    id: progress
                    Layout.alignment: Qt.AlignLeft
                    Layout.preferredHeight: 20
                    Layout.preferredWidth: 300
                    from: 0
                    to: 100
                    value: 0
                    width: 300
                }
                RowLayout {
                    spacing: 2

                    Text {
                        id: caption_status
                        text: qsTr("STATUS: ")
                    }
                    Text {
                        id: status
                        text: qsTr("Idle")
                    }
                }
                RowLayout {
                    id: encode_control_panel
                    width: parent

                    // anchors.left: parent
                    Button {
                        id: butt_load_image
                        text: qsTr("Load Image")

                        onClicked: {
                            load_image_dia.open();
                            status.text = qsTr("Loading image");
                            baseimage.height = image.getHeight();
                            baseimage.width = image.getWidth();
                            image.displaySource(baseimage);
                        }
                    }
                    Button {
                        id: butt_load_zif
                        text: qsTr("Load ZIF")

                        onClicked: {
                            status.text = qsTr("Loading ZIF");
                            zif_load_dia.open();
                        }
                    }
                    Button {
                        id: start_encode
                        text: qsTr("Encode image")

                        onClicked: {
                            // Junktimer.start
                            progressTimer.start();
                            image.encode(sigmaChooser.value);
                        }
                    }

                    Button {
                        id: terminate
                        text: qsTr("Terminate")

                        onClicked: {
                            image.terminate();
                            status.text = qsTr("Job terminated");

                            progressTimer.stop();
                        }
                    }

                    Text {
                        text: qsTr("Sigma: ")
                    }
                    Slider {
                        id: sigmaChooser
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
                }
                RowLayout {
                    id: decode_control_panel
                    width: parent

                    // anchors.left: Parent
                    Button {
                        id: butt_save_image
                        text: qsTr("Save image to...")

                        onClicked: {
                            save_image_dia.open();
                            status.text = qsTr("Setting save path");
                        }
                    }
                    Button {
                        id: butt_save_zif
                        text: qsTr("Save ZIF to...")

                        onClicked: {
                            zif_save_dia.open();
                            status.text = qsTr("Setting save path");
                        }
                    }
                    Button {
                        id: start_decode
                        text: qsTr("Decode image")

                        onClicked: {
                            status.text = qsTr("Decode start sent");
                            progressTimer.start();
                            image.decode();
                            // Junktimer.start()
                        }
                    }
                }
                RowLayout {
                    id: preprocessor
                    Text {
                        text: qsTr("Preprocess options")
                    }
                    Button {
                        text: qsTr("Clip")

                        onClicked: {
                            image.trimSource();
                            baseimage.height = image.getHeight();
                            baseimage.width = image.getWidth();
                            image.displaySource(baseimage);

                            var statusStr = "Image clipped to " + image.getHeight().toString() + "x" + image.getWidth().toString();
                            status.text = qsTr(statusStr);
                        }
                    }
                    Button {
                        text: qsTr("Add padding")

                        onClicked: {
                            image.addPadding();
                            baseimage.height = image.getHeight();
                            baseimage.width = image.getWidth();
                            image.displaySource(baseimage);
                        }
                    }
                }
                RowLayout {
                    ComboBox {
                        id: statSelect
                        currentIndex: 0
                        model: ["R", "G", "B"]

                        onCurrentIndexChanged: {
                        }
                    }
                    Button {
                        id: loadStat
                        text: qsTr("Load Stat")

                        onClicked: {
                            image.loadStat(statSelect.currentIndex, stat);
                            var bpp = stat.BPP;
                            var blocksize = stat.BlockCount;
                            var statusStr = "Stat loaded, bpp: " + bpp.toString() + ", blocksize: " + blocksize.toString();
                            status.text = qsTr(statusStr);
                        }
                    }
                    RNAM_Stat {
                        id: stat
                    }
                }
                RowLayout {
                    Button {
                        id: showdiff
                        text: qsTr("Toggle difference")

                        onClicked: {
                            image.displayDiff(layover);
                            layover.visible = true;
                        }
                    }
                    Text {
                        text: qsTr("Display Segmentation of Channel: ")
                    }
                    ComboBox {
                        id: chanSelect
                        currentIndex: 0
                        model: ["R", "G", "B"]

                        onCurrentIndexChanged: {
                            status.text = qsTr("Channel changed to ") + chanSelect.currentText.toString();
                            image.displaySegment(layover, chanSelect.currentIndex);
                            layover.visible = true;
                        }
                    }
                    Button {
                        id: notshowdiff
                        text: qsTr("Display result")

                        onClicked: {
                            image.displayResult();
                            layover.visible = false;
                        }
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
                }
                Timer {
                    id: progressTimer
                    interval: 20
                    repeat: true
                    running: false

                    onTriggered: {
                        var statusStr = "Encoded " + image.getProgress().toString() + "%";
                        status.text = qsTr(statusStr);
                        progress.value = image.getProgress();
                    }
                }
                FileDialog {
                    id: load_image_dia
                    fileMode: FileDialog.OpenFile

                    onAccepted: {
                        image.setSourceImagePath(selectedFile);
                        image.loadImage();
                        console.log(baseimage.width, baseimage.height);
                        var statusStr = "Loaded " + selectedFile + " as source [" + image.getWidth() + "x" + image.getHeight() + "]";
                        status.text = qsTr(statusStr);
                        image.displaySource(baseimage);
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
                    id: zif_load_dia
                    fileMode: FileDialog.OpenFile

                    onAccepted: {
                        image.setZifInputPath(selectedFile);
                        var statusStr = "Loaded " + selectedFile + " as ZIF";
                        image.readZif();
                        status.text = qsTr(statusStr);
                    }
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
            }
            WrapperRNAM {
                id: image
                onDecodeFinished: {
                    progress.value = 100;
                    progressTimer.stop();
                    var statusStr = "Decode Finished in ";
                    image.displayResult(baseimage);
                    baseimage.update();
                }
                onEncodeFinished: {
                    progress.value = 100;
                    progressTimer.stop();
                    var statusStr = "Encode Finished in ";
                    status.text = qsTr(statusStr);
                }
            }
        }
    }
}
