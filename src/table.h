#ifndef TABLE_H
#define TABLE_H

#include <QMainWindow>

#include "game.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class Table;
}
QT_END_NAMESPACE

class Table : public QMainWindow {
  Q_OBJECT

  Game game_{this};

 public:
  Table(QWidget *parent = nullptr);
  ~Table();

 private:
  Ui::Table *ui;
};
#endif  // TABLE_H
