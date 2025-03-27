#include "moveableimage.h"
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPixmap>
#include <QTimer>
#include "QRandomGenerator"
#include "mainwindow.h"
#include <QtMultimedia>

#define BAD_SHIP "RedShip"


MovableImage::MovableImage(const QPixmap &pixmap, int startX, int startY, const QString &type, MainWindow *mainWindow)
    : QGraphicsPixmapItem(pixmap), speed(50), animation(nullptr), currentFrame(0), type(type), mainWindow(mainWindow) {
    setPos(startX, startY);

    moveTimer = new QTimer(this);
    connect(moveTimer, &QTimer::timeout, this, &MovableImage::move);
    moveTimer->start(50);
}

void MovableImage::move() {
    setPos(x() - speed, y());

    if (x() + pixmap().width() < 0) {

        if (type == "meteor" || type == BAD_SHIP) {
            player->setAudioOutput(audioOutput);
            player->setSource(QUrl("qrc:/new/prefix1/MeteorPass.mp3"));
            audioOutput->setVolume(0.5);
            player->play();

            if (mainWindow) {
                mainWindow->notifyHit();
            }
        }

        scene()->removeItem(this);
        delete this;
    }
}

void MovableImage::mousePressEvent(QGraphicsSceneMouseEvent *event) {

    if(!mainWindow->canPlayerShoot()){

        player->setAudioOutput(audioOutput);
        player->setSource(QUrl("qrc:/new/prefix1/Empty.mp3"));
        audioOutput->setVolume(0.5);
        player->play();

        return;
    }

        if (moveTimer) {
        moveTimer->stop();
    }

    playAnimation();

    if (type == "meteor") {
        type = "AHHH";

        player->setAudioOutput(audioOutput);
        player->setSource(QUrl("qrc:/new/prefix1/MeteorHit.mp3"));
        audioOutput->setVolume(0.5);
        player->play();

        mainWindow->writeMessage(QString("#shoot:meteor"));
    } else if (type != BAD_SHIP && (type == "BlueShip" || type == "RedShip")) {
        type = "AHHH";

        mainWindow->notifyHit();

        player->setAudioOutput(audioOutput);
        player->setSource(QUrl("qrc:/new/prefix1/HitBad.mp3"));
        audioOutput->setVolume(0.1);
        player->play();
        mainWindow->writeMessage(QString("#shoot:GoodShip"));
    } else if (type == BAD_SHIP) {
        type = "AHHH";
        player->setAudioOutput(audioOutput);
        player->setSource(QUrl("qrc:/new/prefix1/UfoClick.mp3"));
        audioOutput->setVolume(0.5);
        player->play();
        mainWindow->writeMessage(QString("#shoot:BadShip"));
    } else if (type == "Lootbox") {
        type = "AHHH";

        mainWindow->writeMessage(QString("#shoot:Lootbox"));
    }
}

void MovableImage::playAnimation() {
    QStringList frames = {":/new/prefix1/stage1.png", ":/new/prefix1/stage2.png", ":/new/prefix1/stage3.png"};

    if (currentFrame < frames.size()) {
        QPixmap framePixmap(frames[currentFrame]);
        if (!framePixmap.isNull()) {
            framePixmap = framePixmap.scaled(pixmap().size(), Qt::KeepAspectRatio, Qt::SmoothTransformation); // Ensure the frame is scaled properly
            setPixmap(framePixmap);

            // Center the animation image on the original position
            QPointF currentPosition = pos();
            QPointF centerOffset((pixmap().width() - framePixmap.width()) / 2, (pixmap().height() - framePixmap.height()) / 2);
            setPos(currentPosition + centerOffset);
        }
        currentFrame++;
        QTimer::singleShot(100, this, &MovableImage::playAnimation); // Play next frame after 200 ms
    } else {
        // Animation complete, remove the item
        scene()->removeItem(this);
        delete this;
    }
}

void MovableImage::playEmptied()
{
    player->setAudioOutput(audioOutput);
    player->setSource(QUrl("qrc:/new/prefix1/AmmoEmptied.mp3"));
    audioOutput->setVolume(0.5);
    player->play();
}





