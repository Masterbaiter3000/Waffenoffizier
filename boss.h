#ifndef BOSS_H
#define BOSS_H

#include <QObject>
#include <QGraphicsEllipseItem>
#include <QTimer>
#include <QGraphicsScene>

class MainWindow;

class Boss : public QObject, public QGraphicsEllipseItem
{
    Q_OBJECT

public:
    explicit Boss(MainWindow *mainWindow, QGraphicsScene *scene);
    void decreaseHealth();
    void startShooting();
    void stopShooting();
    void setHealth(int health);
    int getHealth() const;

private slots:
    void shootProjectile();
    void updateHealthBar();

private:
    MainWindow *mainWindow;
    QGraphicsScene *scene;
    int health;
    QTimer *shootTimer;
    QGraphicsRectItem *healthBar;
    QGraphicsTextItem *healthText;
};

#endif // BOSS_H
