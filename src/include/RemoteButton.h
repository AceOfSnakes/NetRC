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
#ifndef REMOTEBUTTON_H
#define REMOTEBUTTON_H

#include <QPushButton>

enum DPadSegment { None, Up, Down, Left, Right, Ok };

class RemoteButton : public QPushButton {
    Q_OBJECT
    Q_PROPERTY(QBrush clickGradient READ clickGradient WRITE setClickGradient)
    Q_PROPERTY(QBrush hoverGradient READ hoverGradient WRITE setHoverGradient)
public:
    void setClickGradient(const QBrush &brush) {
        m_clickGradient = brush;
        update(); // Redraw when the property changes
    }
    QBrush clickGradient() const { return m_clickGradient; }

    void setHoverGradient(const QBrush &brush) {
        m_hoverGradient = brush;
        update(); // Redraw when the property changes
    }
    QBrush hoverGradient() const { return m_hoverGradient; }
    QPushButton *ok, *up, *down, *left, *right;
    QPushButton* getButton(DPadSegment seg);
    explicit RemoteButton(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override; // To reset hover when mouse leaves
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    QBrush getBrush(QStyleOptionButton option, DPadSegment segID);
    QBrush m_clickGradient;
    QBrush m_hoverGradient;
    DPadSegment m_hoveredSegment = None;
    DPadSegment m_pressedSegment = None;
    DPadSegment getSegmentAt(QPointF pos);

    QPushButton * createButton(QPushButton * btn);

signals:
    void upClicked();
    void downClicked();
    void leftClicked();
    void rightClicked();
    void okClicked();
};

#endif // REMOTEBUTTON_H