// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Contributors

#include "objectdetector.h"

ObjectDetector::ObjectDetector(QObject* parent)
  : QObject(parent)
  , m_initialized(false)
{}
