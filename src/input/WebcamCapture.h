#pragma once

#include <functional>
#include <cstdint>
#include <vector>
#include <mutex>
#include <atomic>
#include <memory>

struct FrameData {
    std::vector<uint8_t> pixels;  // RGB, row-major
    int width = 0;
    int height = 0;
    double timestamp = 0.0;       // ms since capture start
};

struct CameraInfo {
    std::string device_id;   // unique AVCaptureDevice ID
    std::string name;        // human-readable name (e.g., "FaceTime HD Camera")
};

class WebcamCapture {
public:
    WebcamCapture(int width = 640, int height = 480, int fps = 30);
    ~WebcamCapture();

    // List available cameras (can be called before start)
    static std::vector<CameraInfo> listCameras();

    // Select a camera by device_id (empty = default). Call before start().
    void selectCamera(const std::string& device_id);

    // Currently selected camera ID (empty = default)
    const std::string& selectedCamera() const { return selected_device_id_; }

    bool start();
    void stop();
    bool isRunning() const;

    // Get latest frame (thread-safe, returns false if no new frame)
    bool getLatestFrame(FrameData& out);

    // Set callback for new frames (called from capture thread)
    void setFrameCallback(std::function<void(const FrameData&)> callback);

    // Called internally from the capture delegate to deliver a new frame
    void deliverFrame(FrameData frame);

private:
    struct Impl;  // Pimpl for Objective-C++ AVFoundation internals
    std::unique_ptr<Impl> impl_;

    int target_width_;
    int target_height_;
    int target_fps_;
    std::string selected_device_id_;

    mutable std::mutex frame_mutex_;
    FrameData latest_frame_;
    std::atomic<bool> has_new_frame_{false};
    std::atomic<bool> running_{false};
    std::function<void(const FrameData&)> frame_callback_;
};
