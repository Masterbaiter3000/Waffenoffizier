#include "boss.h"
#include "mainwindow.h"
#include <QGraphicsScene>
#include <QGraphicsEllipseItem>
#include <QGraphicsTextItem>
#include <QGraphicsRectItem>
#include <QTimer>
#include <QBrush>
#include <QPen>
#include <QColor>
#include <QLinearGradient>
#include <QRadialGradient>

Boss::Boss(MainWindow *mainWindow, QGraphicsScene *scene)
    : QObject(), QGraphicsEllipseItem(), mainWindow(mainWindow), scene(scene), health(100) {

    // Create red boss circle with gradient
    QLinearGradient gradient(0, 0, 0, 200);
    gradient.setColorAt(0, Qt::red);
    gradient.setColorAt(1, Qt::darkRed);

    setRect(0, 0, 200, 200);
    setPos(scene->width() - 250, scene->height() / 2 - 100);
    setBrush(QBrush(gradient));
    setPen(QPen(Qt::black, 3));

    // Health bar
    healthBar = new QGraphicsRectItem(0, 0, 200, 20, this);
    healthBar->setPos(0, -30);
    healthBar->setBrush(QBrush(Qt::green));

    // Health text
    healthText = new QGraphicsTextItem("Health: 100", this);
    healthText->setPos(0, -50);
    healthText->setDefaultTextColor(Qt::white);

    // Shooting timer
    shootTimer = new QTimer(this);
    connect(shootTimer, &QTimer::timeout, this, &Boss::shootProjectile);
}

void Boss::startShooting() {
    if (shootTimer) {
        shootTimer->start(2000); // Shoot every 2 seconds
    }
}

void Boss::stopShooting() {
    if (shootTimer) {
        shootTimer->stop();
    }
}

void Boss::decreaseHealth() {
    health -= 10;
    updateHealthBar();

    if (health <= 0) {
        scene->removeItem(this);
        mainWindow->bossDefeated();
        delete this;
    }
}

void Boss::setHealth(int health) {
    this->health = health;
    updateHealthBar();
}

int Boss::getHealth() const {
    return health;
}

void Boss::updateHealthBar() {
    if (healthBar) {
        healthBar->setRect(0, 0, 2 * health, 20);
    }
    if (healthText) {
        healthText->setPlainText("Health: " + QString::number(health));
    }
}

void Boss::shootProjectile() {
    // Create stun projectile (yellow circle)
    QGraphicsEllipseItem *projectile = new QGraphicsEllipseItem(0, 0, 30, 30);

    // Make projectile glow
    QRadialGradient radialGrad(15, 15, 15);
    radialGrad.setColorAt(0, Qt::yellow);
    radialGrad.setColorAt(1, Qt::transparent);

    projectile->setBrush(QBrush(radialGrad));
    projectile->setPen(QPen(Qt::NoPen));
    projectile->setPos(pos().x() - 30, pos().y() + 100);
    projectile->setZValue(10);

    scene->addItem(projectile);

    // Projectile movement timer
    QTimer *moveTimer = new QTimer(this);
    connect(moveTimer, &QTimer::timeout, [projectile, this]() {
        projectile->setPos(projectile->x() - 15, projectile->y());

        if (projectile->x() < 0) {
            scene->removeItem(projectile);
            delete projectile;
            mainWindow->playerHitByProjectile();
        }
    });
    moveTimer->start(50);
}
