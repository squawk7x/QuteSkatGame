#ifndef TABLE_H
#define TABLE_H

#include <qboxlayout.h>
#include <qpushbutton.h>

#include <QMainWindow>
#include <QMouseEvent>

#include "game.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class Skattisch;
}
QT_END_NAMESPACE

class Table : public QMainWindow {
  Q_OBJECT

 private:
  Ui::Skattisch *ui;
  Game *game_;
  CardSize cardSize_ = CardSize::Normal;

 public:
  Table(QWidget *parent = nullptr);
  ~Table();

  void start();
  void updateSkatLayout(bool hand = false);
  void updatePlayerLayout(int playerId, LinkTo dest = LinkTo::Trick);
  void updateTrickLayout(const Card &card, int playerId);

 protected:
  void mousePressEvent(QMouseEvent *event) override;

 public slots:
  void onGegeben();
  void onGeboten(int idSager, int idHoerer, QString antwortSager,
                 QString antwortHoerer);
  void onFrageHand();
  void onEnableDruecken();
  void onClearTrickLayout();
};
#endif  // TABLE_H
