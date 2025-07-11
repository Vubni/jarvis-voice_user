#ifndef ANIMATIONCONTROLLER_H
#define ANIMATIONCONTROLLER_H

#include <QObject>

class AnimationController : public QObject {
    Q_OBJECT
public:
    explicit AnimationController(QObject* parent = nullptr) : QObject(parent) {}

signals:
    void toggleAnimation(bool isActive);
    void startAnimation();
    void stopAnimation();
};

#endif // ANIMATIONCONTROLLER_H