#include "animation.h"
#include "GradientWidget.h"

#include <QApplication>
#include <QTimer>

namespace {
    GradientWidget* widget = nullptr;
    QApplication* app = nullptr;
    int argc = 1;
    char* argv[] = { const_cast<char*>("animation_app") };
}

void OnAnimation() {
    if (!app) {
        app = new QApplication(argc, argv);
    }

    if (!widget) {
        widget = new GradientWidget();
        widget->show();
    }

    QTimer::singleShot(5000, []() { OffAnimation(); });
    app->exec();
}

void OffAnimation() {
    if (widget) {
        widget->hide();
        delete widget;
        widget = nullptr;
    }

    if (app) {
        app->quit();
        delete app;
        app = nullptr;
    }
}