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
#include <QDate>
#include <QFile>
#include <QTextStream>
#include <QSslSocket>
#include <QIcon>
#include <QPixmap>
#include <QSize>
#include <QToolButton>
#include <QDebug>
#include <QClipboard>
#include <windows.h>
#include <QProcess>
#include <QNetworkInterface>
#include <QCheckBox>
#include <QDir>
#include <QDialog>
#include <QSlider>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QInputDialog>

//-------------------------------------------------------------------------------------------------//
class SettingsDialog : public QDialog {
public:
    SettingsDialog(Widget *parent, QSettings *settings) : QDialog(parent), m_parent(parent), m_settings(settings) {
        setWindowTitle("Settings");

        setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);

        QVBoxLayout *layout = new QVBoxLayout(this);

        // --- Opacity Section ---//
        layout->addWidget(new QLabel("Window Opacity:"));
        QSlider *opacitySlider = new QSlider(Qt::Horizontal, this);
        opacitySlider->setRange(30, 100);
        opacitySlider->setValue(m_parent->windowOpacity() * 100);
        layout->addWidget(opacitySlider);

        connect(opacitySlider, &QSlider::valueChanged, this, [this](int value) {
            m_parent->setWindowOpacity(value / 100.0);
            m_settings->setValue("opacity", value / 100.0);
        });

        // --- Startup Section ---
        QCheckBox *startupCheck = new QCheckBox("Start with Windows", this);

        // Check current Registry state
        QSettings bootSettings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
        startupCheck->setChecked(bootSettings.contains("SAPAssistant"));

        connect(startupCheck, &QCheckBox::toggled, m_parent, &Widget::toggleAutoStart);
        layout->addWidget(startupCheck);

        // --- Close Button ---
        QPushButton *closeBtn = new QPushButton("Done", this);
        connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
        layout->addWidget(closeBtn);
    }

private:
    Widget *m_parent;
    QSettings *m_settings;
};

//-------------------------------------------------------------------------------------------------//
namespace {

QString aotIconResourcePath(bool checked)
{
    return checked ? QStringLiteral(":/icons/checked.png")
                   : QStringLiteral(":/icons/unchecked.png");
}

QIcon aotIconFromResources(bool checked)
{
    const QString path = aotIconResourcePath(checked);
    QPixmap base;
    if (!base.load(path)) {
        return {};
    }
    QIcon icon;
    for (int d : {16, 20, 24, 32, 48, 64})
        icon.addPixmap(base.scaled(d, d, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    return icon;
}

}
//--------------buttonSettingsKey-----------//
QString Widget::buttonSettingsKey(int index) const
{
    return QString("button%1").arg(index + 1);
}
//------------------------------------------//
// this is a switch data table that references each element from the ui to a unique "case" number ie.case 0 referes to button 1, added + 1 cause I am lazy
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
    case 8: return ui->button9;
    default: return nullptr;
    }
}

//---------selectFileForButtons------------//

void Widget::selectFileForIndex(int index)
{


    QPushButton *btn = buttonForIndex(index);
    if (!btn) return;

    QString appDir = QCoreApplication::applicationDirPath();

    // Open dialog starting at the app directory
    QString filePath = QFileDialog::getOpenFileName(this, "Select File", appDir);

    if (filePath.isEmpty()) return;

    // Convert the absolute path to a relative path
    // Example: "C:/MyApp/tools/calc.exe" becomes "tools/calc.exe"
    QDir dir(appDir);
    QString relativePath = dir.relativeFilePath(filePath);

    const QFileInfo info(filePath);
    btn->setText(info.completeBaseName());

    m_settings->setValue(buttonSettingsKey(index), relativePath);

}
//-------------------------------------//
void Widget::editScriptLine(const QString &filePath)
{

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::warning(this, "Error", "Cannot open file");
        return;
    }

    QStringList lines;
    QTextStream in(&file);
    while (!in.atEnd())
        lines << in.readLine();

    file.close();

    int targetIndex = -1;

    for (int i = 0; i < lines.size(); ++i)
    {
        if (lines[i].contains("pwdRSYST-BCODE")){
            targetIndex = i;
            break;
        }
    }

    if (targetIndex == -1)
    {
        QMessageBox::warning(this, "Error", "Password line not found");
        return;
    }

    QString line = lines[targetIndex];

        QRegularExpression regex("\\.text\\s*=\\s*\"([^\"]*)\"");
        QRegularExpressionMatch match = regex.match(line);

    QString currentPassword = match.captured(1);

    bool ok;
    QString newPassword = QInputDialog::getText(
        this,
        "Edit Password",
        "Enter new password:",
        QLineEdit::Password,
        currentPassword,
        &ok
        );

    if (!ok || newPassword.isEmpty()) return;

    QString newLine = line;
    newLine.replace(regex, QString(".text = \"%1\"").arg(newPassword));

    lines[targetIndex] = newLine;

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::warning(this, "Error", "Cannot write file");
        return;
    }

    QTextStream out(&file);
    for (int i = 0; i < lines.size(); ++i)
        out << lines.at(i) << "\n";

    file.close();
}
//------------------------------------//
/*void Widget::openFileForIndex(int index)
{
    if (!m_settings) return;

    QString relativePath = m_settings->value(buttonSettingsKey(index), "").toString();

    if (!relativePath.isEmpty()) {
        QDir appDir(QCoreApplication::applicationDirPath());

        // Convert back to absolute path based on CURRENT location
        QString absolutePath = appDir.absoluteFilePath(relativePath);

        QFileInfo checkFile(absolutePath);
        if (checkFile.exists() && checkFile.isFile()) {
            QDesktopServices::openUrl(QUrl::fromLocalFile(absolutePath));
        }
    }
}*/
//------------------------------------------------//
void Widget::handleAlwaysOnTopToggled(bool checked)
{
    ui->AOT->setIcon(aotIconFromResources(checked));
    Qt::WindowFlags wf = windowFlags();
    wf.setFlag(Qt::WindowStaysOnTopHint, checked);
    setWindowFlags(wf);
    show();

}
//-----------------------------------------------//
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
//----------------------------------------------//
void Widget::on_clearButton_clicked()
{

    QString filePath = QCoreApplication::applicationDirPath() + "/weather_log.txt";
    QFile file(filePath);

    if (file.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        file.close();
    }


    logModel->removeRows(0, logModel->rowCount());

    // clears from memory
    m_loggedTimestamps.clear();

    // Instantly refresh logs with new coordinates
    recoverMissedLogs();
}
//---------------------------------------------//
void Widget::logWeather(double temp, double hum, const QString &timestamp)
{
    m_loggedTimestamps.insert(timestamp);
    QString logPath = QCoreApplication::applicationDirPath() + "/weather_log.txt";
    QFile file(logPath);
    ui->logTableView->scrollToBottom();

    double thi = 0.8 * temp + hum / 100 * (temp - 14.4) + 46.4;

    if (file.open(QIODevice::Append | QIODevice::Text))
    {

        QTextStream out(&file);
        out << timestamp
            << " | Temp: " << temp
            << "°C | Humidity: " << hum
            << " | THI: " << thi << "%\n";

        file.close();

    }
    int row = logModel->rowCount();
    logModel->insertRow(row);
    logModel->setData(logModel->index(row, 0), timestamp);
    logModel->setData(logModel->index(row, 1), temp);
    logModel->setData(logModel->index(row, 2), hum);
    logModel->setData(logModel->index(row, 3), thi);

    if (thi >= 70 && thi <= 80)
    {
        int col = 3;
        {
            logModel->setData(logModel->index(row, col),
                              QBrush(QColorConstants::Svg::yellow),
                              Qt::BackgroundRole);
        }
    }
    else if (thi >= 81 && thi <= 90)
    {
        int col = 3;
        {
            logModel->setData(logModel->index(row, col),
                              QBrush(QColorConstants::Svg::orange),
                              Qt::BackgroundRole);
        }
    }

    else if (thi > 91)
    {
        int col = 3;
        {
            logModel->setData(logModel->index(row, col),
                              QBrush(QColorConstants::Svg::red),
                              Qt::BackgroundRole);
        }
    }

    }
//-----------------------------------------------//
void Widget::recoverMissedLogs()
{

    QString lat = m_settings->value("latitude", "30.6").toString();
    QString lon = m_settings->value("longitude", "32.27").toString();

    QString url = QString("https://api.open-meteo.com/v1/forecast?"
                          "latitude=%1&longitude=%2&"
                          "hourly=temperature_2m,relativehumidity_2m&"
                          "timezone=auto&past_days=7")
                      .arg(lat, lon);

    QNetworkRequest request{QUrl(url)};

    QNetworkReply *reply = networkManager->get(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        handleRecoveryReply(reply);
    });
}
//--------------------------------------------------//
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

            if (!isAlreadyLogged(t))
            {
                logWeather(temp, hum, t);
            }
        }
    }
    reply->deleteLater();
}
//---------------------------------------//
bool Widget::isAlreadyLogged(const QString &timestamp)           // || update needed || This function will get slower because it has to read the whole file from the top every time.
{                                                                //Function inside logWeather can be used. Done
    return m_loggedTimestamps.contains(timestamp);               //( upated it so it reads the weather log fom ram instead of reading it from disk for every line ) no more memory overhead
}
//--------------------------------------//
void Widget::loadLogsFromFile()
{
    QString logPath = QCoreApplication::applicationDirPath() + "/weather_log.txt";
    QFile file(logPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QTextStream in(&file);
    while (!in.atEnd())
    {
        QString line = in.readLine();

        QStringList parts = line.split('|');
        if (parts.size() != 3) continue;

        int row = logModel->rowCount();
        logModel->insertRow(row);

        QString timestamp = parts[0].trimmed();

        m_loggedTimestamps.insert(timestamp);

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

        double thi = 0.8 * temp + hum / 100 * (temp - 14.4) + 46.4;

        logModel->setData(logModel->index(row, 3), thi);

        if (thi >= 70 && thi <= 80)
        {
            int col = 3;
            {
                logModel->setData(logModel->index(row, col),
                                  QBrush(QColorConstants::Svg::yellow),
                                  Qt::BackgroundRole);
            }
        }
        else if (thi >= 81 && thi <= 90)
        {
            int col = 3;
            {
                logModel->setData(logModel->index(row, col),
                                  QBrush(QColorConstants::Svg::orange),
                                  Qt::BackgroundRole);
            }
        }
        else if (thi > 91)
        {
            int col = 3;
            {
                logModel->setData(logModel->index(row, col),
                                  QBrush(QColorConstants::Svg::red),
                                  Qt::BackgroundRole);
            }
        }

    }

    file.close();
}
//---------------------------------//
void Widget::on_copyButton_clicked()
{
    QString today = QDate::currentDate().toString("yyyy-MM-dd");

    QStringList targetTimes = {
        "08:00",
        "12:00",
        "16:00"
    };

    QString result;

    for (int row = 0; row < logModel->rowCount(); ++row)
    {
        QString timestamp = logModel->data(logModel->index(row, 0)).toString();

        if (!timestamp.startsWith(today))
            continue;

        QString timePart = timestamp.mid(11, 5); // pulls the time from the index "start from char 11 and pull 5 chars"

        if (targetTimes.contains(timePart))
        {
            double temp = logModel->data(logModel->index(row, 1)).toDouble();
            double hum  = logModel->data(logModel->index(row, 2)).toDouble()/ 100.0;

            result += QString("%1\t %2\n")
                          .arg(temp)
                          .arg(hum);
        }
    }

    if (result.isEmpty())
    {
        result = "No data available for today.";
    }

    // Copy to clipboard
    QGuiApplication::clipboard()->setText(result);

}
//---------------------------//
void Widget::on_settingsButton_clicked()
{
    QMenu *menu = new QMenu(this);

    QWidget *container = new QWidget(menu);
    QVBoxLayout *layout = new QVBoxLayout(container);

    // Latitude Setup

    layout->addWidget(new QLabel("Latitude:", container));
    m_latEdit = new QLineEdit(m_settings->value("latitude", "30.6").toString(), container);
    layout->addWidget(m_latEdit);

    // Longitude Setup

    layout->addWidget(new QLabel("Longitude:", container));
    m_lonEdit = new QLineEdit(m_settings->value("longitude", "32.27").toString(), container);
    layout->addWidget(m_lonEdit);

    QWidgetAction *action = new QWidgetAction(menu);
    action->setDefaultWidget(container);
    menu->addAction(action);

    // This fires whenever the menu starts to close (user clicks away or presses Esc)

    connect(menu, &QMenu::aboutToHide, this, &Widget::saveSettingsFromMenu);

    //dropdown always at the bottom
    menu->exec(ui->settingsButton->mapToGlobal(QPoint(0, ui->settingsButton->height())));

    menu->deleteLater();
}
//-----------------------------//
void Widget::toggleAutoStart(bool enable)
{
    QSettings bootSettings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
    QString appPath = QDir::toNativeSeparators(QCoreApplication::applicationFilePath());
    QString value = QString("\"%1\"").arg(appPath);

    if (enable) {
        bootSettings.setValue("SAPAssistant", value);

    } else {
        bootSettings.remove("SAPAssistant");
    }
}
//---------------------------//
void Widget::saveSettingsFromMenu()
{
    if (!m_latEdit || !m_lonEdit) return;

    QString newLat = m_latEdit->text();
    QString newLon = m_lonEdit->text();

    if (newLat != m_settings->value("latitude").toString() ||
        newLon != m_settings->value("longitude").toString())
    {
        m_settings->setValue("latitude", newLat);
        m_settings->setValue("longitude", newLon);

        logModel->removeRows(0, logModel->rowCount());


        m_loggedTimestamps.clear();


        recoverMissedLogs();


        QString filePath = QCoreApplication::applicationDirPath() + "/weather_log.txt";
        QFile file(filePath);

        if (file.open(QIODevice::WriteOnly | QIODevice::Truncate))
        {
            file.close();
        }
    }

    m_latEdit = nullptr;
    m_lonEdit = nullptr;
}
//----------------------------//
void Widget::handleConnection()
{
    QTcpSocket *socket = server->nextPendingConnection();

    if(!serverEnabled){
        socket->disconnectFromHost();
        socket->deleteLater();
        return;
    }

    connect(socket, &QTcpSocket::readyRead, [this, socket]()
    {
        QByteArray request = socket->readAll();
        QString req = QString(request);

        // Serve web page
        if (req.contains("GET / "))
        {

            QString tableRows;


            for (int row = 0; row < logModel->rowCount(); ++row)
            {

                QString time = logModel->data(logModel->index(row, 0)).toString();
                if (!time.startsWith(QDate::currentDate().toString("yyyy-MM-dd")))
                    continue;
                QString temp = logModel->data(logModel->index(row, 1)).toString();
                QString hum  = logModel->data(logModel->index(row, 2)).toString();

                tableRows += QString(
                                 "<tr>"
                                 "<td>%1</td>"
                                 "<td>%2</td>"
                                 "<td>%3</td>"
                                 "</tr>"
                                 ).arg(time, temp, hum);
            }

            QString html = QString(R"HTML(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<style>
body { font-family: Arial; text-align: center; background: #1e1e1e; color: white; }

button {
    width: 80%;
    padding: 15px;
    margin: 5px;
    font-size: 16px;
    border-radius: 10px;
    border: none;
    background-color: #3498db;
    color: white;
}

table {
    width: 100%;
    margin-top: 20px;
    border-collapse: collapse;
}

td, th {
    border: 1px solid #555;
    padding: 8px;
}

th {
    background-color: #444;
}
</style>
</head>

<body>

<h2>SAP Assistant Remote</h2>

<button onclick="send('/button1')">Button 1</button>
<button onclick="send('/button2')">Button 2</button>
<button onclick="send('/button3')">Button 3</button>
<button onclick="send('/button4')">Button 4</button>
<button onclick="send('/button5')">Button 5</button>
<button onclick="send('/button6')">Button 6</button>
<button onclick="send('/button7')">Button 7</button>
<button onclick="send('/button8')">Button 8</button>
<button onclick="send('/clear')">Refresh Logs</button>

<h3>Weather Logs</h3>

<table>
<tr>
<th>Time</th>
<th>Temp</th>
<th>Humidity</th>
</tr>

%1

</table>

<script>
function send(path) {
    fetch(path).then(() => location.reload());
}
</script>

</body>
</html>
)HTML").arg(tableRows);

            QByteArray response =
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: text/html\r\n\r\n" +
                html.toUtf8();

            socket->write(response);
        }


       QRegularExpression re("GET /button(\\d+)");
        QRegularExpressionMatch match = re.match(req);

        if (match.hasMatch())
        {
            int index = match.captured(1).toInt() - 1; // button1 → index 0

            QPushButton *btn = buttonForIndex(index);

            if (btn)
            {
                btn->click();
            }
        }

        else if (req.contains("GET /clear"))
        {
            on_clearButton_clicked();
        }
        socket->disconnectFromHost();
    });
}
    QString getLocalIP()
{
    const QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();

    for (const QNetworkInterface &iface : interfaces)
    {
        if (iface.flags().testFlag(QNetworkInterface::IsUp) &&
            !iface.flags().testFlag(QNetworkInterface::IsLoopBack))
        {
            for (const QNetworkAddressEntry &entry : iface.addressEntries())
            {
                if (entry.ip().protocol() == QAbstractSocket::IPv4Protocol)
                {
                    return entry.ip().toString();
                }
            }
        }
    }

    return "127.0.0.1";

}
//----------------------------//
void Widget::on_button1_clicked()
{
    QString selectedVariant = ui->variantCombo_0->currentText();
    QString selectedVariant_2 = ui->variantCombo_6->currentText();

    QString relativePath = m_settings->value(buttonSettingsKey(0), "").toString();
    if (relativePath.isEmpty()) return;

    QDir appDir(QCoreApplication::applicationDirPath());
    QString scriptPath = appDir.absoluteFilePath(relativePath);

    QStringList arguments;
    arguments<< scriptPath;
    arguments<< selectedVariant;
    arguments<< selectedVariant_2;

    QProcess *process = new QProcess(this);
    process->start("wscript.exe", arguments);

    connect(process, &QProcess::finished, process, &QObject::deleteLater);
}
//-------------------------------------------------------------------------//
void Widget::on_button2_clicked()
{
    QString selectedVariant = ui->variantCombo_1->currentText();
    QString selectedVariant_2 = ui->variantCombo_7->currentText();

    QString relativePath = m_settings->value(buttonSettingsKey(1), "").toString();
    if (relativePath.isEmpty()) return;

    QDir appDir(QCoreApplication::applicationDirPath());
    QString scriptPath = appDir.absoluteFilePath(relativePath);

    QStringList arguments;
    arguments<< scriptPath;
    arguments<< selectedVariant;
    arguments<< selectedVariant_2;

    QProcess *process = new QProcess(this);
    process->start("wscript.exe", arguments);

    connect(process, &QProcess::finished, process, &QObject::deleteLater);
}
//---------------------------------------------------------------//
void Widget::on_button3_clicked()
{
    QString selectedVariant = ui->variantCombo_2->currentText();
    QString selectedVariant_2 = ui->variantCombo_8->currentText();

    QString relativePath = m_settings->value(buttonSettingsKey(2), "").toString();
    if (relativePath.isEmpty()) return;

    QDir appDir(QCoreApplication::applicationDirPath());
    QString scriptPath = appDir.absoluteFilePath(relativePath);

    QStringList arguments;
    arguments<< scriptPath;
    arguments<< selectedVariant;
    arguments<< selectedVariant_2;

    QProcess *process = new QProcess(this);
    process->start("wscript.exe", arguments);

    connect(process, &QProcess::finished, process, &QObject::deleteLater);
}
//----------------------------------------------//
void Widget::on_button4_clicked()
{
    QString selectedVariant = ui->variantCombo_3->currentText();
    QString selectedVariant_2 = ui->variantCombo_9->currentText();

    QString relativePath = m_settings->value(buttonSettingsKey(3), "").toString();
    if (relativePath.isEmpty()) return;

    QDir appDir(QCoreApplication::applicationDirPath());
    QString scriptPath = appDir.absoluteFilePath(relativePath);


    QStringList arguments;
    arguments<< scriptPath;
    arguments<< selectedVariant;
    arguments<< selectedVariant_2;

    QProcess *process = new QProcess(this);
    process->start("wscript.exe", arguments);

    connect(process, &QProcess::finished, process, &QObject::deleteLater);
}
//------------------------------------------------//
void Widget::on_button5_clicked()
{
    QString selectedVariant = ui->variantCombo_4->currentText();
    QString selectedVariant_2 = ui->variantCombo_10->currentText();

    QString relativePath = m_settings->value(buttonSettingsKey(4), "").toString();
    if (relativePath.isEmpty()) return;

    QDir appDir(QCoreApplication::applicationDirPath());
    QString scriptPath = appDir.absoluteFilePath(relativePath);

    QStringList arguments;
    arguments<< scriptPath;
    arguments<< selectedVariant;
    arguments<< selectedVariant_2;

    QProcess *process = new QProcess(this);
    process->start("wscript.exe", arguments);

    connect(process, &QProcess::finished, process, &QObject::deleteLater);
}
//-----------------------------------------------//
void Widget::on_button6_clicked()
{
    QString selectedVariant = ui->variantCombo_5->currentText();
    QString selectedVariant_2 = ui->variantCombo_11->currentText();

    QString relativePath = m_settings->value(buttonSettingsKey(5), "").toString();
    if (relativePath.isEmpty()) return;

    QDir appDir(QCoreApplication::applicationDirPath());
    QString scriptPath = appDir.absoluteFilePath(relativePath);

    QStringList arguments;
    arguments<< scriptPath;
    arguments<< selectedVariant;
    arguments<< selectedVariant_2;

    QProcess *process = new QProcess(this);
    process->start("wscript.exe", arguments);

    connect(process, &QProcess::finished, process, &QObject::deleteLater);
}
//-----------------------------------------------//
void Widget::on_button7_clicked()
{
    if (!m_settings) return;

    QString relativePath = m_settings->value(buttonSettingsKey(6), "").toString();

    if (!relativePath.isEmpty()) {
        QDir appDir(QCoreApplication::applicationDirPath());

        QString absolutePath = appDir.absoluteFilePath(relativePath);

        QFileInfo checkFile(absolutePath);
        if (checkFile.exists() && checkFile.isFile()) {
            QDesktopServices::openUrl(QUrl::fromLocalFile(absolutePath));
        }
    }
}
//----------------------------------------------//
void Widget::on_button8_clicked()
{
    if (!m_settings) return;

    QString relativePath = m_settings->value(buttonSettingsKey(7), "").toString();

    if (!relativePath.isEmpty()) {
        QDir appDir(QCoreApplication::applicationDirPath());

        QString absolutePath = appDir.absoluteFilePath(relativePath);

        QFileInfo checkFile(absolutePath);
        if (checkFile.exists() && checkFile.isFile()) {
            QDesktopServices::openUrl(QUrl::fromLocalFile(absolutePath));
        }
    }
}
//------------------------------------------------//
void Widget::on_button9_clicked()
{
    if (!m_settings) return;

    QString relativePath = m_settings->value(buttonSettingsKey(8), "").toString();

    if (!relativePath.isEmpty()) {
        QDir appDir(QCoreApplication::applicationDirPath());

        QString absolutePath = appDir.absoluteFilePath(relativePath);

        QFileInfo checkFile(absolutePath);
        if (checkFile.exists() && checkFile.isFile()) {
            QDesktopServices::openUrl(QUrl::fromLocalFile(absolutePath));
        }
    }
}
//---------------------------------------------//
void Widget::on_openSettingsBtn_clicked()
{
    SettingsDialog dlg(this, m_settings);
    dlg.exec();
}

//-------------------------//
void Widget::on_Exit_clicked()
{
    this->close();
}
//---------------------------//
void Widget::on_minimizeButton_clicked()
{
    this->hide();
}

//--------------------------------------------------------------CONSTRUCT--------------------------------------------------------------//

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);

    networkManager = new QNetworkAccessManager(this);

    QString settingsPath = QCoreApplication::applicationDirPath() + "/settings.ini";
    m_settings = new QSettings(settingsPath, QSettings::IniFormat, this);

    ui->AOT->setIcon(aotIconFromResources(ui->AOT->isChecked()));
    connect(ui->AOT, &QToolButton::toggled, this, &Widget::handleAlwaysOnTopToggled);

    ui->openSettingsBtn->setIcon(QIcon(":/icons/settings.png"));

    // -------------------------//


    logModel = new QStandardItemModel(this);
    logModel->setColumnCount(4);
    logModel->setHeaderData(0, Qt::Horizontal, "DateTime");
    logModel->setHeaderData(1, Qt::Horizontal, "Temperature (°C)");
    logModel->setHeaderData(2, Qt::Horizontal, "Humidity (%)");
    logModel->setHeaderData(3, Qt::Horizontal, "THI");


    //---------Attach model to table view--------//
    ui->logTableView->setModel(logModel);
    ui->logTableView->horizontalHeader()->setStretchLastSection(true);
    loadLogsFromFile();
    recoverMissedLogs();

    ui->logTableView->scrollToBottom();

    //------------------styleSheets----------------//

    ui->Exit->setStyleSheet(
        "QPushButton#Exit {"
        "   border: none;"
        "   background-color: transparent;"
        "   font-size: 16px;"
        "border-radius: 0px;"
        "}"
        "QPushButton#Exit:hover {"
        "  background-color: #c42b1c;"
        "  color: #ffffff;"
        "}"
        "QPushButton#Exit:pressed {"
        "  background-color: #9e2418;"
        "}"
    );

    ui->AOT->setStyleSheet(
        "QToolButton#AOT {"
        "   border: none;"
        "   background-color: transparent;"
        "   font-size: 16px;"
        "border-radius: 0px;"
        "}"
        "QToolButton#AOT:hover {"
        "   background-color: #444;"
        "}"
    );

    ui->minimizeButton->setStyleSheet(
        "QPushButton {"
        "   border: none;"
        "   background-color: transparent;"
        "   font-size: 16px;"
        "border-radius: 0px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #444;"
        "}"
    );

    ui->openSettingsBtn->setStyleSheet(
        "QPushButton {"
        "   border: none;"
        "   background-color: transparent;"
        "   font-size: 16px;"
        "border-radius: 0px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #444;"
        "}"
        );
    ui->frame->setStyleSheet(
        "QFrame {"
        " background-color: #def0ff;"
        "border-radius : 5px;"
        "}"
        );
    ui->frame_2->setStyleSheet(
        "QFrame {"
        "background-color: #def0ff;"
        "border-radius : 5px;"
        "}"
        );
    // this is a lambda function that connects the slots "selectfileforindex" and "openfileforindex" to the toolbuttons and the buttons
    // changed it to right click to change file : right click opens the pick file dialog now instead of the tool buttons and
    for (int i = 0; i < 8; ++i)
    {
        if (QPushButton *b = buttonForIndex(i))
        {
            b->setContextMenuPolicy(Qt::CustomContextMenu);
            //connect(b, &QPushButton::clicked, this, [this, i]() { openFileForIndex(i); });
            connect(b, &QPushButton::customContextMenuRequested, this, [this, i](const QPoint &pos) {
                selectFileForIndex(i);
            });
        }
    }
             connect(ui->button9, &QPushButton::clicked, this, [this]()
            {
            //openFileForIndex(8);
            });

    ui->button9->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(ui->button9, &QPushButton::customContextMenuRequested,
            this, [this](const QPoint &)
            {
        QString appDir = QCoreApplication::applicationDirPath();

        QString relativePath = m_settings->value(buttonSettingsKey(8)).toString();

        QString fullPath = QDir(appDir).filePath(relativePath);

        editScriptLine(fullPath);
            });
    // Load button labels from settings (show base name only).
    for (int i = 0; i < 8; ++i)
    {
        if (QPushButton *b = buttonForIndex(i))
        {
            const QString filePath = m_settings->value(buttonSettingsKey(i), "").toString();
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
    QPoint pos = m_settings->value("windowPos", QPoint(-1, -1)).toPoint();

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
    {
        Qt::WindowFlags wf = windowFlags();
        wf.setFlag(Qt::FramelessWindowHint, true);
        wf.setFlag(Qt::WindowStaysOnTopHint, ui->AOT->isChecked());
        setWindowFlags(wf);
    }

    connect(ui->openSettingsBtn, &QPushButton::clicked, this, &Widget::on_openSettingsBtn_clicked);

    double savedOpacity = m_settings->value("opacity", 100).toDouble();
    this->setWindowOpacity(savedOpacity);

    ui->dragWidget->installEventFilter(this);
    show();
//---------------hotKeys------------------//
    hotkeyFilter = new HotkeyFilter();

    hotkeyFilter->callback = [this](int id)
    {

        int index = id - 1; // Hotkey IDs start at 1

        QPushButton *btn = buttonForIndex(index);

        if (btn)
        {
            btn->click();
        }
    };
    qApp->installNativeEventFilter(hotkeyFilter);


    for (int i = 0; i < 8; ++i)
    {
        RegisterHotKey(NULL, i + 1, MOD_CONTROL | MOD_ALT, '1' + i);
    }
        RegisterHotKey(NULL, 9, MOD_CONTROL | MOD_ALT, '0');

    //------------------------------------------------//
    server = new QTcpServer(this);

    connect(server, &QTcpServer::newConnection,
            this, &Widget::handleConnection);

    server->listen(QHostAddress::Any, 8080);

    QString ip = getLocalIP();
    QString url = QString("http://%1:8080").arg(ip);

    ui->serverLabel->setText("Server: " + url);

    connect(ui->connectionButton,&QCheckBox::toggled,this, [this](bool checked)
            {
             serverEnabled = checked;

             m_settings->setValue("serverEnabled",checked);
             });
            bool enabled = m_settings->value("serverEnabled", false).toBool();
            ui->connectionButton->setChecked(enabled);
            serverEnabled = enabled;
    //-----------------------------------------//

    trayIcon = new QSystemTrayIcon(this);

    trayIcon->setIcon(QIcon(":/icons/icon.ico"));

    trayMenu = new QMenu(this);

    QAction *restoreAction = new QAction("Restore", this);
    QAction *exitAction = new QAction("Exit", this);

    trayMenu->addAction(restoreAction);
    trayMenu->addAction(exitAction);
    trayIcon->setContextMenu(trayMenu);


    connect(restoreAction, &QAction::triggered, this, [this]()
            {
                this->show();
                this->raise();
                this->activateWindow();
            });

    connect(exitAction, &QAction::triggered, this, [this]()
            {
                qApp->quit();
            });

    connect(trayIcon, &QSystemTrayIcon::activated,
          this,[this](QSystemTrayIcon::ActivationReason reason)
            {
                if(reason == QSystemTrayIcon :: Trigger)
                {this->show();}
    });

    trayIcon->show();

    //-----------------layout change-----------------------//

    ui->variantCombo_0->addItems({"/HATEM", "/MOB", "/dryme", "/ME", "/MED", "/MEC"});

    ui->variantCombo_1->addItems({"/HATEM", "/MOB", "/dryme", "/ME", "/MED", "/MEC"});

    ui->variantCombo_2->addItems({"/HATEM", "/MOB", "/dryme", "/ME", "/MED", "/MEC"});

    ui->variantCombo_3->addItems({"/HATEM", "/MOB", "/dryme", "/ME", "/MED", "/MEC"});

    ui->variantCombo_4->addItems({"/HATEM", "/MOB", "/dryme", "/ME", "/MED", "/MEC"});

    ui->variantCombo_5->addItems({"/HATEM", "/MOB", "/dryme", "/ME", "/MED", "/MEC"});

    ui->variantCombo_6->addItems({"qlty", "qlt2", "qlt3"});

    ui->variantCombo_7->addItems({"qlty", "qlt2", "qlt3"});

    ui->variantCombo_8->addItems({"qlty", "qlt2", "qlt3"});

    ui->variantCombo_9->addItems({"qlty", "qlt2", "qlt3"});

    ui->variantCombo_10->addItems({"qlty", "qlt2", "qlt3"});

    ui->variantCombo_11->addItems({"qlty", "qlt2", "qlt3"});

    m_comboList = { ui->variantCombo_0, ui->variantCombo_1, ui->variantCombo_2, ui->variantCombo_3,
                ui->variantCombo_4, ui->variantCombo_5, ui->variantCombo_6,
                ui->variantCombo_7, ui->variantCombo_8, ui->variantCombo_9,
                ui->variantCombo_10, ui->variantCombo_11};

    for (int i = 0; i < m_comboList.size(); ++i) {
        QString key = QString("lastVariant_%1").arg(i);
        QString lastValue = m_settings->value(key, "/HATEM").toString();
        m_comboList[i]->setCurrentText(lastValue);
    }
    //---------------------------------------//

}


Widget::~Widget()
{

    for (int i = 0; i < 9; ++i)
    {
        UnregisterHotKey(NULL, i + 1);
    }

    if (hotkeyFilter)
    {
        qApp->removeNativeEventFilter(hotkeyFilter);
        delete hotkeyFilter;
        hotkeyFilter = nullptr;
    }
    m_settings->setValue("windowPos", this->pos());

    for (int i = 0; i < m_comboList.size(); ++i) {
        QString key = QString("lastVariant_%1").arg(i);
        QString currentValue = m_comboList[i]->currentText();

        m_settings->setValue(key, currentValue);
    }

    if (trayIcon)
        trayIcon->hide();

    if (server)
        server->close();

    delete ui;

}
// move all buttons in the weather page to settings menu **
// maybe make the window resizable from the bottom so the table area can be increased.
// I can make profils to change scripts but what is the point, it already works
// Login credentials should be stored in an ecripted data base through the variant param method by right clicking the log in button
// make the damn THI gtaph
// I created this app when I was fueled by hate, I mean I still am but I am not crippled by it anymore.
// It's all gone I cant really feel a thing, I destroyed myself , I fucked up, please someone save me I cant save myself, I am so deep
// I just cant get out.
// as sad as that sounds this feelings is shaping me right now, do I like it ? no, but I cant really do much about it
// maybe I like it like that, maybe the damage is irreversible.
// what to do ???
// is it over ??
// no clue !