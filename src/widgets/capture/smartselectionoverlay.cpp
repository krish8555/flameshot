// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Contributors

#include "smartselectionoverlay.h"
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QDebug>

SmartSelectionOverlay::SmartSelectionOverlay(QWidget* parent)
  : QWidget(parent)
  , m_hoveredIndex(-1)
  , m_selectedIndex(-1)
  , m_showLabels(true)
  , m_normalColor(0, 150, 255, 120)
  , m_hoverColor(0, 255, 150, 180)
  , m_selectedColor(255, 200, 0, 200)
{
    setAttribute(Qt::WA_TransparentForMouseEvents, false);
    setAttribute(Qt::WA_NoSystemBackground, true);
    setAttribute(Qt::WA_TranslucentBackground, true);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
}

void SmartSelectionOverlay::setDetections(
  const QVector<DetectedObject>& detections)
{
    m_detections = detections;
    m_hoveredIndex = -1;
    m_selectedIndex = -1;
    update();
    
    qDebug() << "Smart selection: detected" << detections.size() << "objects";
}

void SmartSelectionOverlay::clearDetections()
{
    m_detections.clear();
    m_hoveredIndex = -1;
    m_selectedIndex = -1;
    update();
    emit selectionCleared();
}

QRect SmartSelectionOverlay::getSelectedRegion() const
{
    if (m_selectedIndex >= 0 && m_selectedIndex < m_detections.size()) {
        return m_detections[m_selectedIndex].boundingBox;
    }
    return QRect();
}

bool SmartSelectionOverlay::hasSelection() const
{
    return m_selectedIndex >= 0 && m_selectedIndex < m_detections.size();
}

void SmartSelectionOverlay::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    // Draw all detections
    for (int i = 0; i < m_detections.size(); ++i) {
        bool isHovered = (i == m_hoveredIndex);
        bool isSelected = (i == m_selectedIndex);
        drawDetection(painter, m_detections[i], isHovered, isSelected);
    }

    // Draw help text if no selection
    if (m_selectedIndex < 0 && !m_detections.isEmpty()) {
        painter.setPen(Qt::white);
        painter.setFont(QFont("Arial", 12, QFont::Bold));
        
        QString helpText = tr("Click on a highlighted region to select it (ESC to cancel)");
        QRect textRect = painter.fontMetrics().boundingRect(helpText);
        textRect.moveCenter(QPoint(width() / 2, 30));
        
        // Draw background for text
        painter.fillRect(textRect.adjusted(-10, -5, 10, 5),
                        QColor(0, 0, 0, 180));
        painter.drawText(textRect, Qt::AlignCenter, helpText);
    }
}

void SmartSelectionOverlay::mouseMoveEvent(QMouseEvent* event)
{
    int oldHovered = m_hoveredIndex;
    m_hoveredIndex = findDetectionAt(event->pos());

    if (oldHovered != m_hoveredIndex) {
        update();
        
        // Update cursor
        if (m_hoveredIndex >= 0) {
            setCursor(Qt::PointingHandCursor);
        } else {
            setCursor(Qt::ArrowCursor);
        }
    }

    QWidget::mouseMoveEvent(event);
}

void SmartSelectionOverlay::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        int clickedIndex = findDetectionAt(event->pos());
        
        if (clickedIndex >= 0) {
            m_selectedIndex = clickedIndex;
            QRect selectedRegion = m_detections[clickedIndex].boundingBox;
            
            qDebug() << "Selected object:"
                     << m_detections[clickedIndex].label
                     << "confidence:" << m_detections[clickedIndex].confidence
                     << "region:" << selectedRegion;
            
            emit regionSelected(selectedRegion);
            update();
        }
    }

    QWidget::mousePressEvent(event);
}

void SmartSelectionOverlay::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape) {
        clearDetections();
    } else if (event->key() == Qt::Key_L) {
        // Toggle labels
        m_showLabels = !m_showLabels;
        update();
    } else if (event->key() == Qt::Key_Tab) {
        // Cycle through detections
        if (!m_detections.isEmpty()) {
            m_selectedIndex = (m_selectedIndex + 1) % m_detections.size();
            QRect selectedRegion = m_detections[m_selectedIndex].boundingBox;
            emit regionSelected(selectedRegion);
            update();
        }
    }

    QWidget::keyPressEvent(event);
}

int SmartSelectionOverlay::findDetectionAt(const QPoint& pos) const
{
    // Check from last to first (top to bottom in z-order)
    for (int i = m_detections.size() - 1; i >= 0; --i) {
        if (m_detections[i].boundingBox.contains(pos)) {
            return i;
        }
    }
    return -1;
}

void SmartSelectionOverlay::drawDetection(QPainter& painter,
                                          const DetectedObject& detection,
                                          bool isHovered,
                                          bool isSelected) const
{
    QRect box = detection.boundingBox;

    // Choose color based on state
    QColor boxColor = m_normalColor;
    int borderWidth = 2;
    
    if (isSelected) {
        boxColor = m_selectedColor;
        borderWidth = 3;
    } else if (isHovered) {
        boxColor = m_hoverColor;
        borderWidth = 3;
    }

    // Draw filled rectangle
    painter.fillRect(box, boxColor);

    // Draw border
    QPen pen(boxColor.lighter(150), borderWidth);
    pen.setStyle(Qt::SolidLine);
    painter.setPen(pen);
    painter.drawRect(box);

    // Draw label if enabled
    if (m_showLabels && !detection.label.isEmpty()) {
        QString labelText =
          QString("%1 (%.1f%%)").arg(detection.label).arg(detection.confidence * 100);

        // Calculate label background size
        QFont font = painter.font();
        font.setPointSize(10);
        font.setBold(true);
        painter.setFont(font);
        
        QFontMetrics fm(font);
        QRect textRect = fm.boundingRect(labelText);
        textRect.adjust(-4, -2, 4, 2);
        textRect.moveTopLeft(box.topLeft() + QPoint(0, -textRect.height()));

        // Ensure label stays within widget bounds
        if (textRect.top() < 0) {
            textRect.moveTop(box.top());
        }
        if (textRect.right() > width()) {
            textRect.moveRight(width());
        }

        // Draw label background
        QColor labelBg = boxColor.darker(120);
        labelBg.setAlpha(220);
        painter.fillRect(textRect, labelBg);

        // Draw label text
        painter.setPen(Qt::white);
        painter.drawText(textRect, Qt::AlignCenter, labelText);
    }

    // Draw corner markers for better visibility
    if (isSelected || isHovered) {
        int cornerSize = 10;
        QPen cornerPen(Qt::white, 2);
        painter.setPen(cornerPen);

        // Top-left corner
        painter.drawLine(box.topLeft(), box.topLeft() + QPoint(cornerSize, 0));
        painter.drawLine(box.topLeft(), box.topLeft() + QPoint(0, cornerSize));

        // Top-right corner
        painter.drawLine(box.topRight(), box.topRight() + QPoint(-cornerSize, 0));
        painter.drawLine(box.topRight(), box.topRight() + QPoint(0, cornerSize));

        // Bottom-left corner
        painter.drawLine(box.bottomLeft(), box.bottomLeft() + QPoint(cornerSize, 0));
        painter.drawLine(box.bottomLeft(), box.bottomLeft() + QPoint(0, -cornerSize));

        // Bottom-right corner
        painter.drawLine(box.bottomRight(), box.bottomRight() + QPoint(-cornerSize, 0));
        painter.drawLine(box.bottomRight(), box.bottomRight() + QPoint(0, -cornerSize));
    }
}
