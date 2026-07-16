#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QCloseEvent>
#include <QNetworkAccessManager>
#include <QTimer>
#include <QStandardItemModel>
#include <QTableView>
#include <QPushButton>
#include <QToolButton>
#include <QAbstractNativeEventFilter>
#include <qcombobox.h>
#include <windows.h>
#include <QSet>
#include <QMenu>
#include <QWidgetAction>
#include <QLineEdit>
#include <QLabel>
#include <QVBoxLayout>
#include <QTcpServer>
#include <QTcpSocket>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QSettings>

class HotkeyFilter : public QAbstractNativeEventFilter
{
public:
    std::function<void(int)> callback;

    bool nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result) override
    {
        MSG* msg = static_cast<MSG*>(message);

        if (msg->message == WM_HOTKEY)
        {
            if (callback)
                callback((int)msg->wParam);
        }

        return false;
    }
};

class QNetworkReply;

QT_BEGIN_NAMESPACE
namespace Ui {
class Widget;
}
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget() override;
    void toggleAutoStart(bool enable);
     bool serverEnabled = true;

protected:   //  closeEvent is protected in QWidget
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    void handleAlwaysOnTopToggled(bool checked);
    void logWeather(double temp, double hum, const QString &timestamp);
    void handleRecoveryReply(QNetworkReply *reply);
    bool isAlreadyLogged(const QString &timestamp);
    void recoverMissedLogs();
    void loadLogsFromFile();
    void on_Exit_clicked();
    void on_copyButton_clicked();
    void on_clearButton_clicked();
    void on_minimizeButton_clicked();
    void on_settingsButton_clicked();
    void handleConnection();
    void editScriptLine(const QString &filePath);
    void on_button1_clicked();
    void on_button2_clicked();
    void on_button3_clicked();
    void on_button4_clicked();
    void on_button5_clicked();
    void on_button6_clicked();
    void on_button7_clicked();
    void on_button8_clicked();
    void on_button9_clicked();


private:
    void selectFileForIndex(int index);
   // void openFileForIndex(int index);
    QString buttonSettingsKey(int index) const;
    QPushButton *buttonForIndex(int index) const;
    HotkeyFilter *hotkeyFilter;
    QList<QComboBox*> m_comboList;
    Ui::Widget *ui;
    QPoint m_dragPosition; // store mouse position when dragging
    bool m_dragging = false; // flag for dragging
    ////////////////////
    QString lastLoggedTime;
    ///////////////////
    QNetworkAccessManager *networkManager;
    QTimer *timer;
    //////////////////
    QStandardItemModel *logModel;
    QSet<QString> m_loggedTimestamps;
    /////////////////
    QLineEdit *m_latEdit = nullptr;
    QLineEdit *m_lonEdit = nullptr;
    void saveSettingsFromMenu();
    ////////////////
    QTcpServer *server;
    QSystemTrayIcon *trayIcon;
    QMenu *trayMenu;
    QSettings *m_settings;
    void on_openSettingsBtn_clicked();
};
#endif // WIDGET_H
