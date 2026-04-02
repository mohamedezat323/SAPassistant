#include "widget.h"
#include "ui_widget.h"
#include <QSettings>
#include <QFileDialog>
#include <QFileInfo>
#include <QDesktopServices>
#include <QUrl>
#include <QScreen>
#include <QGuiApplication>
#include <QCloseEvent>
#include <QShortcut>
#include <QKeySequence>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTimer>
#include <QDateTime>
#include <QFile>
#include <QTextStream>

QString Widget::buttonSettingsKey(int index) const
{
    return QString("button%1").arg(index + 1);
}
// this a switch data table that references each element from the ui to a unique "case" number ie.case 0 referes to button 1, added + 1 cause I am lazy
// later we can use loops to refere toeach button ie
//for (int i = 0; i < 8; ++i) {
//QPushButton *btn = buttonForIndex(i);
//btn->setText("Click Me!");
//}   btn->setText("Hello"); (The arrow means "follow the pointer to the actual object and then do this")

QPushButton *Widget::buttonForIndex(int index) const
{
    switch (index)
    {
    case 0: return ui->button1;
    case 1: return ui->button2;
    case 2: return ui->button3;
    case 3: return ui->button4;
    case 4: return ui->button5;
    case 5: return ui->button6;
    case 6: return ui->button7;
    case 7: return ui->button8;
    default: return nullptr;
    }
}


void Widget::selectFileForIndex(int index)
{
    QPushButton *btn = buttonForIndex(index);
    if (!btn)
        return;

    const QString filePath = QFileDialog::getOpenFileName(this, "Select File");
    if (filePath.isEmpty())
        return;

    const QFileInfo info(filePath);
    btn->setText(info.completeBaseName());

    QSettings settings(QCoreApplication::applicationDirPath() + "/settings.ini", QSettings::IniFormat);
    settings.setValue(buttonSettingsKey(index), filePath);
}

void Widget::openFileForIndex(int index)
{
    QSettings settings(QCoreApplication::applicationDirPath() + "/settings.ini", QSettings::IniFormat);
    const QString filePath = settings.value(buttonSettingsKey(index), "").toString();

    if (!filePath.isEmpty())
        QDesktopServices::openUrl(QUrl::fromLocalFile(filePath));
}
//////////////////////////
void Widget::on_AOT_toggled(bool checked)
{
    this->setWindowFlag(Qt::WindowStaysOnTopHint, checked);
    this->show(); // reapply flags
}
///////////////////////////
void Widget::closeEvent(QCloseEvent *event)
{
    QSettings settings(QCoreApplication::applicationDirPath() + "/settings.ini", QSettings::IniFormat);

    // Save window position
    settings.setValue("windowPos", this->pos());

    QWidget::closeEvent(event); // call base class
}
//////////////////////////
bool Widget::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->dragWidget)
    {
        if (event->type() == QEvent::MouseButtonPress)
        {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
            if (mouseEvent->button() == Qt::LeftButton)
            {
                m_dragging = true;
                m_dragPosition = mouseEvent->globalPosition().toPoint() - this->frameGeometry().topLeft();
                return true; // consume event
            }
        }
        else if (event->type() == QEvent::MouseMove)
        {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
            if (m_dragging && (mouseEvent->buttons() & Qt::LeftButton))
            {
                this->move(mouseEvent->globalPosition().toPoint() - m_dragPosition);
                return true; // consume event
            }
        }
        else if (event->type() == QEvent::MouseButtonRelease)
        {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
            if (mouseEvent->button() == Qt::LeftButton)
            {
                m_dragging = false;
                return true;
            }
        }
    }

    // pass other events to base class
    return QWidget::eventFilter(obj, event);
}
////////////////////////
////////////////////////
////////////////////////
void Widget::logWeather(double temp, double hum, const QString &timestamp)
{
    QFile file("weather_log.txt");
    ui->logTableView->scrollToBottom();

    if (file.open(QIODevice::Append | QIODevice::Text))
    {
        QTextStream out(&file);
        out << timestamp
            << " | Temp: " << temp
            << "°C | Humidity: " << hum << "%\n";

        file.close();

    }
    int row = logModel->rowCount();
    logModel->insertRow(row);
    logModel->setData(logModel->index(row, 0), timestamp);
    logModel->setData(logModel->index(row, 1), temp);
    logModel->setData(logModel->index(row, 2), hum);
    QString status;
    if (hum >= 70)
    {
        status = "⚠️ Alert";
    }
    else
    {
        status = "OK";
    }

    logModel->setData(logModel->index(row, 3), status);
    //color rows
    if (hum >= 70)
    {
        int col = 3;
        {
            logModel->setData(logModel->index(row, col),
                              QBrush(Qt::red),
                              Qt::BackgroundRole);
        }
    }

}
////////////////////////
void Widget::recoverMissedLogs()
{
    QSettings settings(QCoreApplication::applicationDirPath() + "/settings.ini", QSettings::IniFormat);

    QString lat = settings.value("latitude", "30.6").toString();
    QString lon = settings.value("longitude", "32.27").toString();

    QString url = QString("http://api.open-meteo.com/v1/forecast?"            // Have to fix openssl to use HTTPS protocol
                          "latitude=%1&longitude=%2&"
                          "hourly=temperature_2m,relativehumidity_2m&"
                          "timezone=auto&past_days=7")
                      .arg(lat)
                      .arg(lon);

    QNetworkRequest request{QUrl(url)};

    // ✅ CORRECT WAY
    QNetworkReply *reply = networkManager->get(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        handleRecoveryReply(reply);
    });
}
///////////////////////
void Widget::handleRecoveryReply(QNetworkReply *reply)
{

    QByteArray data = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);

    QJsonObject obj = doc.object();
    QJsonObject hourly = obj["hourly"].toObject();

    QJsonArray times = hourly["time"].toArray();
    QJsonArray temps = hourly["temperature_2m"].toArray();
    QJsonArray hums = hourly["relativehumidity_2m"].toArray();

    QStringList targetTimes;


    for (int d = 0; d < 3; ++d)
    {
        QString day = QDate::currentDate().addDays(-d).toString("yyyy-MM-dd");

        targetTimes << day + "T08:00";
        targetTimes << day + "T12:00";
        targetTimes << day + "T16:00";
    };

    for (int i = 0; i < times.size(); ++i)
    {
        QString t = times[i].toString();

        if (t.contains("T08:00") || t.contains("T12:00") || t.contains("T16:00"))
        {
            double temp = temps[i].toDouble();
            double hum = hums[i].toDouble();

            // Avoid duplicate logging
            if (!isAlreadyLogged(t))
            {
                logWeather(temp, hum, t);
            }
        }
    }

    reply->deleteLater();
}
/////////////////////////
bool Widget::isAlreadyLogged(const QString &timestamp)           // || update needed || This function will get slower because it has to read the whole file from the top every time.
{                                                                //Function inside logWeather can be used.
    QFile file("weather_log.txt");

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    QTextStream in(&file);

    while (!in.atEnd())   // (!in.atEnd() means "not at the end ( ! means not )
    {
        QString line = in.readLine(); // created a variable of the line that is currently being read by the while loop in the form of string and named line
        if (line.contains(timestamp)) // if the timestamp equals to "contains" the string line return true
            return true;
    }

    return false;
}
////////////////////////
void Widget::loadLogsFromFile()
{
    QFile file("weather_log.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QTextStream in(&file);
    while (!in.atEnd())
    {
        QString line = in.readLine();
        // Example line: 2026-03-21T08:00 | Temp: 15.3°C | Humidity: 63%
        QStringList parts = line.split('|');
        if (parts.size() != 3) continue;

        int row = logModel->rowCount();
        logModel->insertRow(row);

        QString timestamp = parts[0].trimmed();

        double temp = parts[1]
                          .replace("Temp:", "")
                          .replace("°C", "")
                          .trimmed()
                          .toDouble();

        double hum = parts[2]
                         .replace("Humidity:", "")
                         .replace("%", "")
                         .trimmed()
                         .toDouble();

        logModel->setData(logModel->index(row, 0), timestamp);
        logModel->setData(logModel->index(row, 1), temp);
        logModel->setData(logModel->index(row, 2), hum);


        QString status;
        if (hum >= 70)
        {
            status = "⚠️ Alert";
        }
        else
        {
            status = "OK";
        }

        logModel->setData(logModel->index(row, 3), status);

        //color rows
        if (hum >= 70)
        {
            int col = 3;
            {
                logModel->setData(logModel->index(row, col),
                                  QBrush(Qt::red),
                                  Qt::BackgroundRole);
            }
        }

    }
    file.close();
}
//////////////////////////////
void Widget::on_saveLocationButton_clicked()
{
    QString lat = ui->latEdit->text();
    QString lon = ui->lonEdit->text();

    QSettings settings(QCoreApplication::applicationDirPath() + "/settings.ini", QSettings::IniFormat);
    settings.setValue("latitude", lat);
    settings.setValue("longitude", lon);
}
////////////////////////
Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{

    ///////////////////
    ui->setupUi(this);

    // -------------------------
    // Weather contsturct
    networkManager = new QNetworkAccessManager(this);
    recoverMissedLogs();
    // change long and and lat and save new values to QSittings

    // -------------------------
    logModel = new QStandardItemModel(this);
    logModel->setColumnCount(4);
    logModel->setHeaderData(0, Qt::Horizontal, "DateTime");
    logModel->setHeaderData(1, Qt::Horizontal, "Temperature (°C)");
    logModel->setHeaderData(2, Qt::Horizontal, "Humidity (%)");
    logModel->setHeaderData(3, Qt::Horizontal, "Status");

    // Attach model to table view
    ui->logTableView->setModel(logModel);
    ui->logTableView->horizontalHeader()->setStretchLastSection(true);
    loadLogsFromFile();
    networkManager = new QNetworkAccessManager(this);
    recoverMissedLogs();


    // buttons style
    QPushButton* buttons[8] = {
        ui->button1, ui->button2, ui->button3,
        ui->button4, ui->button5, ui->button6,
        ui->button7, ui->button8

    };

    for (int i = 0; i < 8; ++i)
    {
        buttons[i]->setStyleSheet(
            "QPushButton {"
            "   border-radius: 5px;"
            "   background-color: #696969;"
            "   border: 1px solid #000000;"
            "   text-align: center;"
            "   padding-left: 2px;"
            "}"
            "QPushButton:hover {"
            "   background-color: #4A4A4A;"
            "}"
            );
    }

    // this is a lambda function that connects the slots "selectfileforindex" and "openfileforindex" to the toolbuttons and the buttons
    // changed it to right click to change file : right click opens the pick file dialog now instead of the tool buttons and
    for (int i = 0; i < 8; ++i)
    {
        //if (QToolButton *tb = toolButtonForIndex(i))
       // {
          //  connect(tb, &QToolButton::clicked, this, [this, i]() { selectFileForIndex(i); });
       // }
        if (QPushButton *b = buttonForIndex(i))
        {
            b->setContextMenuPolicy(Qt::CustomContextMenu);
            connect(b, &QPushButton::clicked, this, [this, i]() { openFileForIndex(i); });
            connect(b, &QPushButton::customContextMenuRequested, this, [this, i](const QPoint &pos) {
                selectFileForIndex(i);
            });
        }
    }

    QSettings settings(QCoreApplication::applicationDirPath() + "/settings.ini", QSettings::IniFormat);
    ui->latEdit->setText(settings.value("latitude", "30.6").toString());
    ui->lonEdit->setText(settings.value("longitude", "32.27").toString());

    // Load button labels from settings (show base name only).
    for (int i = 0; i < 8; ++i)
    {
        if (QPushButton *b = buttonForIndex(i))
        {
            const QString filePath = settings.value(buttonSettingsKey(i), "").toString();
            if (!filePath.isEmpty())
            {
                const QFileInfo info(filePath);
                b->setText(info.completeBaseName());
            }
            else
            {
                b->setText("Select File");
            }
        }
    }

    // Load last saved position
    QPoint pos = settings.value("windowPos", QPoint(-1, -1)).toPoint();

    if (pos != QPoint(-1, -1))
    {
        this->move(pos);
    }
    else
    {
        // Default: bottom-right if no saved position
        QScreen *screen = QGuiApplication::primaryScreen();
        QRect screenGeometry = screen->availableGeometry();
        int x = screenGeometry.right() - this->width();
        int y = screenGeometry.bottom() - this->height();
        this->move(x, y);
    }
    this->setWindowFlag(Qt::WindowStaysOnTopHint, ui->AOT->isChecked());
    this->show();
    this->setWindowFlag(Qt::FramelessWindowHint);
    this->setWindowOpacity(0.80);
    ui->dragWidget->installEventFilter(this);


}


Widget::~Widget()
{

    delete ui;

}


//✅ Add a “View Logs” button in your UI
//✅ Show logs inside your app (table view)
//✅ Export logs to Excel (