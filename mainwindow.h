#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QLabel>
#include <QPropertyAnimation>
#include "boss.h"

class MovableImage;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(const QString &ipAddress, QWidget *parent = nullptr);
    ~MainWindow();

    // Public interface
    void writeMessage(QString message);
    void bossDefeated();
    void playerHitByProjectile();
    void spawnBoss();
    void removeBoss();

    // Public interface for MovableImage
    void notifyHit();
    bool canPlayerShoot() const;

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private slots:
    void spawnImage();
    void updateAmmoDisplay();
    void showResult(const QString &resultType);

private:
    void fireEvent();
    bool canShoot();
    void shakeContent();
    QTimer *stunTimer;
    bool isStunned;

    QTcpSocket *tcpSocket;
    QString ipAddress;
    QGraphicsScene *scene;
    QGraphicsView *view;
    QTimer *spawnTimer;
    QLabel *resultLabel = nullptr;
    QLabel *ammoLabel = nullptr;
    QPropertyAnimation *shakeAnimation = nullptr;
    int ammo = 10;
    bool _canShoot = true;
    QMediaPlayer *player;
    QAudioOutput *audioOutput;
    Boss *boss = nullptr;
};

#endif // MAINWINDOW_H
