#include "WebcamPanel.h"
#include "WebcamWidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>

WebcamPanel::WebcamPanel(QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(4);

    // Camera selector row
    auto* selectorRow = new QHBoxLayout();
    selectorRow->setContentsMargins(4, 4, 4, 0);

    auto* label = new QLabel("Camera:", this);
    label->setStyleSheet("QLabel { color: #888; font-size: 11px; }");
    selectorRow->addWidget(label);

    cameraCombo_ = new QComboBox(this);
    cameraCombo_->setMinimumWidth(200);
    cameraCombo_->setStyleSheet(
        "QComboBox { background: #1a1a3a; color: #e0e0e0; border: 1px solid #444; "
        "padding: 2px 8px; font-size: 11px; }"
        "QComboBox::drop-down { border: none; }"
        "QComboBox QAbstractItemView { background: #1a1a3a; color: #e0e0e0; "
        "selection-background-color: #3a3a5a; }");
    selectorRow->addWidget(cameraCombo_, 1);

    statusLabel_ = new QLabel(this);
    statusLabel_->setStyleSheet("QLabel { color: #4a8; font-size: 11px; }");
    selectorRow->addWidget(statusLabel_);

    layout->addLayout(selectorRow);

    // Webcam feed
    webcamView_ = new WebcamWidget(this);
    layout->addWidget(webcamView_, 1);

    // Only emit cameraChanged on USER interaction, not programmatic changes
    connect(cameraCombo_, QOverload<int>::of(&QComboBox::activated),
            this, &WebcamPanel::onCameraSelected);
}

void WebcamPanel::setCameras(const std::vector<CameraEntry>& cameras) {
    cameraCombo_->blockSignals(true);
    cameraCombo_->clear();

    if (cameras.empty()) {
        cameraCombo_->addItem("No cameras found", QString());
        statusLabel_->setText("No camera");
        statusLabel_->setStyleSheet("QLabel { color: #f44; font-size: 11px; }");
    } else {
        for (const auto& cam : cameras) {
            cameraCombo_->addItem(
                QString::fromStdString(cam.name),
                QString::fromStdString(cam.device_id));
        }
        statusLabel_->setText(QString("%1 camera(s)").arg(cameras.size()));
        statusLabel_->setStyleSheet("QLabel { color: #4a8; font-size: 11px; }");
    }

    cameraCombo_->blockSignals(false);
}

std::string WebcamPanel::selectedCameraId() const {
    return cameraCombo_->currentData().toString().toStdString();
}

void WebcamPanel::onCameraSelected(int index) {
    if (index < 0) return;
    QString deviceId = cameraCombo_->itemData(index).toString();
    statusLabel_->setText("Switching...");
    statusLabel_->setStyleSheet("QLabel { color: #ff4; font-size: 11px; }");
    emit cameraChanged(deviceId);
}
