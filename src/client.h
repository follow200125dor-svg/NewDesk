#pragma once
#include <QObject>
#include <QTcpSocket>
#include <QLabel>
#include <sodium.h>
#include <zlib.h>
#include "remote.pb.h"

class Client : public QObject {
    Q_OBJECT
public:
    explicit Client(QObject *parent = nullptr);
    void connectToServer(const QString &host, int port = 12345);
    void setDisplayLabel(QLabel *label);

private slots:
    void onReadyRead();

private:
    QByteArray decrypt(const QByteArray &data);
    QByteArray decompress(const QByteArray &data);

    QTcpSocket *m_socket;
    QLabel *m_label;
    unsigned char m_key[crypto_secretbox_KEYBYTES];
    QByteArray m_buffer;
    int32_t m_expectedSize = 0;
};
