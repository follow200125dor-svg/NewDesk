#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include "server.h"
#include "client.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    if (sodium_init() < 0) {
        qFatal("libsodium init failed!");
    }
    
    QMainWindow window;
    window.setWindowTitle("RemoteDesk");
    window.resize(800, 600);
    
    QWidget *central = new QWidget(&window);
    QVBoxLayout *layout = new QVBoxLayout(central);
    
    QLabel *screen = new QLabel("Waiting for connection...");
    screen->setAlignment(Qt::AlignCenter);
    screen->setMinimumSize(640, 480);
    layout->addWidget(screen);
    
    QPushButton *startServer = new QPushButton("Start Server");
    QPushButton *startClient = new QPushButton("Connect to Server");
    layout->addWidget(startServer);
    layout->addWidget(startClient);
    
    window.setCentralWidget(central);
    
    // Сервер и клиент создаются по кнопкам
    Server *server = nullptr;
    Client *client = nullptr;
    
    QObject::connect(startServer, &QPushButton::clicked, [&]() {
        if (!server) {
            server = new Server(&window);
            server->start(12345);
            startServer->setText("Server Running on :12345");
            startServer->setEnabled(false);
        }
    });
    
    QObject::connect(startClient, &QPushButton::clicked, [&]() {
        if (!client) {
            client = new Client(&window);
            client->setDisplayLabel(screen);
            client->connectToServer("127.0.0.1", 12345);
            startClient->setText("Connecting...");
            startClient->setEnabled(false);
        }
    });
    
    window.show();
    return app.exec();
}
