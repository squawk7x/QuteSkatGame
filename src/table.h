#ifndef TABLE_H
#define TABLE_H

#include <qboxlayout.h>
#include <qpushbutton.h>

#include <QMainWindow>

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
  void updatePlayerLayout(int playerId, MoveTo dest = MoveTo::Trick);
  void updateTrickLayout(const Card &card, int playerId);
  // void changeCardSize(CardSize cardSize);

  // Card *findCardButton(int playerId, const Card &card);

 public slots:
  void onStarted();
  void onGesagt(int idSager, int idHoerer, QString antwortSager,
                QString antwortHoerer);
  void onClearTrickLayout();
};
#endif  // TABLE_H
