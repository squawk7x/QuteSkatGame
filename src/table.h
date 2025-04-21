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

 protected:
  void mousePressEvent(QMouseEvent *event) override;
  void setButtonLogic();

 public slots:
  void onGegeben();
  void onGeboten(int idSager, int idHoerer, QString antwortSager,
                 QString antwortHoerer);
  void onFrageHand();
  void onRuleAndTrump(Rule rule, std::string trump, bool hand,
                      bool schneiderAngesagt, bool schwarzAngesagt,
                      bool ouvert);
  // void onRoboGedrueckt(); // direct
  void onResultat();

  void onUpdateSkatLayout(LinkTo dest = LinkTo::Skat);
  void onUpdatePlayerLayout(int playerId, LinkTo dest = LinkTo::Trick);
  void onUpdateTrickLayout(const Card &card, int playerId);

  void onClearTrickLayout();
};
#endif  // TABLE_H
