// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Contributors

#include "onnxdetector.h"
#include <QDebug>
#include <QFile>
#include <QtMath>
#include <algorithm>

OnnxDetector::OnnxDetector(QObject* parent)
  : ObjectDetector(parent)
  , m_inputWidth(640)
  , m_inputHeight(640)
  , m_useGPU(false)
{
    // COCO dataset class labels (80 classes)
    m_classLabels = QStringList{
        "person",        "bicycle",      "car",
        "motorcycle",    "airplane",     "bus",
        "train",         "truck",        "boat",
        "traffic light", "fire hydrant", "stop sign",
        "parking meter", "bench",        "bird",
        "cat",           "dog",          "horse",
        "sheep",         "cow",          "elephant",
        "bear",          "zebra",        "giraffe",
        "backpack",      "umbrella",     "handbag",
        "tie",           "suitcase",     "frisbee",
        "skis",          "snowboard",    "sports ball",
        "kite",          "baseball bat", "baseball glove",
        "skateboard",    "surfboard",    "tennis racket",
        "bottle",        "wine glass",   "cup",
        "fork",          "knife",        "spoon",
        "bowl",          "banana",       "apple",
        "sandwich",      "orange",       "broccoli",
        "carrot",        "hot dog",      "pizza",
        "donut",         "cake",         "chair",
        "couch",         "potted plant", "bed",
        "dining table",  "toilet",       "tv",
        "laptop",        "mouse",        "remote",
        "keyboard",      "cell phone",   "microwave",
        "oven",          "toaster",      "sink",
        "refrigerator",  "book",         "clock",
        "vase",          "scissors",     "teddy bear",
        "hair drier",    "toothbrush"
    };
}

OnnxDetector::~OnnxDetector()
{
#ifdef ENABLE_ONNX_ML
    m_session.reset();
    m_sessionOptions.reset();
    m_env.reset();
#endif
}

void OnnxDetector::setInputSize(int width, int height)
{
    m_inputWidth = width;
    m_inputHeight = height;
}

void OnnxDetector::enableGPU(bool enable)
{
    m_useGPU = enable;
}

bool OnnxDetector::initialize(const QString& modelPath)
{
#ifdef ENABLE_ONNX_ML
    try {
        // Check if model file exists
        if (!QFile::exists(modelPath)) {
            qWarning() << "Model file not found:" << modelPath;
            emit initializationComplete(false);
            return false;
        }

        // Initialize ONNX Runtime environment
        m_env = std::make_unique<Ort::Env>(ORT_LOGGING_LEVEL_WARNING,
                                           "FlameshotOnnx");

        // Create session options
        m_sessionOptions = std::make_unique<Ort::SessionOptions>();
        m_sessionOptions->SetIntraOpNumThreads(4);
        m_sessionOptions->SetGraphOptimizationLevel(
          GraphOptimizationLevel::ORT_ENABLE_ALL);

        // Enable GPU if requested and available
        if (m_useGPU) {
            try {
                // Try to use CUDA if available
                OrtCUDAProviderOptions cuda_options;
                m_sessionOptions->AppendExecutionProvider_CUDA(cuda_options);
                qDebug() << "GPU acceleration enabled for ML model";
            } catch (const Ort::Exception& e) {
                qWarning() << "GPU not available, using CPU:" << e.what();
                m_useGPU = false;
            }
        }

        // Create session
        std::wstring modelPathW = modelPath.toStdWString();
        m_session = std::make_unique<Ort::Session>(*m_env,
                                                    modelPathW.c_str(),
                                                    *m_sessionOptions);

        // Get input/output names
        Ort::AllocatorWithDefaultOptions allocator;
        
        // Input
        size_t numInputNodes = m_session->GetInputCount();
        if (numInputNodes > 0) {
            auto inputName = m_session->GetInputNameAllocated(0, allocator);
            m_inputNames.push_back(inputName.get());
        }

        // Output
        size_t numOutputNodes = m_session->GetOutputCount();
        if (numOutputNodes > 0) {
            auto outputName = m_session->GetOutputNameAllocated(0, allocator);
            m_outputNames.push_back(outputName.get());
        }

        m_initialized = true;
        qDebug() << "ONNX model initialized successfully:" << modelPath;
        emit initializationComplete(true);
        return true;

    } catch (const Ort::Exception& e) {
        qWarning() << "Failed to initialize ONNX model:" << e.what();
        m_initialized = false;
        emit initializationComplete(false);
        return false;
    }
#else
    qWarning() << "ONNX support not compiled. Rebuild with ENABLE_ONNX_ML=ON";
    emit initializationComplete(false);
    return false;
#endif
}

bool OnnxDetector::isInitialized() const
{
    return m_initialized;
}

QString OnnxDetector::getBackendName() const
{
    return "ONNX Runtime";
}

QStringList OnnxDetector::getSupportedClasses() const
{
    return m_classLabels;
}

QVector<DetectedObject> OnnxDetector::detect(const QImage& image,
                                              float confidenceThreshold)
{
#ifdef ENABLE_ONNX_ML
    if (!m_initialized) {
        qWarning() << "Detector not initialized";
        return QVector<DetectedObject>();
    }

    try {
        // Preprocess image
        std::vector<float> inputTensorValues = preprocessImage(image);

        // Create input tensor
        std::vector<int64_t> inputShape = { 1, 3, m_inputHeight, m_inputWidth };
        Ort::MemoryInfo memoryInfo =
          Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);

        Ort::Value inputTensor = Ort::Value::CreateTensor<float>(
          memoryInfo,
          inputTensorValues.data(),
          inputTensorValues.size(),
          inputShape.data(),
          inputShape.size());

        // Run inference
        auto outputTensors = m_session->Run(Ort::RunOptions{ nullptr },
                                             m_inputNames.data(),
                                             &inputTensor,
                                             1,
                                             m_outputNames.data(),
                                             m_outputNames.size());

        // Get output tensor
        float* outputData = outputTensors[0].GetTensorMutableData<float>();
        auto outputShape = outputTensors[0].GetTensorTypeAndShapeInfo().GetShape();
        
        size_t outputSize = 1;
        for (auto dim : outputShape) {
            outputSize *= dim;
        }

        std::vector<float> output(outputData, outputData + outputSize);

        // Post-process output
        QVector<DetectedObject> detections =
          postprocessOutput(output, image.size(), confidenceThreshold);

        // Apply Non-Maximum Suppression
        detections = applyNMS(detections);

        emit detectionComplete(detections);
        return detections;

    } catch (const Ort::Exception& e) {
        QString error = QString("Detection failed: %1").arg(e.what());
        qWarning() << error;
        emit detectionError(error);
        return QVector<DetectedObject>();
    }
#else
    Q_UNUSED(image)
    Q_UNUSED(confidenceThreshold)
    qWarning() << "ONNX support not compiled";
    return QVector<DetectedObject>();
#endif
}

#ifdef ENABLE_ONNX_ML
std::vector<float> OnnxDetector::preprocessImage(const QImage& image)
{
    // Convert to RGB and resize
    QImage resized = image.scaled(m_inputWidth,
                                  m_inputHeight,
                                  Qt::IgnoreAspectRatio,
                                  Qt::SmoothTransformation)
                       .convertToFormat(QImage::Format_RGB888);

    // Normalize and convert to CHW format (Channel, Height, Width)
    std::vector<float> inputData(3 * m_inputHeight * m_inputWidth);

    const uchar* imageData = resized.constBits();
    int index = 0;

    // Normalize to [0, 1] and rearrange from HWC to CHW
    for (int c = 0; c < 3; ++c) {
        for (int h = 0; h < m_inputHeight; ++h) {
            for (int w = 0; w < m_inputWidth; ++w) {
                int pixelIndex = (h * m_inputWidth + w) * 3;
                inputData[index++] = imageData[pixelIndex + c] / 255.0f;
            }
        }
    }

    return inputData;
}

QVector<DetectedObject> OnnxDetector::postprocessOutput(
  const std::vector<float>& output,
  const QSize& originalSize,
  float confidenceThreshold)
{
    QVector<DetectedObject> detections;

    // YOLO format: [batch, num_detections, 85]
    // Each detection: [x, y, w, h, confidence, class_scores...]
    
    // This is a simplified implementation for YOLOv5/YOLOv8 output format
    // Adjust based on your specific model output format
    
    size_t numDetections = output.size() / 85;
    float scaleX = static_cast<float>(originalSize.width()) / m_inputWidth;
    float scaleY = static_cast<float>(originalSize.height()) / m_inputHeight;

    for (size_t i = 0; i < numDetections; ++i) {
        size_t baseIdx = i * 85;
        
        float objectness = output[baseIdx + 4];
        if (objectness < confidenceThreshold) {
            continue;
        }

        // Find class with highest score
        float maxClassScore = 0.0f;
        int classId = -1;
        for (int c = 0; c < 80; ++c) {
            float classScore = output[baseIdx + 5 + c];
            if (classScore > maxClassScore) {
                maxClassScore = classScore;
                classId = c;
            }
        }

        float confidence = objectness * maxClassScore;
        if (confidence < confidenceThreshold) {
            continue;
        }

        // Convert from center coordinates to corner coordinates
        float cx = output[baseIdx] * scaleX;
        float cy = output[baseIdx + 1] * scaleY;
        float w = output[baseIdx + 2] * scaleX;
        float h = output[baseIdx + 3] * scaleY;

        int x = static_cast<int>(cx - w / 2);
        int y = static_cast<int>(cy - h / 2);

        QRect box(x, y, static_cast<int>(w), static_cast<int>(h));
        QString label =
          classId >= 0 && classId < m_classLabels.size()
            ? m_classLabels[classId]
            : "unknown";

        detections.append(DetectedObject(box, confidence, label, classId));
    }

    return detections;
}

QVector<DetectedObject> OnnxDetector::applyNMS(
  const QVector<DetectedObject>& detections,
  float iouThreshold)
{
    if (detections.isEmpty()) {
        return detections;
    }

    // Sort by confidence (descending)
    QVector<DetectedObject> sorted = detections;
    std::sort(sorted.begin(),
              sorted.end(),
              [](const DetectedObject& a, const DetectedObject& b) {
                  return a.confidence > b.confidence;
              });

    QVector<DetectedObject> result;
    QVector<bool> suppressed(sorted.size(), false);

    for (int i = 0; i < sorted.size(); ++i) {
        if (suppressed[i]) {
            continue;
        }

        result.append(sorted[i]);

        // Suppress overlapping boxes
        for (int j = i + 1; j < sorted.size(); ++j) {
            if (suppressed[j]) {
                continue;
            }

            float iou = calculateIoU(sorted[i].boundingBox, sorted[j].boundingBox);
            if (iou > iouThreshold) {
                suppressed[j] = true;
            }
        }
    }

    return result;
}

float OnnxDetector::calculateIoU(const QRect& box1, const QRect& box2)
{
    QRect intersection = box1.intersected(box2);
    if (intersection.isEmpty()) {
        return 0.0f;
    }

    float intersectionArea = intersection.width() * intersection.height();
    float box1Area = box1.width() * box1.height();
    float box2Area = box2.width() * box2.height();
    float unionArea = box1Area + box2Area - intersectionArea;

    return unionArea > 0 ? intersectionArea / unionArea : 0.0f;
}
#endif
