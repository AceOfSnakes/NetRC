/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "RemoteButton.h"
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QApplication>
#include <QStyleOptionButton>
#include <cmath>

RemoteButton::RemoteButton(QWidget *parent) : QPushButton(parent) {
    setFixedSize(130, 130);
    setMouseTracking(true); // CRITICAL: Allows mouseMoveEvent without clicking

    ok = new QPushButton(this);
    ok->setObjectName("rc_btn_ok");
    ok->setVisible(false);

    up = new QPushButton(this);
    up->setObjectName("rc_btn_up");
    up->setVisible(false);

    down = new QPushButton(this);
    down->setObjectName("rc_btn_down");
    down->setVisible(false);

    left = new QPushButton(this);
    left->setObjectName("rc_btn_left");
    left->setVisible(false);

    right = new QPushButton(this);
    right->setObjectName("rc_btn_right");
    right->setVisible(false);
}

QPushButton* RemoteButton::getButton(DPadSegment seg) {
    switch (seg) {
    case Down:
        return down;
    case Right:
        return right;
    case Left:
        return left;
    case Up:
        return up;
    case Ok:
        return ok;
    default:
        return nullptr;
    }
}
// Helper to determine which segment the mouse is over
DPadSegment RemoteButton::getSegmentAt(QPointF pos) {
    QPointF center(width() / 2.0, height() / 2.0);
    double dx = pos.x() - center.x();
    double dy = pos.y() - center.y();
    double dist = std::sqrt(dx*dx + dy*dy);

    double outerR = (qMin(width(), height()) / 2.0) - 10;
    double innerR = outerR * 0.45;

    if (dist > outerR) return None;
    if (dist < innerR) return Ok;

    // Calculate angle: Qt 0 deg is Right, 90 is Up
    double angle = std::atan2(-dy, dx) * 180.0 / M_PI;
    if (angle < 0) angle += 360;

    if (angle >= 45 && angle < 135) return Up;
    if (angle >= 135 && angle < 225) return Left;
    if (angle >= 225 && angle < 315) return Down;
    return Right;
}

void RemoteButton::mouseMoveEvent(QMouseEvent *event) {
    DPadSegment current = getSegmentAt(event->position());
    if (current != m_hoveredSegment) {
        m_hoveredSegment = current;
        update(); // Redraw only when the hovered segment changes
    }
    QPushButton::mouseMoveEvent(event);
}

void RemoteButton::leaveEvent(QEvent *) {
    m_hoveredSegment = None;
    update();
}

void RemoteButton::mousePressEvent(QMouseEvent *event) {
    // Determine which segment was actually pressed
    m_pressedSegment = getSegmentAt(event->position());
    
    if (m_pressedSegment != None) {
        update(); // Redraw to show "pressed" color
    }
    
    // We don't call QPushButton::mousePressEvent(event) because we are 
    // handling the logic for 5 different virtual buttons ourselves.
}

void RemoteButton::mouseReleaseEvent(QMouseEvent *event) {
    DPadSegment releasedSegment = getSegmentAt(event->position());

    // Only trigger the signal if the user released the mouse 
    // over the same segment they originally pressed.
    if (releasedSegment != None && releasedSegment == m_pressedSegment) {
        switch (releasedSegment) {
        case Up:
            emit up->click();
            break;
        case Down:
            emit down->click();
            break;
        case Left:
            emit left->click();
            break;
        case Right:
            emit right->click();
            break;
        case Ok:
            emit ok->click();
            break;
        default:
            break;
        }
    }

    m_pressedSegment = None; // Reset press state
    update(); // Return to normal/hover color
}

QBrush RemoteButton::getBrush(QStyleOptionButton option, DPadSegment segID) {

    QStyleOptionButton segOption = option;
    QBrush color = segOption.palette.brush(QPalette::Active, QPalette::Button);
    QPushButton * button = getButton(segID);
    if (button != nullptr) {
        if(!button->isEnabled()) {
            return segOption.palette.brush(QPalette::Disabled, QPalette::Button);
        }
    } else {
        return segOption.palette.brush(QPalette::Disabled, QPalette::Button);
    }

    if (m_pressedSegment == segID) {
        color = QBrush(m_clickGradient);
    } else if (m_hoveredSegment == segID) {
        color = QBrush(m_hoverGradient);
    } else if (!isEnabled()) {
        color = segOption.palette.brush(QPalette::Disabled, QPalette::Button);
    }

    return color;
}

void RemoteButton::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QStyleOptionButton option;
    option.initFrom(this);

    double outerR = (qMin(width(), height()) / 2.0) - 10;
    double innerR = outerR * 0.45;
    QPointF center(width() / 2.0, height() / 2.0);
    double gap = 6.0;
    double span = 90.0 - gap;

    auto drawDirection = [&](double startAngle, double arrowRotation, DPadSegment segID) {
        painter.save();
        
        // Define Segment Path
        QPainterPath path;
        QRectF outerRect(center.x()-outerR, center.y()-outerR, outerR*2, outerR*2);
        QRectF innerRect(center.x()-innerR, center.y()-innerR, innerR*2, innerR*2);
        path.arcMoveTo(outerRect, startAngle + gap/2);
        path.arcTo(outerRect, startAngle + gap/2, span);
        path.arcTo(innerRect, startAngle + gap/2 + span, -span);
        path.closeSubpath();

        if(getButton(segID)->isEnabled()) {
            option.palette.setCurrentColorGroup(QPalette::Active);
        } else {
            option.palette.setCurrentColorGroup(QPalette::Disabled);
        }

        QBrush color = getBrush(option, segID);

        painter.setBrush(color);
        painter.setPen(QPen(option.palette.text(), 1));
        painter.drawPath(path);

        // Draw Arrow
        painter.translate(center);
        painter.rotate(-arrowRotation);
        double arrowPos = innerR + (outerR - innerR) / 2.0;
        QPolygonF tri;
        tri << QPointF(0, -arrowPos - 8) << QPointF(-8, -arrowPos + 6) << QPointF(8, -arrowPos + 6);
        painter.setBrush(option.palette.text());
        painter.setPen(Qt::NoPen);
        painter.drawPolygon(tri);
        painter.restore();
    };

    // 3. Render all directions (Angles: 0 is Right, 90 is Up, etc.)
    drawDirection(45  + gap/2, 0, Up);   // UP: 0 deg points "North" after the translate
    drawDirection(135 + gap/2, 90, Left);  // LEFT: Rotate 90 deg clockwise
    drawDirection(225 + gap/2, 180, Down); // DOWN: Rotate 180 deg
    drawDirection(315 + gap/2, 270, Right); // RIGHT: Rotate 270 deg

    // Draw OK Button with highlight

    if(getButton(Ok)->isEnabled()) {
        option.palette.setCurrentColorGroup(QPalette::Active);
    } else {
        option.palette.setCurrentColorGroup(QPalette::Disabled);
    }

    QBrush okColor = getBrush(option, Ok);

    painter.setBrush(okColor);
    painter.setPen(QPen(option.palette.text(), 1));
    painter.drawEllipse(center, innerR - 3, innerR - 3);

    QFont bold("Verdana", 9);
    bold.setBold(true);

    painter.setFont(bold);

    painter.drawText(QRectF(center.x()-innerR, center.y()-innerR, innerR*2, innerR*2),
                     Qt::AlignCenter, "OK");
}
