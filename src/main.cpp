#include <QApplication>
#include <QTimer>
#include <iostream>
#include <memory>
#include <thread>
#include <csignal>

// Core
#include "core/Simulation.h"
#include "core/BrainRegion.h"
#include "core/SpikeEvent.h"

// Compute backends
#include "compute/CPUBackend.h"
#include "compute/MetalBackend.h"

// Plasticity
#include "plasticity/DopamineSTDP.h"

// Input
#include "input/WebcamCapture.h"
#include "input/RetinalEncoder.h"

// Brain regions
#include "regions/Retina.h"
#include "regions/LGN.h"
#include "regions/V1.h"
#include "regions/V2V4.h"
#include "regions/ITCortex.h"
#include "regions/VTA.h"
#include "regions/Striatum.h"
#include "regions/MotorCortex.h"

// Recording
#include "recording/SpikeRecorder.h"

// GUI
#include "gui/MainWindow.h"
#include "gui/WebcamWidget.h"
#include "gui/WebcamPanel.h"
#include "gui/SpikeRasterWidget.h"

// Debug API
#include "harness/DebugAPI.h"

using namespace biobrain;

static std::atomic<bool> g_shutdown{false};

static void signalHandler(int) {
    g_shutdown.store(true);
}

// Build the complete brain circuit with all 9 regions wired together.
static std::shared_ptr<Simulation> buildBrain() {
    auto sim = std::make_shared<Simulation>();

    // Create compute backends
    auto cpu = std::make_shared<CPUBackend>(6);
    auto metal = std::make_shared<MetalBackend>();

    // Use Metal for large visual cortex regions, CPU for smaller RL regions
    auto visualBackend = metal->isAvailable()
        ? std::static_pointer_cast<ComputeBackend>(metal)
        : std::static_pointer_cast<ComputeBackend>(cpu);
    auto rlBackend = cpu;

    // Create plasticity rule (STDP + Dopamine for all regions)
    auto plasticity = std::make_shared<DopamineSTDP>();

    // --- Create brain regions with cumulative neuron ID offsets ---
    uint32_t offset = 0;

    auto retina = Retina::create(offset);
    retina->setComputeBackend(cpu);
    offset += Retina::NEURON_COUNT;

    auto lgn = LGN::create(offset);
    lgn->setComputeBackend(visualBackend);
    lgn->setPlasticityRule(plasticity);
    offset += LGN::NEURON_COUNT;

    auto v1 = V1::create(offset);
    v1->setComputeBackend(visualBackend);
    v1->setPlasticityRule(plasticity);
    offset += V1::NEURON_COUNT;

    auto v2v4 = V2V4::create(offset);
    v2v4->setComputeBackend(visualBackend);
    v2v4->setPlasticityRule(plasticity);
    offset += V2V4::NEURON_COUNT;

    auto it = ITCortex::create(offset);
    it->setComputeBackend(visualBackend);
    it->setPlasticityRule(plasticity);
    offset += ITCortex::NEURON_COUNT;

    auto vta = VTA::create(offset);
    vta->setComputeBackend(rlBackend);
    offset += VTA::NEURON_COUNT;

    auto striatum = Striatum::create(offset);
    striatum->setComputeBackend(rlBackend);
    striatum->setPlasticityRule(plasticity);
    offset += Striatum::NEURON_COUNT;

    auto motor = MotorCortex::create(offset);
    motor->setComputeBackend(rlBackend);
    offset += MotorCortex::NEURON_COUNT;

    std::cout << "BioBrain: " << offset << " total neurons across 8 regions\n";
    std::cout << "  Compute: " << (metal->isAvailable() ? "Metal GPU" : "CPU-only")
              << " for visual cortex, CPU for RL loop\n";

    // Add all regions to simulation
    sim->addRegion(retina);
    sim->addRegion(lgn);
    sim->addRegion(v1);
    sim->addRegion(v2v4);
    sim->addRegion(it);
    sim->addRegion(vta);
    sim->addRegion(striatum);
    sim->addRegion(motor);

    return sim;
}

int main(int argc, char* argv[]) {
    // Handle Ctrl+C gracefully
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    // Log to file for crash debugging
    freopen("/tmp/biobrain.log", "w", stderr);
    std::cerr << "BioBrain starting...\n";

    QApplication app(argc, argv);
    app.setApplicationName("BioBrain");
    app.setApplicationVersion("0.1.0");

    // Build neural circuit
    std::cout << "Building brain...\n";
    auto simulation = buildBrain();

    // Set up spike recorder
    auto recorder = std::make_shared<SpikeRecorder>("biobrain_spikes.h5");
    for (auto& region : simulation->regions()) {
        recorder->enableRegion(region->id());
    }
    simulation->setRecorder(recorder);

    // Set up webcam and retinal encoder
    auto webcam = std::make_unique<WebcamCapture>(640, 480, 30);
    auto encoder = std::make_unique<RetinalEncoder>(64);  // 64x64 = 8192 RGCs

    // Create GUI first so we can wire webcam to it
    auto mainWindow = std::make_unique<MainWindow>(simulation);
    mainWindow->setWindowTitle("BioBrain — Neural Simulation Dashboard");
    mainWindow->resize(1400, 900);
    mainWindow->show();

    // Wire camera switch handler
    auto* webcamPtr = webcam.get();
    auto* panelPtr = mainWindow->webcamPanel();
    if (panelPtr) {
        QObject::connect(panelPtr, &WebcamPanel::cameraChanged,
            [webcamPtr](const QString& deviceId) {
                std::cerr << "Switching camera to: " << deviceId.toStdString() << "\n";
                webcamPtr->stop();
                QTimer::singleShot(500, [webcamPtr, deviceId]() {
                    webcamPtr->selectCamera(deviceId.toStdString());
                    if (webcamPtr->start()) {
                        std::cerr << "Camera switched successfully\n";
                    }
                });
            });
    }

    // Wire webcam → retinal encoder → simulation AND → GUI webcam widget
    auto* encoderPtr = encoder.get();
    auto simWeakPtr = std::weak_ptr<Simulation>(simulation);
    auto* webcamWidget = mainWindow->webcamWidget();

    webcam->setFrameCallback([encoderPtr, simWeakPtr, webcamWidget](const FrameData& frame) {
        // Feed the GUI webcam display (works even when sim is stopped)
        if (webcamWidget && !frame.pixels.empty()) {
            webcamWidget->updateFrame(frame.pixels.data(), frame.width, frame.height);
        }

        // Feed retinal encoder → simulation (only when running)
        auto sim = simWeakPtr.lock();
        if (!sim || !sim->isRunning()) return;

        double t_start = sim->currentTime();
        double t_end = t_start + 33.3;  // 30fps = 33.3ms per frame

        SpikeOutput retinalSpikes = encoderPtr->encode(
            frame.pixels.data(), frame.width, frame.height, t_start, t_end);

        // Convert retinal spike output to SpikeEvents targeting LGN
        std::vector<SpikeEvent> events;
        events.reserve(retinalSpikes.neuron_ids.size());
        for (size_t i = 0; i < retinalSpikes.neuron_ids.size(); ++i) {
            SpikeEvent ev;
            ev.source_id = retinalSpikes.neuron_ids[i];
            ev.target_id = retinalSpikes.neuron_ids[i];  // 1:1 topographic
            ev.time = retinalSpikes.spike_times[i];
            ev.delay = 2.0;  // optic nerve delay (ms)
            ev.source_region = Retina::REGION_ID;
            ev.target_region = LGN::REGION_ID;
            events.push_back(ev);
        }

        sim->injectSpikes(events);
    });

    // Start webcam after event loop is running (so permission dialog can display).
    // Then populate the camera list once we know permission status.
    QTimer::singleShot(500, [webcamPtr, panelPtr]() {
        bool started = webcamPtr->start();
        if (started) {
            std::cerr << "Webcam started (640x480 @ 30fps)\n";
        } else {
            std::cerr << "WARNING: Could not start webcam. If permission dialog appeared, "
                         "it will auto-retry. Otherwise check System Settings > Privacy > Camera.\n";
        }

        // Populate camera list (works regardless of permission for listing)
        auto cameras = WebcamCapture::listCameras();
        std::cerr << "Found " << cameras.size() << " camera(s)\n";
        if (panelPtr) {
            std::vector<WebcamPanel::CameraEntry> entries;
            for (auto& cam : cameras) {
                std::cerr << "  Camera: " << cam.name << " [" << cam.device_id << "]\n";
                entries.push_back({cam.device_id, cam.name});
            }
            panelPtr->setCameras(entries);
        }
    });

    // Debug REST API on port 9090
    auto debugApi = std::make_unique<DebugAPI>(simulation, webcam.get(),
                                                mainWindow.get(), 9090);

    // Wire spike callback to both GUI and debug API
    auto* debugApiPtr = debugApi.get();
    simulation->setSpikeCallback(
        [&mainWindow, debugApiPtr](uint32_t region_id,
                                    const std::vector<uint32_t>& neuron_ids,
                                    const std::vector<double>& times) {
            // Feed GUI spike raster
            auto* raster = mainWindow->findChild<SpikeRasterWidget*>();
            if (raster) {
                QMetaObject::invokeMethod(raster, [raster, neuron_ids, times]() {
                    raster->addSpikes(neuron_ids, times);
                }, Qt::QueuedConnection);
            }
            // Feed debug API activity log
            debugApiPtr->recordSpikeBatch(region_id, neuron_ids.size(),
                                           times.empty() ? 0 : times.back());
        });

    debugApi->start();
    std::cout << "Debug API: http://localhost:9090\n";

    // Auto-start the simulation
    simulation->start();
    recorder->start();
    std::cerr << "Simulation auto-started.\n";

    // Periodic stimulus: inject random spikes into Retina every 100ms
    // to drive visible activity through the visual cortex pipeline
    QTimer stimulusTimer;
    auto simWeak2 = std::weak_ptr<Simulation>(simulation);
    QObject::connect(&stimulusTimer, &QTimer::timeout, [simWeak2]() {
        auto sim = simWeak2.lock();
        if (!sim || !sim->isRunning()) return;

        double t = sim->currentTime();
        std::vector<SpikeEvent> events;
        // Inject 500 spikes spread across retinal neurons
        for (uint32_t i = 0; i < 500; ++i) {
            SpikeEvent ev;
            ev.source_id = (i * 17) % 8192;  // pseudo-random retinal neurons
            ev.target_id = ev.source_id;
            ev.time = t + (i * 0.05);
            ev.delay = 2.0;
            ev.source_region = 0;  // Retina
            ev.target_region = 1;  // LGN
            events.push_back(ev);
        }
        sim->injectSpikes(events);
    });
    stimulusTimer.start(100);  // every 100ms

    // Handle shutdown signal
    QTimer shutdownTimer;
    QObject::connect(&shutdownTimer, &QTimer::timeout, [&]() {
        if (g_shutdown.load()) {
            simulation->stop();
            recorder->stop();
            webcam->stop();
            app.quit();
        }
    });
    shutdownTimer.start(100);

    std::cout << "BioBrain ready. Press Run to start simulation.\n";

    int result = app.exec();

    // Cleanup
    debugApi->stop();
    simulation->stop();
    recorder->stop();
    webcam->stop();

    std::cout << "BioBrain shutdown. Recorded " << recorder->totalSpikes() << " spikes.\n";

    return result;
}
