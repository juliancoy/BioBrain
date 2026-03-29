#pragma once

#include <QWidget>
#include <QComboBox>
#include <QLabel>
#include <string>
#include <vector>
#include <functional>

class WebcamWidget;

// Panel containing a camera selector dropdown and the webcam feed display.
class WebcamPanel : public QWidget {
    Q_OBJECT
public:
    explicit WebcamPanel(QWidget* parent = nullptr);

    WebcamWidget* webcamWidget() const { return webcamView_; }

    // Populate camera list (call after WebcamCapture::listCameras)
    struct CameraEntry {
        std::string device_id;
        std::string name;
    };
    void setCameras(const std::vector<CameraEntry>& cameras);

    // Currently selected camera device ID (empty = default)
    std::string selectedCameraId() const;

signals:
    void cameraChanged(const QString& device_id);

private slots:
    void onCameraSelected(int index);

private:
    QComboBox* cameraCombo_;
    WebcamWidget* webcamView_;
    QLabel* statusLabel_;
};
