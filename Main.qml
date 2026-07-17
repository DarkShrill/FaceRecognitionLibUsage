import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import VideoStream 1.0

ApplicationWindow {
    id: root
    width: 1280
    height: 760
    minimumWidth: 960
    minimumHeight: 620
    visible: true
    title: "FaceRecognition + QVideoStream"
    color: "#17191f"

    Component.onCompleted: recognition.initialize()

    header: ToolBar {
        height: 56
        background: Rectangle { color: "#20242b" }

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 14
            anchors.rightMargin: 14
            spacing: 10

            Label {
                text: "FaceRecognition"
                color: "#f3f0e8"
                font.pixelSize: 18
                font.bold: true
            }

            TextField {
                id: sourceField
                Layout.fillWidth: true
                text: recognition.sourceUrl
                selectByMouse: true
                placeholderText: "video=Integrated Camera oppure video/prova.mp4"
                color: "#f3f0e8"
                placeholderTextColor: "#8f98a3"
                background: Rectangle {
                    color: "#11151b"
                    border.color: sourceField.activeFocus ? "#4fb6a3" : "#3a414b"
                    radius: 4
                }
                onEditingFinished: recognition.sourceUrl = text
            }

            Button {
                text: "Play"
                onClicked: {
                    recognition.sourceUrl = sourceField.text
                    video.start(recognition.sourceUrl)
                }
            }

            Button {
                text: "Stop"
                onClicked: video.stop()
            }

            CheckBox {
                id: cudaBox
                text: "CUDA"
                checked: recognition.useCuda
                contentItem: Text {
                    text: cudaBox.text
                    color: "#f3f0e8"
                    font: cudaBox.font
                    verticalAlignment: Text.AlignVCenter
                    leftPadding: cudaBox.indicator.width + cudaBox.spacing
                }
                onToggled: {
                    recognition.useCuda = checked
                    recognition.initialize()
                }
            }

            ComboBox {
                id: landmarkMode
                Layout.preferredWidth: 130
                textRole: "label"
                valueRole: "value"
                model: [
                    { label: "Nessuno", value: 0 },
                    { label: "5 punti", value: 1 },
                    { label: "106", value: 2 },
                    { label: "Tutti", value: 3 },
                    { label: "3D68", value: 4 }
                ]
                currentIndex: 3
                onActivated: recognition.landmarkMode = currentValue
            }
        }
    }

    Rectangle {
        id: mainSurface
        anchors.fill: parent
        color: "#17191f"

        RowLayout {
            anchors.fill: parent
            spacing: 0

            Item {
                id: videoPane
                Layout.fillWidth: true
                Layout.fillHeight: true

                Rectangle {
                    anchors.fill: parent
                    color: "#05070a"
                }

                VideoStream {
                    id: video
                    anchors.fill: parent
                    forceCpuMode: true
                    onFrameImageReady: function(image) {
                        recognition.submitFrame(image)
                    }
                }

                Canvas {
                    id: overlay
                    anchors.fill: video
                    renderTarget: Canvas.FramebufferObject

                    Connections {
                        target: recognition
                        function onFacesChanged() { overlay.requestPaint() }
                        function onFrameSizeChanged() { overlay.requestPaint() }
                    }

                    onPaint: {
                        var ctx = getContext("2d")
                        ctx.clearRect(0, 0, width, height)

                        if (recognition.frameWidth <= 0 || recognition.frameHeight <= 0) {
                            return
                        }

                        var sx = width / recognition.frameWidth
                        var sy = height / recognition.frameHeight
                        var faces = recognition.faces

                        for (var i = 0; i < faces.length; ++i) {
                            var face = faces[i]
                            var x = face.x * sx
                            var y = face.y * sy
                            var w = face.w * sx
                            var h = face.h * sy

                            ctx.lineWidth = 3
                            ctx.strokeStyle = face.name === "Unknown" ? "#f0b45a" : "#4fb6a3"
                            ctx.strokeRect(x, y, w, h)

                            ctx.font = "14px sans-serif"
                            var label = face.name + " " + Math.round(face.confidence) + "%"
                            var labelWidth = ctx.measureText(label).width + 12
                            var labelY = Math.max(0, y - 24)
                            ctx.fillStyle = "rgba(17, 21, 27, 0.86)"
                            ctx.fillRect(x, labelY, labelWidth, 22)
                            ctx.fillStyle = "#f3f0e8"
                            ctx.fillText(label, x + 6, labelY + 15)

                            drawPoints(ctx, face.landmarks5, sx, sy, "#e9d46a", 3)
                            drawPoints(ctx, face.landmarks106, sx, sy, "#7fc7ff", 1.4)
                            drawPoints(ctx, face.landmarks3d68, sx, sy, "#e879ad", 1.8)
                        }
                    }

                    function drawPoints(ctx, points, sx, sy, color, radius) {
                        if (!points) {
                            return
                        }

                        ctx.fillStyle = color
                        for (var i = 0; i < points.length; ++i) {
                            var p = points[i]
                            ctx.beginPath()
                            ctx.arc(p.x * sx, p.y * sy, radius, 0, Math.PI * 2)
                            ctx.fill()
                        }
                    }
                }

                Rectangle {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                    height: 42
                    color: "#20242b"

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 14
                        anchors.rightMargin: 14
                        spacing: 16

                        Label {
                            text: recognition.statusText
                            color: recognition.initialized ? "#8ed8be" : "#f0b45a"
                            elide: Text.ElideRight
                            Layout.fillWidth: true
                        }

                        Label {
                            text: recognition.summaryText
                            color: "#f3f0e8"
                            horizontalAlignment: Text.AlignRight
                        }
                    }
                }
            }

            Rectangle {
                Layout.preferredWidth: 320
                Layout.fillHeight: true
                color: "#242832"
                border.color: "#343b46"

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 14
                    spacing: 12

                    Label {
                        text: "Rilevazioni"
                        color: "#f3f0e8"
                        font.pixelSize: 18
                        font.bold: true
                    }

                    Label {
                        text: recognition.busy ? "Frame in analisi" : "Pronto per il prossimo frame"
                        color: recognition.busy ? "#f0b45a" : "#8ed8be"
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        height: 1
                        color: "#3a414b"
                    }

                    ListView {
                        id: faceList
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        clip: true
                        spacing: 8
                        model: recognition.faces

                        delegate: Rectangle {
                            width: faceList.width
                            height: 64
                            radius: 6
                            color: modelData.name === "Unknown" ? "#302a22" : "#1f3030"
                            border.color: modelData.name === "Unknown" ? "#9d7442" : "#3d8f82"

                            Column {
                                anchors.fill: parent
                                anchors.margins: 10
                                spacing: 4

                                Text {
                                    width: parent.width
                                    text: modelData.name
                                    color: "#f3f0e8"
                                    font.pixelSize: 15
                                    font.bold: true
                                    elide: Text.ElideRight
                                }

                                Text {
                                    width: parent.width
                                    text: "Confidenza " + modelData.confidence.toFixed(1) + "%"
                                    color: "#bac1c9"
                                    elide: Text.ElideRight
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
