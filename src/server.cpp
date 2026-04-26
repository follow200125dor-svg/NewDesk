#include "server.h"
#include <QApplication>

Server::Server(QObject *parent) : QObject(parent) {
    m_server = new QTcpServer(this);
    m_timer = new QTimer(this);
    m_screen = QApplication::primaryScreen();
    
    // Генерируем ключ шифрования
    crypto_secretbox_keygen(m_key);
    
    connect(m_server, &QTcpServer::newConnection, this, &Server::onNewConnection);
    connect(m_timer, &QTimer::timeout, this, &Server::captureAndSend);
}

Server::~Server() {
    m_timer->stop();
    m_server->close();
}

void Server::start(int port) {
    m_server->listen(QHostAddress::Any, port);
    qDebug() << "Server listening on port" << port;
}

void Server::onNewConnection() {
    m_client = m_server->nextPendingConnection();
    qDebug() << "Client connected!";
    m_timer->start(50);  // 20 FPS
}

void Server::captureAndSend() {
    if (!m_client || !m_screen) return;
    
    // 1. Захват экрана
    QPixmap pixmap = m_screen->grabWindow(0);
    QImage image = pixmap.toImage().convertToFormat(QImage::Format_RGB888);
    
    // 2. Упаковка в protobuf
    rdp::Frame frame;
    frame.set_width(image.width());
    frame.set_height(image.height());
    frame.set_data(image.constBits(), image.sizeInBytes());
    frame.set_timestamp(QDateTime::currentMSecsSinceEpoch());
    
    QByteArray frameData(frame.ByteSizeLong(), Qt::Uninitialized);
    frame.SerializeToArray(frameData.data(), frameData.size());
    
    // 3. Сжатие
    QByteArray compressed = compress(frameData);
    
    // 4. Шифрование
    QByteArray encrypted = encrypt(compressed);
    
    // 5. Отправка
    rdp::Packet packet;
    packet.set_type(rdp::Packet::FRAME);
    packet.set_encrypted_data(encrypted.data(), encrypted.size());
    
    QByteArray packetData(packet.ByteSizeLong(), Qt::Uninitialized);
    packet.SerializeToArray(packetData.data(), packetData.size());
    
    // Сначала отправляем размер пакета (4 байта)
    int32_t size = packetData.size();
    m_client->write((const char*)&size, sizeof(size));
    m_client->write(packetData);
    m_client->flush();
}

QByteArray Server::compress(const QByteArray &data) {
    uLongf compressedSize = compressBound(data.size());
    QByteArray compressed(compressedSize, Qt::Uninitialized);
    
    compress2((Bytef*)compressed.data(), &compressedSize,
              (const Bytef*)data.data(), data.size(), Z_BEST_SPEED);
    
    compressed.resize(compressedSize);
    return compressed;
}

QByteArray Server::encrypt(const QByteArray &data) {
    QByteArray encrypted(data.size() + crypto_secretbox_MACBYTES, Qt::Uninitialized);
    unsigned char nonce[crypto_secretbox_NONCEBYTES];
    randombytes_buf(nonce, sizeof nonce);
    
    crypto_secretbox_easy((unsigned char*)encrypted.data(),
                          (const unsigned char*)data.data(), data.size(),
                          nonce, m_key);
    
    // Добавляем nonce в начало
    return QByteArray((const char*)nonce, sizeof nonce) + encrypted;
}
