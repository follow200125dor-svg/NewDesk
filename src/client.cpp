#include "client.h"
#include <QBuffer>

Client::Client(QObject *parent) : QObject(parent) {
    m_socket = new QTcpSocket(this);
    connect(m_socket, &QTcpSocket::readyRead, this, &Client::onReadyRead);
    
    // Тот же ключ что и на сервере (в реальном проекте — обмен ключами)
    memset(m_key, 0x42, crypto_secretbox_KEYBYTES);
}

void Client::connectToServer(const QString &host, int port) {
    m_socket->connectToHost(host, port);
    qDebug() << "Connecting to" << host << ":" << port;
}

void Client::setDisplayLabel(QLabel *label) {
    m_label = label;
}

void Client::onReadyRead() {
    m_buffer.append(m_socket->readAll());
    
    while (m_buffer.size() >= 4) {
        if (m_expectedSize == 0) {
            memcpy(&m_expectedSize, m_buffer.data(), 4);
            m_buffer.remove(0, 4);
        }
        
        if (m_buffer.size() >= m_expectedSize) {
            QByteArray packetData = m_buffer.left(m_expectedSize);
            m_buffer.remove(0, m_expectedSize);
            m_expectedSize = 0;
            
            // Десериализация
            rdp::Packet packet;
            packet.ParseFromArray(packetData.data(), packetData.size());
            
            if (packet.type() == rdp::Packet::FRAME) {
                QByteArray encrypted(packet.encrypted_data().data(), packet.encrypted_data().size());
                QByteArray decrypted = decrypt(encrypted);
                QByteArray decompressed = decompress(decrypted);
                
                rdp::Frame frame;
                frame.ParseFromArray(decompressed.data(), decompressed.size());
                
                QImage image((const uchar*)frame.data().data(),
                             frame.width(), frame.height(),
                             QImage::Format_RGB888);
                
                if (m_label) {
                    m_label->setPixmap(QPixmap::fromImage(image).scaled(
                        m_label->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
                }
            }
        } else {
            break;
        }
    }
}

QByteArray Client::decrypt(const QByteArray &data) {
    if (data.size() < crypto_secretbox_NONCEBYTES + crypto_secretbox_MACBYTES)
        return {};
    
    const unsigned char *nonce = (const unsigned char*)data.data();
    const unsigned char *encrypted = nonce + crypto_secretbox_NONCEBYTES;
    int encryptedSize = data.size() - crypto_secretbox_NONCEBYTES;
    
    QByteArray decrypted(encryptedSize - crypto_secretbox_MACBYTES, Qt::Uninitialized);
    
    if (crypto_secretbox_open_easy((unsigned char*)decrypted.data(),
                                   encrypted, encryptedSize,
                                   nonce, m_key) != 0) {
        qWarning() << "Decryption failed!";
        return {};
    }
    
    return decrypted;
}

QByteArray Client::decompress(const QByteArray &data) {
    uLongf decompressedSize = data.size() * 10;
    QByteArray decompressed;
    
    while (true) {
        decompressed.resize(decompressedSize);
        int result = uncompress((Bytef*)decompressed.data(), &decompressedSize,
                                (const Bytef*)data.data(), data.size());
        
        if (result == Z_OK) {
            decompressed.resize(decompressedSize);
            return decompressed;
        } else if (result == Z_BUF_ERROR) {
            decompressedSize *= 2;
        } else {
            qWarning() << "Decompression failed!";
            return {};
        }
    }
}
