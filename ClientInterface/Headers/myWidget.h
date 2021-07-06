#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include"Communication.h"

namespace Ui {
class myWidget;
}

namespace Core {

class Communication;

class myWidget : public QWidget
{
    Q_OBJECT

public:
    explicit myWidget(QWidget *parent = 0);
    ~myWidget();

private slots:
    void on_pushButton_Send_clicked();

    void on_pushButton_Connect_clicked();

    void on_pushButton_SendFile_clicked();

    void on_pushButton_Download_clicked();

private:
    Ui::myWidget *ui;
    Communication*com;
    friend class Communication;
    char *username;
    char *msg;
    char *recv_msg;
    char **arguments;
    int sock;
};

} // end namespace
#endif // WIDGET_H
