// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Contributors

#pragma once

#include <QImage>
#include <QObject>
#include <QRect>
#include <QString>
#include <QVector>

// Represents a detected object/region in the image
struct DetectedObject
{
    QRect boundingBox; // Location and size of detected object
    float confidence;  // Detection confidence (0.0 to 1.0)
    QString label;     // Object class/type label
    int classId;       // Numeric class identifier

    DetectedObject(const QRect& box = QRect(),
                   float conf = 0.0f,
                   const QString& lbl = "",
                   int id = -1)
      : boundingBox(box)
      , confidence(conf)
      , label(lbl)
      , classId(id)
    {}
};

// Abstract interface for object detection backends
class ObjectDetector : public QObject
{
    Q_OBJECT

public:
    explicit ObjectDetector(QObject* parent = nullptr);
    virtual ~ObjectDetector() = default;

    // Initialize detector with model path
    virtual bool initialize(const QString& modelPath) = 0;

    // Check if detector is ready to use
    virtual bool isInitialized() const = 0;

    // Detect objects in the given image
    virtual QVector<DetectedObject> detect(const QImage& image,
                                           float confidenceThreshold = 0.5f) = 0;

    // Get list of supported class labels
    virtual QStringList getSupportedClasses() const = 0;

    // Get detector backend name (e.g., "ONNX", "TensorFlow Lite")
    virtual QString getBackendName() const = 0;

signals:
    void detectionComplete(const QVector<DetectedObject>& objects);
    void detectionError(const QString& error);
    void initializationComplete(bool success);

protected:
    bool m_initialized;
};
