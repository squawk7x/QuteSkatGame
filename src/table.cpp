#include "table.h"

#include <QTimer>

#include "ui_table.h"

Table::Table(
    QWidget *parent)
    : QMainWindow(parent), ui(new Ui::Skattisch), game_(new Game(this)) {
  ui->setupUi(this);
  game_->initGame();

  // connect pushbuttons
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
    QObject::connect(ui->pbGrand, &QPushButton::clicked, this, [this]() {
      game_->rule_ = Rule::Grand;
      game_->trump_ = "J";
      qDebug() << "trump_ set to" << QString::fromStdString(game_->trump_);
    });
    QObject::connect(ui->pbNull, &QPushButton::clicked, this, [this]() {
      game_->rule_ = Rule::Null;
      game_->trump_ = "";
      qDebug() << "trump_ set to" << QString::fromStdString(game_->trump_);
    });
    QObject::connect(ui->pbRamsch, &QPushButton::clicked, this, [this]() {
      game_->rule_ = Rule::Ramsch;
      game_->trump_ = "J";
      qDebug() << "trump_ set to" << QString::fromStdString(game_->trump_);
    });

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
    QObject::connect(ui->pbSchwarz, &QPushButton::toggled, this,
                     [this](bool checked) {
                       game_->schwarz_ = checked;
                       qDebug() << "schwarz_ set to" << game_->schwarz_;
                     });

    // Connections
    QObject::connect(game_, &Game::clearTrickLayout, this,
                     &Table::onClearTrickLayout);

    QObject::connect(ui->pbSagen, &QPushButton::clicked, this, [this]() {
      ui->pbSagen->setText(QString::number(game_->gereizt_));
      updateSkatLayout(true);
    });

    // Reizen
    // QObject::connect(game_, &Game::geboten, ui->pbSagen,
    // &QPushButton::click);

    QObject::connect(ui->pbDruecken, &QPushButton::clicked, this, [this]() {
      if (game_->skat_.cards().size() == 2) {
        // disconnect skat
        updateSkatLayout(false);
        for (int playerId = 1; playerId <= 3; playerId++)
          // connect all players to stick
          updatePlayerLayout(playerId, 2);
      }
    });

    QObject::connect(ui->pbPassen, &QPushButton::clicked, this, [this]() {
      qDebug() << "" << QString::fromStdString("");
    });
  }

  // without connect (false)
  updateSkatLayout(false);

  // without connect (0)
  for (int playerId = 1; playerId <= 3; playerId++)
    updatePlayerLayout(playerId, 0);

  game_->startGame();
}

void Table::updateSkatLayout(
    bool open) {
  QLayoutItem *item;

  while ((item = ui->gbSkatLayout->takeAt(0)) != nullptr) {
    if (item->widget()) {
      delete item->widget();  // Delete the widget (if it exists)
    }
    delete item;  // Delete the layout item
  }
  for (const Card &card : game_->skat_.cards()) {
    QPushButton *cardButton = new QPushButton(this);
    cardButton->setText(QString::fromStdString(card.str()));
    ui->gbSkatLayout->addWidget(cardButton);

    if (open) {
      Player *player = game_->getPlayerByIsSolo();
      int playerId = player->id();

      connect(cardButton, &QPushButton::clicked, this, [=, this]() {
        game_->skat_.moveCardTo(card, game_->getPlayerByIsSolo()->handdeck_);
        ui->gbSkatLayout->removeWidget(cardButton);
        cardButton->setParent(nullptr);
        qDebug() << game_->skat_.cards().size();
        qDebug() << game_->getPlayerByIsSolo()->handdeck_.cards().size();
        updatePlayerLayout(playerId, 1);
      });
    }
  }
}

void Table::updatePlayerLayout(
    int playerId, int dest) {
  Player &player = game_->getPlayerById(playerId);

  QLayout *layout;
  if (playerId == 1) layout = ui->gbPlayer1Layout;
  if (playerId == 2) layout = ui->gbPlayer2Layout;
  if (playerId == 3) layout = ui->gbPlayer3Layout;
  QLayoutItem *item;

  while ((item = layout->takeAt(0)) != nullptr) {
    if (item->widget()) {
      delete item->widget();  // Delete the widget (if it exists)
    }
    delete item;  // Delete the layout item
  }

  for (const Card &card : player.handdeck_.cards()) {
    QPushButton *cardButton = new QPushButton(this);
    cardButton->setText(QString::fromStdString(card.str()));
    layout->addWidget(cardButton);

    // connect to skat
    if (dest == 1) {
      connect(
          cardButton, &QPushButton::clicked, this,
          [&, this, layout,
           cardButton]() {  // Bugfix: program crashed w/o layout, cardButton
            if (game_->skat_.cards().size() < 2) {
              layout->removeWidget(cardButton);
              cardButton->setParent(nullptr);
              player.handdeck_.moveCardTo(card, game_->skat_);
              updateSkatLayout(true);
            }
          });
    }

    // connect to trick
    if (dest == 2) {
      connect(cardButton, &QPushButton::clicked, this,
              [&, this, card, playerId,
               layout,          // Bugfix w/o card card mismatch in layout
               cardButton]() {  // Bugfix: program crashed w/o playerId, layout,
                                // cardButton
                if (game_->playerList_.front()->id() == playerId) {
                  if (!player.handdeck_.isCardInside(card) ||
                      !game_->isCardValid(card, game_->rule_)) {
                    qDebug() << "Move rejected: Invalid card choice.";
                    return;
                  }
                  layout->removeWidget(cardButton);
                  cardButton->setParent(nullptr);
                  game_->playCard(card);
                  ui->gbTrickLayout->addWidget(cardButton);
                }
              });
    }
  }
}

// Slots
void Table::onClearTrickLayout() {
  while (QLayoutItem *item = ui->gbTrickLayout->takeAt(0)) {
    if (QWidget *widget = item->widget()) {
      widget->deleteLater();  // Ensures safe deletion after event loop
    }
    delete item;  // Cleanup the layout item itself
  }
}

Table::~Table() { delete ui; }
