// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Contributors

#pragma once

#include "src/ml/objectdetector.h"
#include <QLabel>
#include <QPainter>
#include <QTimer>
#include <QVector>
#include <QWidget>

// Widget to display detected objects as clickable overlays
class SmartSelectionOverlay : public QWidget
{
    Q_OBJECT

public:
    explicit SmartSelectionOverlay(QWidget* parent = nullptr);

    // Set the detected objects to display
    void setDetections(const QVector<DetectedObject>& detections);

    // Clear all detections
    void clearDetections();

    // Get currently selected object (if any)
    QRect getSelectedRegion() const;

    // Check if a region is selected
    bool hasSelection() const;

signals:
    void regionSelected(const QRect& region);
    void selectionCleared();

protected:
    void paintEvent(QPaintEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private:
    // Find detection at given position
    int findDetectionAt(const QPoint& pos) const;

    // Draw detection box with label
    void drawDetection(QPainter& painter,
                       const DetectedObject& detection,
                       bool isHovered,
                       bool isSelected) const;

    QVector<DetectedObject> m_detections;
    int m_hoveredIndex;
    int m_selectedIndex;
    bool m_showLabels;
    
    // Colors for visualization
    QColor m_normalColor;
    QColor m_hoverColor;
    QColor m_selectedColor;
};
