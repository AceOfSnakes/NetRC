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