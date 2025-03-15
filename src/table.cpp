#include "table.h"

#include <QTimer>

#include "src/ui_table.h"

Table::Table(
    QWidget *parent)
    : QMainWindow(parent), ui(new Ui::Skattisch), game_(new Game(this)) {
  ui->setupUi(this);

  // Spiel Farbe, Grand, Null, Ramsch
  {
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

    // Karten geben
    QObject::connect(game_, &Game::gegeben, this, &Table::onGegeben);

    // Reizen
    QObject::connect(game_, &Game::geboten, this, &Table::onGeboten);

    // Frage Hand
    QObject::connect(game_, &Game::hand, this, &Table::onHand);

    QObject::connect(ui->pbHandNein, &QPushButton::clicked, this, [this]() {
      ui->pbHandJa->hide();
      ui->pbHandNein->hide();
      ui->pbHand->setChecked(false);
      ui->pbHand->setDisabled(true);
      ui->pbDruecken->show();
      updateSkatLayout(LinkTo::Handdeck);
      // make card face visible
    });

    QObject::connect(ui->pbHandJa, &QPushButton::clicked, this, [this]() {
      ui->pbHand->click();
      ui->pbHand->setChecked(true);
      ui->pbHand->setDisabled(true);
      ui->pbDruecken->click();  // even if not visible
      updateSkatLayout(LinkTo::Skat);
      // leave skat face invisible
    });

    QObject::connect(ui->pbSagen, &QPushButton::clicked, this, [this]() {
      Player *player = &game_->getPlayerById(1);
      game_->bieten();
      // updateSkatLayout(LinkTo::Skat);
    });

    QObject::connect(ui->pbPassen, &QPushButton::clicked, this, [this]() {
      game_->bieten(true);  // bieten (bool passe)
      // updateSkatLayout(true);
      // updateSkatLayout(LinkTo::Handdeck);
      // updateSkatLayout(LinkTo::Skat);
    });

    // Game Control
    QObject::connect(ui->pbDruecken, &QPushButton::clicked, this, [this]() {
      if (game_->skat_.cards().size() == 2) {
        game_->druecken();
        updateSkatLayout(LinkTo::Skat);
        ui->pbDruecken->hide();
        ui->gbSkat->hide();
        ui->gbTrick2->hide();
        ui->gbTrick1->hide();
        ui->gbTrick3->hide();
        ui->gbSpiel->show();
      }
    });

    QObject::connect(ui->pbSpielen, &QPushButton::clicked, this, [this]() {
      ui->gbSpiel->hide();
      ui->pbBieten2->hide();
      ui->pbBieten3->hide();
      ui->lblHand2->setText("");
      ui->lblHand3->setText("");
      ui->gbTrick2->show();
      ui->gbTrick1->show();
      ui->gbTrick3->show();
    });

    QObject::connect(ui->pbStart, &QPushButton::clicked, game_, &Game::start);

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

void Table::updateSkatLayout(
    LinkTo dest) {
  QLayoutItem *item;
  // Clear existing widgets from the layout

  while ((item = ui->gbSkatLayout->takeAt(0)) != nullptr) {
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

    ui->gbSkatLayout->addWidget(cardButton);

    Player *player = game_->getPlayerByIsSolo();
    if (!player) return;  // Early exit if no player is found

    // Handle card actions only if `hand` is true
    if (dest == LinkTo::Handdeck) {
      int playerId = player->id();
      connect(cardButton, &QPushButton::clicked, this,
              [&, this, cardButton, playerId]() {
                game_->skat_.moveCardTo(std::move(card),
                                        game_->getPlayerByIsSolo()->handdeck_);

                // Remove the button from layout and set parent to nullptr
                ui->gbSkatLayout->removeWidget(cardButton);
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
  player.handdeck_.sortByJandSuits();

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
              updateSkatLayout(LinkTo::Handdeck);
            }
          });
    }

    // connect to trick
    if (dest == LinkTo::Trick) {
      connect(
          cardButton, &QPushButton::clicked, this,
          [&, playerId, layout, cardButton]() {
            if (game_->playerList_.front()->id() == playerId) {
              qDebug() << "card clicked:" << QString::fromStdString(card.str());

              if (!player.handdeck_.isCardInside(card) ||
                  !game_->isCardValid(card)) {
                qDebug() << "Move rejected: Invalid card choice.";
                return;
              }

              updateTrickLayout(card, playerId);

              game_->playCard(card);

              layout->removeWidget(cardButton);
              cardButton->setParent(nullptr);
              updatePlayerLayout(playerId, LinkTo::Trick);
            }
          });
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

void Table::onGegeben() {
  {
    ui->gbSkat->show();
    ui->pbSagen->show();
    ui->pbPassen->show();
    ui->pbBieten2->show();
    ui->pbBieten3->show();
    ui->gbSpiel->hide();
    ui->gbTrick2->hide();
    ui->gbTrick1->hide();
    ui->gbTrick3->hide();
    ui->pbHandJa->hide();
    ui->pbHandNein->hide();
    ui->pbDruecken->hide();
    ui->pbRamsch->setDisabled(false);
    ui->pbRamsch->clicked(false);
    ui->pbHand->setDisabled(false);
    ui->pbHand->clicked(false);
    // ui->pbHandJa->clicked(false);
    ui->pbSchneider->setDisabled(false);
    ui->pbSchneider->clicked(false);
    ui->pbSchwarz->clicked(false);
    ui->gbRule->setDisabled(false);

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

  ui->pbSagen->setText("");
  ui->pbBieten2->setText("");
  ui->pbBieten3->setText("");

  ui->pbSagen->show();
  ui->pbBieten2->show();
  ui->pbBieten3->show();

  switch (idSager) {
    case 1:
      ui->pbSagen->setText(antwortSager);
      break;
    case 2:
      ui->pbBieten2->setText(antwortSager);
      ui->pbBieten2->animateClick();
      break;
    case 3:
      ui->pbBieten3->setText(antwortSager);
      ui->pbBieten3->animateClick();
      break;
  }

  switch (idHoerer) {
    case 1:
      ui->pbSagen->setText(antwortHoerer);
      break;
    case 2:
      ui->pbBieten2->setText(antwortHoerer);
      ui->pbBieten2->animateClick();
      break;
    case 3:
      ui->pbBieten3->setText(antwortHoerer);
      ui->pbBieten3->animateClick();
      break;
  }

  if (idSager == 1 || idHoerer == 1)
    ui->pbPassen->setText("passe");
  else
    ui->pbPassen->setText("");

  // Hide the push button for the player who is neither Sager nor Hoerer
  for (int i = 1; i <= 3; ++i) {
    if (i != idSager && i != idHoerer) {
      switch (i) {
        case 1:
          ui->pbSagen->hide();
          break;
        case 2:
          ui->pbBieten2->hide();
          break;
        case 3:
          ui->pbBieten3->hide();
          break;
      }
    }
  }
}

void Table::onHand() {
  ui->pbSagen->hide();
  ui->pbPassen->hide();
  ui->pbBieten2->hide();
  ui->pbBieten3->hide();

  ui->pbHandJa->show();
  ui->pbHandNein->show();
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
