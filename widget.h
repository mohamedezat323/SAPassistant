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

protected:   // ✅ closeEvent is protected in QWidget
    void closeEvent(QCloseEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:   // 👈 ADD THIS SECTION
    void on_AOT_toggled(bool checked);
    void logWeather(double temp, double hum, const QString &timestamp);
    void handleRecoveryReply(QNetworkReply *reply);
    bool isAlreadyLogged(const QString &timestamp);
    void recoverMissedLogs();
    void on_saveLocationButton_clicked();
    void loadLogsFromFile();

private:
    void selectFileForIndex(int index);
    void openFileForIndex(int index);
    QString buttonSettingsKey(int index) const;
    QPushButton *buttonForIndex(int index) const;

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
