#include "mainwindow.h"
#include "moveableimage.h"
#include <QGraphicsView>
#include <QRandomGenerator>
#include <QScreen>
#include <QApplication>
#include <QGraphicsPixmapItem>
#include <QCursor>
#include <QMouseEvent>
#include <QMediaPlayer>
#include <QUrl>
#include <QLabel>
#include <QTimer>
#include <QPropertyAnimation>
#include <QtMultimedia>
#include "boss.h"
#include <QGraphicsScene>
#include <QDebug>
#include <QPainter>
#include <QFont>



MainWindow::MainWindow(const QString &ipAddress, QWidget *parent)
    : QMainWindow(parent), ipAddress(ipAddress), boss(nullptr) {
    // Initialize TCP socket
    tcpSocket = new QTcpSocket(this);
    connect(tcpSocket, &QTcpSocket::connected, this, [this]() {
        qDebug() << "Connected to server!";
        QString message = "#role:WeaponsOfficer\r\n";
        tcpSocket->write(message.toUtf8());

    });

    stunTimer = new QTimer(this);
    isStunned = false;

    connect(tcpSocket, &QTcpSocket::readyRead, this, [this]() {
        QByteArray receivedData = tcpSocket->readAll();
        qDebug() << "Received data: " << receivedData;

        QString receivedMessage = QString::fromUtf8(receivedData);
        qDebug() << "Received message: " << receivedMessage;

        if (receivedMessage == "PositiveLootbox") {
            player->setAudioOutput(audioOutput);
            player->setSource(QUrl("qrc:/new/prefix1/GoodNotif.mp3"));
            audioOutput->setVolume(0.5);
            player->play();
            showResult(receivedMessage);
        } else if(receivedMessage == "NegativeLootbox") {
            player->setAudioOutput(audioOutput);
            player->setSource(QUrl("qrc:/new/prefix1/BadNotif.mp3"));
            audioOutput->setVolume(0.5);
            player->play();
            showResult(receivedMessage);
        } else if(receivedMessage.contains("#ammo:")) {
            int ammoIndex = receivedMessage.indexOf("#ammo:");
            QString ammoValue = receivedMessage.mid(ammoIndex + 6).trimmed();
            qDebug() << ammoValue;
            ammo = ammoValue.toInt();
            updateAmmoDisplay();
        }   if(receivedMessage == "SpawnBoss") {
            spawnBoss();
        } else if(receivedMessage == "RemoveBoss") {
            removeBoss();
        }
    });

    connect(tcpSocket, &QTcpSocket::errorOccurred, this, [](QAbstractSocket::SocketError socketError) {
        qDebug() << "Error:" << socketError;
    });

    tcpSocket->connectToHost(ipAddress, 9999);

    // Window setup
    setWindowFlags(Qt::FramelessWindowHint);
    view = new QGraphicsView(this);

    // Scene setup
    QRect screenGeometry = QApplication::screens().at(0)->geometry();
    int screenWidth = screenGeometry.width();
    int screenHeight = screenGeometry.height();

    scene = new QGraphicsScene(0, 0, screenWidth, screenHeight, this);
    view->setScene(scene);
    setCentralWidget(view);

    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    showMaximized();

    // Background setup
    QPixmap backgroundPixmap(":/new/prefix1/Background.png");
    if (!backgroundPixmap.isNull()) {
        QGraphicsPixmapItem *background = new QGraphicsPixmapItem(backgroundPixmap.scaled(screenWidth, screenHeight, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
        scene->addItem(background);
        background->setZValue(-2);
    }

    // Layer setup
    QPixmap layerPixmap(":/new/prefix1/layer.png");
    if (!layerPixmap.isNull()) {
        QGraphicsPixmapItem *layer = new QGraphicsPixmapItem(layerPixmap.scaled(screenWidth, screenHeight, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
        scene->addItem(layer);
        layer->setZValue(1);
    }

    // Cursor setup
    QPixmap cursorPixmap(":/new/prefix1/cursor.png");
    if (!cursorPixmap.isNull()) {
        int cursorSize = screenWidth / 15;
        cursorPixmap = cursorPixmap.scaled(cursorSize, cursorSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        QCursor customCursor(cursorPixmap);
        setCursor(customCursor);
    }

    // Spawn timer setup
    spawnTimer = new QTimer(this);
    connect(spawnTimer, &QTimer::timeout, this, &MainWindow::spawnImage);
    spawnTimer->start(1000);

    // Ammo display setup
    ammoLabel = new QLabel(this);
    ammoLabel->setStyleSheet("QLabel { color : white; font-size: 60px; font-weight: 1000}");
    updateAmmoDisplay();
    ammoLabel->setGeometry(screenWidth - 450, screenHeight - 250, 400, 230);
    ammoLabel->show();

    // Media player setup
    player = new QMediaPlayer(this);
    audioOutput = new QAudioOutput(this);
}

MainWindow::~MainWindow() {
    delete scene;
    delete spawnTimer;
    delete resultLabel;
    delete shakeAnimation;
    delete ammoLabel;
    if (boss) {
        delete boss;
    }
}

void MainWindow::spawnBoss() {
    if (!boss) {
        boss = new Boss(this, scene);
        scene->addItem(boss);
        boss->startShooting();
    }
}

void MainWindow::removeBoss() {
    if (boss) {
        boss->stopShooting();
        scene->removeItem(boss);
        delete boss;
        boss = nullptr;
    }
}

void MainWindow::bossDefeated() {
    qDebug() << "Boss defeated!";
    removeBoss();
}

void MainWindow::playerHitByProjectile() {
    if (isStunned) return;

    isStunned = true;
    ammoLabel->setStyleSheet("QLabel { color : Red; font-size: 60px; font-weight: 1000}");

    // Play stun sound effect
    player->setAudioOutput(audioOutput);
    player->setSource(QUrl("qrc:/new/prefix1/StunEffect.mp3")); // Add this sound file
    audioOutput->setVolume(0.7);
    player->play();

    // Show stun effect on screen
    QLabel *stunLabel = new QLabel("WEAPONS STUNNED!", this);
    stunLabel->setStyleSheet("QLabel { color : red; font-size: 80px; font-weight: bold; }");
    stunLabel->setAlignment(Qt::AlignCenter);
    stunLabel->setGeometry(0, scene->height()/2 - 50, scene->width(), 100);
    stunLabel->show();

    // Remove stun after 3 seconds
    QTimer::singleShot(3000, [this, stunLabel]() {
        isStunned = false;
        ammoLabel->setStyleSheet("QLabel { color : White; font-size: 60px; font-weight: 1000}");
        stunLabel->hide();
        delete stunLabel;
    });
}

void MainWindow::updateAmmoDisplay() {
    ammoLabel->setText("Munition " + QString::number(ammo));

    if (ammo == 0) {
        ammoLabel->setStyleSheet("QLabel { color : Red; font-size: 60px; font-weight: 1000}");
    } else {
        ammoLabel->setStyleSheet("QLabel { color : White; font-size: 60px; font-weight: 1000}");
    }
}

bool MainWindow::canShoot() {
    return ammo > 0;
}

void MainWindow::fireEvent() {
    if (ammo == 0) {
        player->setAudioOutput(audioOutput);
        player->setSource(QUrl("qrc:/new/prefix1/Empty.mp3"));
        audioOutput->setVolume(0.5);
        player->play();
    } else {
        writeMessage(QString("#shoot:none"));
    }

    if (canShoot()) {
        updateAmmoDisplay();
        if (ammo == 0) {
            player->setAudioOutput(audioOutput);
            player->setSource(QUrl("qrc:/new/prefix1/AmmoEmptied.mp3"));
            audioOutput->setVolume(0.5);
            player->play();
        }
    }
}

void MainWindow::mousePressEvent(QMouseEvent *event) {
    fireEvent();
}

void MainWindow::writeMessage(QString message) {
    tcpSocket->write(message.toUtf8());
}

void MainWindow::shakeContent() {
    if (!shakeAnimation) {
        shakeAnimation = new QPropertyAnimation(view, "pos");
        shakeAnimation->setDuration(100);
        shakeAnimation->setLoopCount(2);
    }

    QPoint originalPos = view->pos();
    shakeAnimation->setKeyValueAt(0, originalPos);
    shakeAnimation->setKeyValueAt(0.1, originalPos + QPoint(-10, 0));
    shakeAnimation->setKeyValueAt(0.2, originalPos + QPoint(10, 0));
    shakeAnimation->setKeyValueAt(0.3, originalPos + QPoint(-10, 0));
    shakeAnimation->setKeyValueAt(0.4, originalPos + QPoint(10, 0));
    shakeAnimation->setKeyValueAt(0.5, originalPos + QPoint(-10, 0));
    shakeAnimation->setKeyValueAt(0.6, originalPos + QPoint(10, 0));
    shakeAnimation->setKeyValueAt(0.7, originalPos + QPoint(-10, 0));
    shakeAnimation->setKeyValueAt(0.8, originalPos + QPoint(10, 0));
    shakeAnimation->setKeyValueAt(0.9, originalPos + QPoint(-10, 0));
    shakeAnimation->setKeyValueAt(1, originalPos);

    shakeAnimation->start();
}

void MainWindow::spawnImage() {
    QRect screenGeometry = QApplication::screens().at(0)->geometry();
    int screenWidth = screenGeometry.width();
    int screenHeight = screenGeometry.height();

    int width = QRandomGenerator::global()->bounded(screenWidth / 10, screenWidth / 5);
    int height = QRandomGenerator::global()->bounded(screenHeight / 10, screenHeight / 5);

    int randomNumber = QRandomGenerator::global()->bounded(1, 101);
    QPixmap pixmap;
    QString type;

    if(randomNumber < 70) {
        pixmap = QPixmap(":/new/prefix1/meteor.png");
        type = "meteor";
    } else {
        randomNumber = QRandomGenerator::global()->bounded(1, 101);

        if (randomNumber < 65) {
            randomNumber = QRandomGenerator::global()->bounded(1, 101);

            if (randomNumber < 50) {
                pixmap = QPixmap(":/new/prefix1/Raumschiff1.png");
                type = "BlueShip";
            } else {
                pixmap = QPixmap(":/new/prefix1/Raumschiff2.png");
                type = "RedShip";
            }
        } else {
            pixmap = QPixmap(":/new/prefix1/Lootbox.png");
            type = "Lootbox";
        }
    }

    if (pixmap.isNull()) {
        return;
    }

    pixmap = pixmap.scaled(width, height, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    int y = QRandomGenerator::global()->bounded(50, screenHeight - height - 50);
    int x = screenWidth + 50;

    MovableImage *img = new MovableImage(pixmap, x, y, type, this);

    if (type != "BlueShip" && type != "RedShip") {
        qreal rotationAngle = QRandomGenerator::global()->bounded(-45, 45);
        img->setRotation(rotationAngle);
    }

    img->setZValue(0);
    scene->addItem(img);
}

void MainWindow::mouseMoveEvent(QMouseEvent *event) {
    QMainWindow::mouseMoveEvent(event);
}

void MainWindow::showResult(const QString &resultType) {
    if (resultLabel) {
        resultLabel->hide();
        delete resultLabel;
        resultLabel = nullptr;
    }

    QPixmap resultPixmap(resultType == "Positive" ? ":/new/prefix1/Positive.png" : ":/new/prefix1/Negative.png");
    if (resultPixmap.isNull()) {
        return;
    }

    resultLabel = new QLabel(this);
    resultPixmap = resultPixmap.scaled(200, 100, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    QImage imageWithText(resultPixmap.size(), QImage::Format_ARGB32);
    imageWithText.fill(Qt::transparent);

    QPainter painter(&imageWithText);
    painter.drawPixmap(0, 0, resultPixmap);
    painter.setPen(Qt::white);
    painter.setFont(QFont("Arial", 16, QFont::Bold));
    painter.drawText(imageWithText.rect(), Qt::AlignCenter, resultType);
    painter.end();

    resultLabel->setPixmap(QPixmap::fromImage(imageWithText));
    resultLabel->setAlignment(Qt::AlignCenter);

    QRect screenGeometry = QApplication::screens().at(0)->geometry();
    resultLabel->setGeometry(20, screenGeometry.height() - 120, 200, 100);
    resultLabel->setCursor(Qt::ArrowCursor);
    resultLabel->setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    resultLabel->setAttribute(Qt::WA_ShowWithoutActivating, false);
    resultLabel->setStyleSheet("background: rgba(0, 0, 0, 0.7); border: 2px solid white;");
    resultLabel->show();

    QTimer::singleShot(2000, [this]() {
        if (resultLabel) {
            resultLabel->hide();
            delete resultLabel;
            resultLabel = nullptr;
        }
    });

    if (resultType == "Negative") {
        shakeContent();
    }
}
void MainWindow::notifyHit() {
    fireEvent();
    shakeContent();
}

bool MainWindow::canPlayerShoot() const {
    return canPlayerShoot();
}
