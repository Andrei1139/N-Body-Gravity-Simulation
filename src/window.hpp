#include <QWindow>
#include <iostream>
#include <QKeyEvent>
#include <qnamespace.h>

class Window: public QWindow {
    public:
        bool activeRendering = false;
    private:
        void keyPressEvent(QKeyEvent *event) override {
            if (event->key() == Qt::Key_Space)
                activeRendering = !activeRendering;
        }
};