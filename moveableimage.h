#ifndef MOVABLEIMAGE_H
#define MOVABLEIMAGE_H

#include "QPropertyAnimation"
#include "qaudiooutput.h"
#include "qmediaplayer.h"
#include <QGraphicsPixmapItem>
#include <QTimer>

class MainWindow;

class MovableImage : public QObject, public QGraphicsPixmapItem {
    Q_OBJECT

public:
    MovableImage(const QPixmap &pixmap, int startX, int startY, const QString &type, MainWindow *mainWindow);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

private slots:
    void move();
    void playAnimation();
    void playEmptied();

private:

    QMediaPlayer *player = new QMediaPlayer;
    QAudioOutput *audioOutput = new QAudioOutput;
    QTimer *moveTimer;
    int speed;
    int currentFrame;
    QPropertyAnimation *animation;
    QString type;
    MainWindow *mainWindow;
};

#endif
