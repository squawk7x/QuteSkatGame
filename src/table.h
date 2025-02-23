#ifndef TABLE_H
#define TABLE_H

#include <qboxlayout.h>

#include <QMainWindow>

#include "game.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class Table;
}
QT_END_NAMESPACE

class Table : public QMainWindow {
  Q_OBJECT

 private:
  Ui::Table *ui;
  Game game_{this};

 public:
  Table(QWidget *parent = nullptr);
  ~Table();

  // Slots
 public slots:
  void onClearTrickLayout();
};
#endif  // TABLE_H
