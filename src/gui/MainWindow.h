#pragma once

#include <QMainWindow>
#include <QTimer>
#include <QLabel>
#include <memory>

namespace biobrain { class Simulation; class BrainRegion; }

class RegionTreeWidget;
class SpikeRasterWidget;
class ActivityMapWidget;
class WebcamWidget;
class WebcamPanel;
class BackendConfigPanel;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(std::shared_ptr<biobrain::Simulation> sim,
                        QWidget* parent = nullptr);
    ~MainWindow() override;

    // Access webcam widget for external frame delivery
    WebcamWidget* webcamWidget() const;
    WebcamPanel* webcamPanel() const { return webcamPanel_; }

private slots:
    void onRun();
    void onPause();
    void onStop();
    void onRegionSelected(uint32_t region_id);
    void updateStatusBar();

private:
    void setupMenus();
    void setupToolbar();
    void setupDocks();
    void setupStatusBar();
    void applyDarkTheme();

    std::shared_ptr<biobrain::Simulation> simulation_;

    RegionTreeWidget*   regionTree_   = nullptr;
    SpikeRasterWidget*  spikeRaster_  = nullptr;
    ActivityMapWidget*  activityMap_  = nullptr;
    WebcamPanel*        webcamPanel_  = nullptr;
    BackendConfigPanel* configPanel_  = nullptr;

    QTimer* statusTimer_ = nullptr;

    // Status bar labels
    QLabel* timeLabel_      = nullptr;
    QLabel* activeLabel_    = nullptr;
    QLabel* gpuLabel_       = nullptr;
    QLabel* spikeRateLabel_ = nullptr;
};
