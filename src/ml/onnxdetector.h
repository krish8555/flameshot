// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Contributors

#pragma once

#include "objectdetector.h"
#include <memory>

#ifdef ENABLE_ONNX_ML
#include <onnxruntime_cxx_api.h>
#endif

// ONNX Runtime implementation of object detection
class OnnxDetector : public ObjectDetector
{
    Q_OBJECT

public:
    explicit OnnxDetector(QObject* parent = nullptr);
    ~OnnxDetector() override;

    bool initialize(const QString& modelPath) override;
    bool isInitialized() const override;
    QVector<DetectedObject> detect(const QImage& image,
                                    float confidenceThreshold = 0.5f) override;
    QStringList getSupportedClasses() const override;
    QString getBackendName() const override;

    // Set input size for model (default 640x640 for YOLO models)
    void setInputSize(int width, int height);

    // Enable GPU acceleration (if available)
    void enableGPU(bool enable);

private:
#ifdef ENABLE_ONNX_ML
    // Preprocess image for model input
    std::vector<float> preprocessImage(const QImage& image);

    // Postprocess model output to extract detections
    QVector<DetectedObject> postprocessOutput(const std::vector<float>& output,
                                               const QSize& originalSize,
                                               float confidenceThreshold);

    // Non-maximum suppression to filter overlapping boxes
    QVector<DetectedObject> applyNMS(const QVector<DetectedObject>& detections,
                                      float iouThreshold = 0.45f);

    // Calculate Intersection over Union for two boxes
    float calculateIoU(const QRect& box1, const QRect& box2);

    std::unique_ptr<Ort::Env> m_env;
    std::unique_ptr<Ort::Session> m_session;
    std::unique_ptr<Ort::SessionOptions> m_sessionOptions;
    std::vector<const char*> m_inputNames;
    std::vector<const char*> m_outputNames;
#endif

    int m_inputWidth;
    int m_inputHeight;
    bool m_useGPU;
    QStringList m_classLabels;
};
