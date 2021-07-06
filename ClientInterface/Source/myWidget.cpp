#include "myWidget.h"
#include "ui_myWidget.h"

char * toCharP(QString in)
{
    QByteArray a; a.append(in);
    return a.data();
}

namespace Core {
myWidget::myWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::myWidget),
    com(new Communication(this))
{
    ui->setupUi(this);
    ui->Chat->setReadOnly(true);
    this->setWindowTitle("ClientApp");
}

myWidget::~myWidget()
{
    delete ui;
    delete com;
}

void myWidget::on_pushButton_Send_clicked()
{
    QString text = ui->Nickname->text() +  ": "  + ui->Message->text() + "\n";
    ui->Message->clear();
    msg = toCharP(text);
    if(msg != NULL)
    {
        com->send_msg(msg, sock);
        msg[0] = '\0';
    }
}

void myWidget::on_pushButton_Connect_clicked()
{
    QString Nickname = ui->Nickname->text();
    if(Nickname != NULL)
    {
       username = toCharP(Nickname);
       sock = com->conection(username);
       this->ui->Nickname->setReadOnly(true);
    }
    else ui->Chat->append("Insert a nickaname.");
}
void myWidget::on_pushButton_SendFile_clicked()
{
    QString text = ui->Message->text();
    ui->Message->clear();
    QString Nickname = this->ui->Nickname->text();
    username = toCharP(Nickname);
    msg = toCharP(text);
    com->send_f(msg, sock, username);
    msg[0] = '\0';
}

void myWidget::on_pushButton_Download_clicked()
{
    QString text = ui->Message->text();
    ui->Message->clear();
    QString Nickname = this->ui->Nickname->text();
    username = toCharP(Nickname);
    msg = toCharP(text);
    com->download_f(msg, sock, username);
    msg[0] = '\0';
}
} // end namespace
