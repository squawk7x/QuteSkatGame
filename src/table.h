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

  void start();
  void updateSkatLayout(bool open = false);
  void updatePlayerLayout(int playerId, int dest = 0);
  void updateTrickLayout();

 public slots:
  void onStarted();
  void onGesagt(int idSager, int idHoerer, QString antwort);
  // void onGehoert(int idHoerer, QString antwort);
  void onClearTrickLayout();
};
#endif  // TABLE_H
