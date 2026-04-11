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
#include <windows.h>

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

protected:   //  closeEvent is protected in QWidget
    void closeEvent(QCloseEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    void handleAlwaysOnTopToggled(bool checked);
    void logWeather(double temp, double hum, const QString &timestamp);
    void handleRecoveryReply(QNetworkReply *reply);
    bool isAlreadyLogged(const QString &timestamp);
    void recoverMissedLogs();
    void on_saveLocationButton_clicked();
    void loadLogsFromFile();
    void on_Exit_clicked();
    void on_copyButton_clicked();
    void on_clearButton_clicked();
    void on_minimizeButton_clicked();

private:
    void selectFileForIndex(int index);
    void openFileForIndex(int index);
    QString buttonSettingsKey(int index) const;
    QPushButton *buttonForIndex(int index) const;
    HotkeyFilter *hotkeyFilter;

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
};
#endif // WIDGET_H
