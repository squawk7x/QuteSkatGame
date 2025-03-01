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

 public:
  Table(QWidget *parent = nullptr);
  ~Table();

  // void addCardsToLayout(int layoutId);

  void addCardsToLayout(int layoutId);
  void addSkatCardsToLayout();
  void addPlayerCardsToLayout(int playerId, bool final = false);
  void updatePlayerLayout();
  void updateSkatLayout();
 public slots:
  void onClearTrickLayout();
};
#endif  // TABLE_H
