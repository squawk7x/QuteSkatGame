#include "table.h"

#include <QTimer>
// #include <thread>

#include "src/ui_table.h"

Table::Table(
    QWidget *parent)
    : QMainWindow(parent), ui(new Ui::Skattisch), game_(new Game(this)) {
  ui->setupUi(this);

  // Spiel Farbe, Grand, Null, Ramsch
  {
    // Karten geben
    QObject::connect(game_, &Game::gegeben, this, &Table::onGegeben);

    // Reizen
    QObject::connect(game_, &Game::geboten, this, &Table::onGeboten);

    // Frage Hand
    QObject::connect(game_, &Game::frageHand, this, &Table::onFrageHand);

    QObject::connect(
        ui->pbKaro, &QPushButton::clicked, this, [this](bool checked) {
          game_->rule_ = checked ? Rule::Suit : Rule::Unset;
          game_->trump_ = checked ? "♦" : "";
          for (int playerId = 1; playerId <= 3; playerId++) {
            onUpdatePlayerLayout(playerId, LinkTo::Trick);
          }
          game_->ouvert_ = false;
          ui->pbOuvert->setChecked(false);

          setButtonLogic();

          qDebug() << "trump_ set to" << QString::fromStdString(game_->trump_);
        });
    QObject::connect(
        ui->pbHerz, &QPushButton::clicked, this, [this](bool checked) {
          game_->rule_ = checked ? Rule::Suit : Rule::Unset;
          game_->trump_ = checked ? "♥" : "";
          for (int playerId = 1; playerId <= 3; playerId++) {
            onUpdatePlayerLayout(playerId, LinkTo::Trick);
          }
          game_->ouvert_ = false;
          ui->pbOuvert->setChecked(false);

          setButtonLogic();

          qDebug() << "trump_ set to" << QString::fromStdString(game_->trump_);
        });
    QObject::connect(
        ui->pbPik, &QPushButton::clicked, this, [this](bool checked) {
          game_->rule_ = checked ? Rule::Suit : Rule::Unset;
          game_->trump_ = checked ? "♠" : "";
          for (int playerId = 1; playerId <= 3; playerId++) {
            onUpdatePlayerLayout(playerId, LinkTo::Trick);
          }
          game_->ouvert_ = false;
          ui->pbOuvert->setChecked(false);

          setButtonLogic();

          qDebug() << "trump_ set to" << QString::fromStdString(game_->trump_);
        });
    QObject::connect(
        ui->pbKreuz, &QPushButton::clicked, this, [this](bool checked) {
          game_->rule_ = checked ? Rule::Suit : Rule::Unset;
          game_->trump_ = checked ? "♣" : "";
          for (int playerId = 1; playerId <= 3; playerId++) {
            onUpdatePlayerLayout(playerId, LinkTo::Trick);
          }
          game_->ouvert_ = false;
          ui->pbOuvert->setChecked(false);

          setButtonLogic();

          qDebug() << "trump_ set to" << QString::fromStdString(game_->trump_);
        });
    QObject::connect(
        ui->pbGrand, &QPushButton::clicked, this, [this](bool checked) {
          game_->rule_ = checked ? Rule::Grand : Rule::Unset;
          game_->trump_ = checked ? "J" : "";
          for (int playerId = 1; playerId <= 3; playerId++) {
            onUpdatePlayerLayout(playerId, LinkTo::Trick);
          }
          game_->ouvert_ = false;
          ui->pbOuvert->setChecked(false);

          setButtonLogic();

          qDebug() << "trump_ set to" << QString::fromStdString(game_->trump_);


        });

    QObject::connect(
        ui->pbNull, &QPushButton::clicked, this, [this](bool checked) {
          game_->rule_ = checked ? Rule::Null : Rule::Unset;
          game_->trump_ = "";
          for (int playerId = 1; playerId <= 3; playerId++) {
            onUpdatePlayerLayout(playerId, LinkTo::Trick);
          }
          game_->ouvert_ = false;
          ui->pbOuvert->setChecked(false);

          setButtonLogic();

          qDebug() << "trump_ set to" << QString::fromStdString(game_->trump_);
        });

    QObject::connect(game_, &Game::ruleAndTrump, this, &Table::onRuleAndTrump);

    // Spiel Ouvert <=> Hand, Schneider, Schwarz
    // außer Null Ouvert
    QObject::connect(ui->pbOuvert, &QPushButton::toggled, this,
                     [this](bool checked) {
                       game_->ouvert_ = checked;

                       setButtonLogic();

                       qDebug() << "ouvert_ set to" << game_->ouvert_;
                     });

    QObject::connect(
        ui->pbSchneider, &QPushButton::toggled, this, [this](bool checked) {
          game_->schneiderAngesagt_ = checked;

          qDebug() << "schneider_ set to" << game_->schneiderAngesagt_;
        });

    QObject::connect(
        ui->pbSchwarz, &QPushButton::toggled, this, [this](bool checked) {
          game_->schwarzAngesagt_ = checked;
          game_->schneiderAngesagt_ = checked;
          ui->pbSchneider->setChecked(checked);

          qDebug() << "schneider_/schwarz_ set to" << game_->schneiderAngesagt_
                   << "/" << game_->schwarzAngesagt_;
        });

    QObject::connect(ui->pbStart, &QPushButton::clicked, game_, &Game::start);

    QObject::connect(ui->pbBieten, &QPushButton::clicked, this, [this]() {
      if (!game_->sager->isRobot()) game_->gereizt_ = game_->reizen();

      qDebug() << "Player pbBieten" << game_->sager->name();

      game_->bieten(Passen::Nein);
    });

    QObject::connect(ui->pbPassen, &QPushButton::clicked, this,
                     [this]() { game_->bieten(Passen::Ja); });

    // Testing
    QObject::connect(ui->pbBieten2, &QPushButton::clicked, this,
                     [this]() { game_->bieten(Passen::Nein); });

    // Testing
    QObject::connect(ui->pbBieten3, &QPushButton::clicked, this,
                     [this]() { game_->bieten(Passen::Nein); });

    QObject::connect(ui->pbHandNein, &QPushButton::clicked, this, [this]() {
      game_->hand_ = false;
      ui->pbHand->setText("");
      ui->gbFrageHand->hide();
      ui->gbDruecken->show();
      onUpdateSkatLayout(LinkTo::SoloPlayer);
    });

    QObject::connect(ui->pbHandJa, &QPushButton::clicked, this, [this]() {
      game_->hand_ = true;
      ui->pbHand->setText("Hand");
      ui->gbFrageHand->hide();
      ui->pbDruecken->click();  // even if not visible
    });

    QObject::connect(ui->pbDruecken, &QPushButton::clicked, this, [this]() {
      if (game_->skat_.size() == 2) {
        onUpdateSkatLayout(LinkTo::Skat);

        // onUpdatePlayerLayout(1, LinkTo::Trick);
        // For testing all 3 players are connected
        for (int playerId = 1; playerId <= 3; playerId++)
          onUpdatePlayerLayout(playerId, LinkTo::Trick);

        game_->druecken();

        ui->gbFrageHand->hide();
        ui->gbDruecken->hide();
        ui->gbSkat->hide();

        // ui->gbTricks->show();
        // ui->gbTrick2->hide();
        // ui->gbTrick1->hide();
        // ui->gbTrick3->hide();

        ui->lblSpielGereiztBis->setText("Gereizt bis: " +
                                        QString::number(game_->gereizt_));
        ui->gbSpiel->show();
      }
    });

    QObject::connect(game_, &Game::roboGedrueckt, this, [this]() {
      ui->pbDruecken->click();
      // ui->pbSpielen->click();
    });

    QObject::connect(ui->pbSpielen, &QPushButton::clicked, this, [this]() {
      // game_->setSpielwert();

      // ui->gbSpiel->hide(); // shown for testing
      ui->pbBieten2->hide();
      ui->pbBieten3->hide();
      ui->lblHand2->setText("");
      ui->lblHand3->setText("");

      ui->gbTricks->show();
      // ui->gbTrick2->show();
      // ui->gbTrick1->show();
      // ui->gbTrick3->show();
    });

    QObject::connect(ui->pbResultatWeiter, &QPushButton::clicked, game_,
                     &Game::start);

    QObject::connect(ui->pbResultatEnde, &QPushButton::clicked, this, [this]() {
      ui->gbSkat->hide();
      ui->gbResultat->hide();
      ui->gbFrageEnde->show();
    });

    QObject::connect(ui->pbEndeJa, &QPushButton::clicked, this,
                     &QWidget::close);

    QObject::connect(ui->pbEndeNein, &QPushButton::clicked, this, [this]() {
      ui->gbSkat->show();
      ui->gbResultat->show();
      ui->gbFrageEnde->hide();
    });

    QObject::connect(game_, &Game::resultat, this, &Table::onResultat);

    // Layouts
    QObject::connect(game_, &Game::updateSkatLayout, this,
                     &Table::onUpdateSkatLayout);

    QObject::connect(game_, &Game::updateTrickLayout, this,
                     &Table::onUpdateTrickLayout);

    QObject::connect(game_, &Game::updatePlayerLayout, this,
                     &Table::onUpdatePlayerLayout);

    QObject::connect(game_, &Game::clearTrickLayout, this,
                     &Table::onClearTrickLayout);

    // Addons
    QObject::connect(ui->rbNormal, &QRadioButton::toggled, this, [&]() {
      if (ui->rbNormal->isChecked()) {
        cardSize_ = CardSize::Normal;
      } else {
        cardSize_ = CardSize::Small;
      }
    });
  }

  game_->init();
}

void Table::onGegeben() {
  ui->lblUrsprungsSkat->hide();

  ui->gbTricks->hide();
  ui->gbSkat->show();

  ui->gbSagenPassen->show();
  ui->pbBieten2->show();
  ui->pbBieten3->show();

  ui->gbFrageHand->hide();
  ui->gbDruecken->hide();
  ui->gbAngesagt->hide();
  ui->gbFrageEnde->hide();

  ui->gbSpiel->hide();

  ui->gbRule->setEnabled(true);

  QList<QPushButton *> suitButtons = {ui->pbKaro,  ui->pbHerz,  ui->pbPik,
                                      ui->pbKreuz, ui->pbGrand, ui->pbNull};

  for (auto *btn : suitButtons) {
    btn->setAutoExclusive(false);  // Deaktiviert Gruppenzwang
    btn->setChecked(false);        // Setzt auf nicht gedrückt
  }

  for (auto *btn : suitButtons) {
    btn->setAutoExclusive(true);  // Gruppenzwang wieder aktivieren
  }

  // ui-pbHand is already set
  ui->pbOuvert->setVisible(false);
  ui->pbSchneider->setVisible(false);
  ui->pbSchwarz->setVisible(false);

  ui->gbTricks->hide();
  // ui->gbTrick2->hide();
  // ui->gbTrick1->hide();
  // ui->gbTrick3->hide();

  // ui->gbFrageEnde->hide();
  ui->gbResultat->hide();

  if (game_->geberHoererSagerPos_[0] == 2) ui->lblHand2->setText("Hinterhand");
  if (game_->geberHoererSagerPos_[1] == 2) ui->lblHand2->setText("Vorhand");
  if (game_->geberHoererSagerPos_[2] == 2) ui->lblHand2->setText("Mittelhand");
  if (game_->geberHoererSagerPos_[0] == 3) ui->lblHand3->setText("Hinterhand");
  if (game_->geberHoererSagerPos_[1] == 3) ui->lblHand3->setText("Vorhand");
  if (game_->geberHoererSagerPos_[2] == 3) ui->lblHand3->setText("Mittelhand");

  // reset to empty
  onClearTrickLayout();

  // first link cards to players handdeck
  for (int playerId = 1; playerId <= 3; playerId++)
    onUpdatePlayerLayout(playerId, LinkTo::Handdeck);

  // first link cards to skat
  onUpdateSkatLayout(LinkTo::Skat);
}

void Table::onGeboten(
    int idSager, int idHoerer, QString antwortSager, QString antwortHoerer) {
  ui->gbSagenPassen->hide();
  ui->pbBieten2->hide();
  ui->pbBieten3->hide();
  ui->pbRamsch->setVisible((game_->gereizt_ > 0) ? false : true);

  switch (idSager) {
    case 1:
      ui->pbBieten->setText(antwortSager);
      ui->gbSagenPassen->show();
      break;
    case 2:
      ui->pbBieten2->setText(antwortSager);
      ui->pbBieten2->show();
      break;
    case 3:
      ui->pbBieten3->setText(antwortSager);
      ui->pbBieten3->show();
      break;
  }

  switch (idHoerer) {
    case 1:
      ui->pbBieten->setText(antwortHoerer);
      ui->gbSagenPassen->show();
      break;
    case 2:
      ui->pbBieten2->setText(antwortHoerer);
      ui->pbBieten2->show();
      break;
    case 3:
      ui->pbBieten3->setText(antwortHoerer);
      ui->pbBieten3->show();
      break;
  }
}

void Table::onFrageHand() {
  ui->gbSagenPassen->hide();
  ui->pbBieten2->hide();
  ui->pbBieten3->hide();

  ui->gbFrageHand->show();
  ui->lblSkatGereiztBis->setText("Gereizt bis: " +
                                 QString::number(game_->gereizt_));
  ui->lblSkatGereiztBis->show();
}

// Robots will emit
void Table::onRuleAndTrump(
    Rule rule, std::string trump) {
  ui->pbRamsch->setVisible(false);

  if (rule == Rule::Ramsch) {
    ui->pbRamsch->setVisible(true);
    ui->pbHand->setVisible(game_->hand_);  // should be false
    // ui->lblSpielwertGereizt->setText("Ramsch!");
  } else if (rule == Rule::Grand) {
    ui->pbGrand->click();
  } else if (rule == Rule::Null) {
    ui->pbNull->click();
  } else if (trump == "♦") {
    ui->pbKaro->click();
  } else if (trump == "♥") {
    ui->pbHerz->click();
  } else if (trump == "♠") {
    ui->pbPik->click();
  } else if (trump == "♣") {
    ui->pbKreuz->click();
  }

  ui->pbDruecken->click();
  ui->gbRule->setDisabled(true);

  ui->gbTricks->show();
  ui->gbTrick2->show();
  ui->gbTrick1->show();
  ui->gbTrick3->show();

  qDebug() << "Trump:" << QString::fromStdString(game_->trump_);
}

void Table::onUpdateSkatLayout(
    LinkTo dest) {
  QLayoutItem *item;

  // Clear existing widgets from the layout
  while ((item = ui->layoutSkatKarten->takeAt(0)) != nullptr) {
    if (item->widget()) {
      delete item->widget();
    }
    delete item;
  }

  // Loop through cards in skat
  for (Card &card : game_->skat_.cards()) {
    QPushButton *cardButton = new QPushButton(this);

    QString rankname = QString::fromStdString(card.rankname());
    QString suitname = QString::fromStdString(card.suitname());
    QString imagePath = QString(":/cards/%1_of_%2.png").arg(rankname, suitname);
    QPixmap pixmap(imagePath);

    if (cardSize_ == CardSize::Normal && !pixmap.isNull()) {
      cardButton->setIcon(QIcon(pixmap));
      cardButton->setStyleSheet("QPushBuktton { padding: 0px; margin: 0px; }");
      cardButton->setFlat(true);
      cardButton->setIconSize(QSize(78, 116));
    } else {
      cardButton->setText(QString::fromStdString(card.str()));
      cardButton->setStyleSheet("QPushButton { font-size: 18x; }");
    }

    ui->layoutSkatKarten->addWidget(cardButton);

    // Handle card actions only if LinkTo::SoloPlayer
    if (dest == LinkTo::SoloPlayer) {
      Player *soloPlayer = game_->getPlayerByIsSolo();
      if (!soloPlayer) {
        qDebug() << "onUpdateSkatLayout: solo player not found";

        return;
      }

      int playerId = soloPlayer->id();

      connect(cardButton, &QPushButton::clicked, this,
              [&, this, cardButton, soloPlayer, playerId]() {
                if (game_->skat_.size() == 2) {
                  game_->skat_.moveCardTo(card, soloPlayer->handdeck_);

                  ui->layoutSkatKarten->removeWidget(cardButton);
                  cardButton->setParent(nullptr);

                  onUpdatePlayerLayout(playerId, LinkTo::Skat);
                }
              });
    }
  }
}

void Table::onUpdatePlayerLayout(
    int playerId, LinkTo dest) {
  Player &player = game_->getPlayerById(playerId);

  QLayout *layout;
  QLayoutItem *item;

  if (playerId == 1) layout = ui->gbPlayer1Layout;
  if (playerId == 2) layout = ui->gbPlayer2Layout;
  if (playerId == 3) layout = ui->gbPlayer3Layout;

  while ((item = layout->takeAt(0)) != nullptr) {
    if (item->widget()) {
      delete item->widget();
    }
    delete item;
  }

  player.handdeck_.sortCardsFor(game_->rule_, game_->trump_);

  for (Card &card : player.handdeck_.cards()) {
    QPushButton *cardButton = new QPushButton(this);

    QString rankname = QString::fromStdString(card.rankname());
    QString suitname = QString::fromStdString(card.suitname());
    QString imagePath = QString(":/cards/%1_of_%2.png").arg(rankname, suitname);
    QPixmap pixmap(imagePath);

    if (cardSize_ == CardSize::Normal && !pixmap.isNull()) {
      cardButton->setIcon(QIcon(pixmap));
      cardButton->setStyleSheet("QPushButton { padding: 0px; margin: 0px; }");
      cardButton->setFlat(true);
      cardButton->setIconSize(
          QSize(78, 116));  // Set the icon size for the button
      cardButton->setIcon(QIcon(pixmap));
      cardButton->setIconSize(QSize(78, 116));
    } else {
      cardButton->setText(QString::fromStdString(card.str()));
      cardButton->setStyleSheet("QPushButton { font-size: 18px; }");
    }
    layout->addWidget(cardButton);

    // connect to skat
    if (dest == LinkTo::Skat) {
      connect(
          cardButton, &QPushButton::clicked, this,
          [&, layout,
           cardButton]() {  // Bugfix: program crashed w/o layout, cardButton
            if (game_->skat_.size() == 1) {
              player.handdeck_.moveCardTo(card, game_->skat_);
              layout->removeWidget(cardButton);
              cardButton->setParent(nullptr);
              // update Layout after card was moved to skat
              onUpdateSkatLayout(LinkTo::SoloPlayer);
            }
          });
    }

    // connect to trick
    if (dest == LinkTo::Trick) {
      connect(cardButton, &QPushButton::clicked, this,
              [&, playerId, layout, cardButton]() {
                if (game_->playerList_.front()->id() == playerId) {
                  qDebug() << "card clicked:"
                           << QString::fromStdString(card.str());

                  if (!player.handdeck_.isCardInside(card) ||
                      !game_->isCardValid(card)) {
                    qDebug() << "Move rejected: Invalid card choice.";

                    return;
                  }

                  onUpdateTrickLayout(card, playerId);

                  game_->playCard(card);

                  layout->removeWidget(cardButton);
                  cardButton->setParent(nullptr);
                  // update Layout after card was played to trick
                  onUpdatePlayerLayout(playerId, LinkTo::Trick);
                }
              });
    }
  }
}

void Table::onUpdateTrickLayout(
    const Card &card, int playerId) {
  QPushButton *cardButton = new QPushButton(this);

  QString rankname = QString::fromStdString(card.rankname());
  QString suitname = QString::fromStdString(card.suitname());
  QString imagePath = QString(":/cards/%1_of_%2.png").arg(rankname, suitname);
  QPixmap pixmap(imagePath);

  if (cardSize_ == CardSize::Normal && !pixmap.isNull()) {
    cardButton->setIcon(QIcon(pixmap));
    cardButton->setFlat(true);
    cardButton->setIconSize(QSize(78, 116));
  } else {
    cardButton->setText(QString::fromStdString(card.str()));
    cardButton->setStyleSheet("QPushButton { font-size: 18px; }");
  }

  if (playerId == 2) ui->gbTrickLayout2->addWidget(cardButton);
  if (playerId == 1) ui->gbTrickLayout1->addWidget(cardButton);
  if (playerId == 3) ui->gbTrickLayout3->addWidget(cardButton);
}

void Table::onClearTrickLayout() {
  while (QLayoutItem *item = ui->gbTrickLayout1->takeAt(0)) {
    if (QWidget *widget = item->widget()) {
      widget->deleteLater();  // Ensures safe deletion after event loop
    }
    delete item;  // Cleanup the layout item itself
  }
  while (QLayoutItem *item = ui->gbTrickLayout2->takeAt(0)) {
    if (QWidget *widget = item->widget()) {
      widget->deleteLater();  // Ensures safe deletion after event loop
    }
    delete item;  // Cleanup the layout item itself
  }
  while (QLayoutItem *item = ui->gbTrickLayout3->takeAt(0)) {
    if (QWidget *widget = item->widget()) {
      widget->deleteLater();  // Ensures safe deletion after event loop
    }
    delete item;  // Cleanup the layout item itself
  }
}

void Table::onResultat() {
  qDebug() << "onResultat";
  Player *player;

  if (game_->rule_ == Rule::Ramsch)
    player = game_->getPlayerByMostTricksPoints();
  else
    player = game_->getPlayerByIsSolo();

  QString resultat;
  bool ueberreizt = game_->gereizt_ > game_->spielwert_;
  bool gewonnen = (player->success_ && not ueberreizt);

  QString spielZeile;
  if (game_->rule_ == Rule::Grand || game_->rule_ == Rule::Suit)
    spielZeile = QString("mit %1 zu %2 Augen.\n")
                     .arg(player->points())
                     .arg(120 - player->points());
  else if (game_->rule_ == Rule::Null)
    spielZeile = "das Nullspiel.\n";
  else
    spielZeile = QString("den Ramsch.\n");

  resultat = QString(
                 "%7\n"
                 "%1 %2\n"
                 "%3"
                 "Gereizt bis %4.\n"
                 "Der Spielwert ist %5.\n"
                 "%6 %8 %9\n")
                 .arg(QString::fromStdString(player->name()))  // %1
                 .arg(gewonnen ? "gewinnt" : "verliert")       // %2
                 .arg(spielZeile)                              // %3
                 .arg(game_->gereizt_)                         // %4
                 .arg(game_->spielwertFinishRound_)            // %5
                 .arg(ueberreizt ? "\nÜberreizt !" : "")       // %6
                 .arg(game_->kontra_ ? "kontra!" : "")         // %7
                 .arg(game_->re_ ? "re!" : "")                 // %8
                 .arg(game_->bock_ ? "bock!" : "");            // %9

  ui->lblScore1->setText(QString::number(game_->player_1.score()));
  ui->lblScore2->setText(QString::number(game_->player_2.score()));
  ui->lblScore3->setText(QString::number(game_->player_3.score()));

  ui->lblStatistik1->setText(QString("%1 / %2")
                                 .arg(game_->player_1.spieleGewonnen_)
                                 .arg(game_->player_1.spieleVerloren_));
  ui->lblStatistik2->setText(QString("%1 / %2")
                                 .arg(game_->player_2.spieleGewonnen_)
                                 .arg(game_->player_2.spieleVerloren_));
  ui->lblStatistik3->setText(QString("%1 / %2")
                                 .arg(game_->player_3.spieleGewonnen_)
                                 .arg(game_->player_3.spieleVerloren_));

  ui->gbSagenPassen->hide();

  ui->lblSpielwert->setText(resultat);
  ui->gbResultat->show();

  // ui->gbTricks->hide();
  // ui->gbTrick2->hide();
  // ui->gbTrick1->hide();
  // ui->gbTrick3->hide();

  ui->lblUrsprungsSkat->show();

  game_->skat_ = game_->urSkat_;
  onUpdateSkatLayout();
  ui->gbSkat->show();
  game_->skat_.cards().clear();
}

void Table::mousePressEvent(
    QMouseEvent *event) {
  if (event->button() == Qt::RightButton) {
    qDebug() << "Right click detected inside window!";

    game_->autoplay();
    return;
  }
  QMainWindow::mousePressEvent(event);  // Default behavior
}

/*
 *Spieltyp              Schneider     Schwarz
Grand Hand                Nein          Nein
Grand Hand mit Schneider  Ja            Nein
Grand Hand mit Schwarz    Ja            Ja
Grand Hand Ouvert         Ja            Ja (automatisch)
 */

void Table::setButtonLogic() {
  if (game_->rule_ == Rule::Grand || game_->rule_ == Rule::Suit) {
    ui->pbHand->setVisible(game_->hand_);
    ui->pbOuvert->setVisible(game_->hand_);

    // Schneider is always announced if playing Grand Hand
    ui->pbSchneider->setVisible(game_->hand_);
    game_->schneiderAngesagt_ = false;
    ui->pbSchneider->setChecked(false || game_->ouvert_);
    ui->pbSchneider->setDisabled(false || game_->ouvert_);

    // Schwarz is only announced if Grand Hand Ouvert is played
    ui->pbSchwarz->setVisible(game_->hand_);
    game_->schwarzAngesagt_ = false;
    ui->pbSchwarz->setChecked(false || game_->ouvert_);
    ui->pbSchwarz->setDisabled(false || game_->ouvert_);

    ui->pbOuvert->setVisible(game_->hand_);
  }

  if (game_->rule_ == Rule::Null) {
    ui->pbOuvert->setVisible(true);
    ui->pbSchneider->setVisible(false);
    ui->pbSchwarz->setVisible(false);
  }
}

Table::~Table() { delete ui; }
