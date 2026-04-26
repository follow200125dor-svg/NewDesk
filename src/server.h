#pragma once
#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QScreen>
#include <QTimer>
#include <QPixmap>
#include <sodium.h>
#include <zlib.h>
#include "remote.pb.h"

class Server : public QObject {
    Q_OBJECT
public:
    explicit Server(QObject *parent = nullptr);
    ~Server();
    void start(int port = 12345);

private slots:
    void onNewConnection();
    void captureAndSend();

private:
    QByteArray encrypt(const QByteArray &data);
    QByteArray compress(const QByteArray &data);

    QTcpServer *m_server;
    QTcpSocket *m_client = nullptr;
    QTimer *m_timer;
    QScreen *m_screen;
    unsigned char m_key[crypto_secretbox_KEYBYTES];
};
