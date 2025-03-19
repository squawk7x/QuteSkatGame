#include "table.h"

#include <QTimer>

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

    QObject::connect(ui->pbKaro, &QPushButton::clicked, this, [this]() {
      game_->rule_ = Rule::Suit;
      game_->trump_ = "♦";
      qDebug() << "trump_ set to" << QString::fromStdString(game_->trump_);
    });
    QObject::connect(ui->pbHerz, &QPushButton::clicked, this, [this]() {
      game_->rule_ = Rule::Suit;
      game_->trump_ = "♥";
      qDebug() << "trump_ set to" << QString::fromStdString(game_->trump_);
    });
    QObject::connect(ui->pbPik, &QPushButton::clicked, this, [this]() {
      game_->rule_ = Rule::Suit;
      game_->trump_ = "♠";
      qDebug() << "trump_ set to" << QString::fromStdString(game_->trump_);
    });
    QObject::connect(ui->pbKreuz, &QPushButton::clicked, this, [this]() {
      game_->rule_ = Rule::Suit;
      game_->trump_ = "♣";
      qDebug() << "trump_ set to" << QString::fromStdString(game_->trump_);
    });
    QObject::connect(ui->pbNull, &QPushButton::clicked, this, [this]() {
      game_->rule_ = Rule::Null;
      game_->trump_ = "";
      qDebug() << "trump_ set to" << QString::fromStdString(game_->trump_);
    });
    QObject::connect(ui->pbGrand, &QPushButton::clicked, this, [this]() {
      game_->rule_ = Rule::Grand;
      game_->trump_ = "J";
      qDebug() << "trump_ set to" << QString::fromStdString(game_->trump_);
    });
    QObject::connect(ui->pbRamsch, &QPushButton::clicked, this, [this]() {
      game_->rule_ = Rule::Ramsch;
      game_->trump_ = "J";
      qDebug() << "trump_ set to" << QString::fromStdString(game_->trump_);
    });

    QObject::connect(game_, &Game::ramsch, this, [this]() {
      ui->pbRamsch->click();
      ui->pbRamsch->setDisabled(true);
      ui->pbDruecken->click();
      ui->gbRule->setDisabled(true);
    });

    // Spiel Hand, Ouvert, Schneider, Schwarz
    QObject::connect(ui->pbHand, &QPushButton::toggled, this,
                     [this](bool checked) {
                       game_->hand_ = checked;
                       qDebug() << "hand_ set to" << game_->hand_;
                     });

    QObject::connect(ui->pbOuvert, &QPushButton::toggled, this,
                     [this](bool checked) {
                       game_->ouvert_ = checked;
                       qDebug() << "ouvert_ set to" << game_->ouvert_;
                     });
    QObject::connect(ui->pbSchneider, &QPushButton::toggled, this,
                     [this](bool checked) {
                       game_->schneider_ = checked;
                       qDebug() << "schneider_ set to" << game_->schneider_;
                     });
    QObject::connect(
        ui->pbSchwarz, &QPushButton::toggled, this, [this](bool checked) {
          if (checked) {
            ui->pbSchneider->setChecked(
                true);  // Wenn schwarz angesagt dann auch Schneider
            ui->pbSchneider->setDisabled(true);
          } else
            ui->pbSchneider->setDisabled(false);
          game_->schwarz_ = checked;

          qDebug() << "schneider_/schwarz_ set to" << game_->schneider_ << "/"
                   << game_->schwarz_;
        });

    // Resultat
    QObject::connect(game_, &Game::resultat, this, &Table::onResultat);

    QObject::connect(ui->pbHandNein, &QPushButton::clicked, this, [this]() {
      ui->pbHand->setChecked(false);
      ui->pbHand->setDisabled(false);
      ui->gbFrageHand->hide();
      ui->gbDruecken->show();
      updateSkatLayout(LinkTo::Handdeck);
    });

    QObject::connect(ui->pbHandJa, &QPushButton::clicked, this, [this]() {
      ui->pbHand->click();
      ui->pbHand->setChecked(true);
      ui->pbHand->setDisabled(true);
      ui->pbDruecken->click();  // even if not visible
      updateSkatLayout(LinkTo::Skat);
    });

    QObject::connect(ui->pbSagen, &QPushButton::clicked, this, [this]() {
      // Player *player = &game_->getPlayerById(1);
      game_->bieten();
    });

    QObject::connect(ui->pbBieten2, &QPushButton::clicked, this, [this]() {
      // Player *player = &game_->getPlayerById(1);
      game_->bieten();
    });

    QObject::connect(ui->pbBieten3, &QPushButton::clicked, this, [this]() {
      // Player *player = &game_->getPlayerById(1);
      game_->bieten();
    });

    QObject::connect(ui->pbPassen, &QPushButton::clicked, this, [this]() {
      game_->bieten(true);  // bieten (bool passe)
    });

    // Game Control
    QObject::connect(ui->pbDruecken, &QPushButton::clicked, this, [this]() {
      if (game_->skat_.cards().size() == 2) {
        game_->druecken();
        updateSkatLayout(LinkTo::Skat);
        ui->gbFrageHand->hide();
        ui->gbDruecken->hide();
        ui->gbSkat->hide();
        ui->gbTrick2->hide();
        ui->gbTrick1->hide();
        ui->gbTrick3->hide();
        ui->lblSpielGereiztBis->setText("Gereizt bis: " +
                                        QString::number(game_->gereizt_));
        ui->gbSpiel->show();
      }
    });

    QObject::connect(ui->pbSpielen, &QPushButton::clicked, this, [this]() {
      // ui->gbSpiel->hide(); // for testing shown
      ui->pbBieten2->hide();
      ui->pbBieten3->hide();
      ui->lblHand2->setText("");
      ui->lblHand3->setText("");
      ui->gbTrick2->show();
      ui->gbTrick1->show();
      ui->gbTrick3->show();
    });

    QObject::connect(ui->pbStart, &QPushButton::clicked, game_, &Game::start);
    QObject::connect(ui->pbSpielwert, &QPushButton::clicked, game_,
                     &Game::spielwert);

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

    // Layouts
    QObject::connect(game_, &Game::clearTrickLayout, this,
                     &Table::onClearTrickLayout);

    QObject::connect(game_, &Game::refreshSkatLayout, this,
                     &Table::updateSkatLayout);

    QObject::connect(game_, &Game::refreshTrickLayout, this,
                     &Table::updateTrickLayout);

    QObject::connect(game_, &Game::refreshPlayerLayout, this,
                     &Table::updatePlayerLayout);

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
  {
    ui->lblUrsprungsSkat->hide();
    ui->gbSkat->show();

    ui->gbSagenPassen->show();
    ui->pbBieten2->show();
    ui->pbBieten3->show();

    ui->gbFrageHand->hide();
    ui->gbDruecken->hide();
    ui->gbAngesagt->hide();
    ui->gbFrageEnde->hide();

    ui->gbSpiel->hide();
    ui->pbRamsch->setDisabled(false);
    ui->pbRamsch->clicked(false);
    ui->pbHand->setDisabled(false);
    ui->pbHand->clicked(false);
    ui->pbSchneider->setDisabled(false);
    ui->pbSchneider->clicked(false);
    ui->pbSchwarz->clicked(false);
    ui->gbRule->setDisabled(false);

    ui->gbTrick2->hide();
    ui->gbTrick1->hide();
    ui->gbTrick3->hide();

    // ui->gbFrageEnde->hide();
    ui->gbResultat->hide();

    onClearTrickLayout();
  }

  for (int playerId = 1; playerId <= 3; playerId++)
    updatePlayerLayout(playerId);

  updateSkatLayout(LinkTo::Skat);

  if (game_->geberHoererSagerPos_[0] == 2) ui->lblHand2->setText("Hinterhand");
  if (game_->geberHoererSagerPos_[1] == 2) ui->lblHand2->setText("Vorhand");
  if (game_->geberHoererSagerPos_[2] == 2) ui->lblHand2->setText("Mittelhand");
  if (game_->geberHoererSagerPos_[0] == 3) ui->lblHand3->setText("Hinterhand");
  if (game_->geberHoererSagerPos_[1] == 3) ui->lblHand3->setText("Vorhand");
  if (game_->geberHoererSagerPos_[2] == 3) ui->lblHand3->setText("Mittelhand");
}

void Table::onGeboten(
    int idSager, int idHoerer, QString antwortSager, QString antwortHoerer) {
  qDebug() << "Spieler" << idSager << "sagt" << antwortSager;
  qDebug() << "Spieler" << idHoerer << "sagt" << antwortHoerer;

  ui->gbSagenPassen->hide();
  ui->pbBieten2->hide();
  ui->pbBieten3->hide();

  switch (idSager) {
    case 1:
      ui->pbSagen->setText(antwortSager);
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
      ui->pbSagen->setText(antwortHoerer);
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

void Table::onResultat() {
  Player *player = game_->getPlayerByIsSolo();
  QString resultat;

  if (player) {
    bool gewonnen = player->points() > 60;
    resultat = QString("%1 %2\nmit %3 zu %4 Augen.\nDer Spielwert ist %5.")
                   .arg(QString::fromStdString(player->name()))
                   .arg(gewonnen ? "gewinnt" : "verliert")
                   .arg(player->points())
                   .arg(120 - player->points())
                   .arg(game_->gereizt_);
  }

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

  ui->lblSpielwert->setText(resultat);
  ui->gbResultat->show();

  ui->gbTrick2->hide();
  ui->gbTrick1->hide();
  ui->gbTrick3->hide();

  ui->lblUrsprungsSkat->show();

  CardVec skat = game_->skat_;
  game_->skat_ = game_->urSkat_;
  updateSkatLayout();
  ui->gbSkat->show();
  game_->skat_ = skat;
}

void Table::updateSkatLayout(
    LinkTo dest) {
  QLayoutItem *item;

  // Clear existing widgets from the layout

  while ((item = ui->layoutSkatKarten->takeAt(0)) != nullptr) {
    if (item->widget()) {
      delete item->widget();
    }
    delete item;
  }

  // Loop through cards in the skat
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

    // if (dest == LinkTo::Skat) return;

    // Handle card actions only if `hand` is true
    if (dest == LinkTo::Handdeck) {
      Player *player = game_->getPlayerByIsSolo();
      if (!player) return;

      int playerId = player->id();
      connect(cardButton, &QPushButton::clicked, this,
              [&, this, cardButton, playerId]() {
                game_->skat_.moveCardTo(std::move(card),
                                        game_->getPlayerByIsSolo()->handdeck_);

                // Remove the button from layout and set parent to nullptr
                ui->layoutSkatKarten->removeWidget(cardButton);
                cardButton->setParent(nullptr);

                // Update the player layout after moving the card
                updatePlayerLayout(playerId, LinkTo::Skat);
              });
    }
  }
}

void Table::updatePlayerLayout(
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
  player.handdeck_.sortJacksSuits();

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
            if (game_->skat_.cards().size() < 2) {
              player.handdeck_.moveCardTo(std::move(card), game_->skat_);
              cardButton->setParent(nullptr);
              layout->removeWidget(cardButton);
            }
            updateSkatLayout(LinkTo::Handdeck);
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

                  updateTrickLayout(card, playerId);

                  game_->playCard(card);

                  cardButton->setParent(nullptr);
                  layout->removeWidget(cardButton);
                  // updatePlayerLayout(playerId, LinkTo::Trick);
                }
              });
      updateSkatLayout(LinkTo::Skat);
    }
  }
}

void Table::updateTrickLayout(
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

// Slots
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

void Table::mousePressEvent(
    QMouseEvent *event) {
  if (event->button() == Qt::RightButton) {
    qDebug() << "Right click detected inside window!";
    game_->autoplay();
    return;
  }
  QMainWindow::mousePressEvent(event);  // Default behavior
}

Table::~Table() { delete ui; }
