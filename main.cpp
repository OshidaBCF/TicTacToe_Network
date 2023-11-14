#include "Zone.h"
#include <SFML/Window/Mouse.hpp>
#include <string>
#include <WS2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

using namespace std;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    string Host = "127.0.0.1"; // Server IP
    int Port = 5004; // Server Port

    WSAData data;
    WORD ver = MAKEWORD(2, 2);
    int wsResult = WSAStartup(ver, &data);

    if (wsResult != 0)
    {
        MessageBox(0, 0, L"Can't start winsocket, error #" + wsResult, 0);
    }

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET)
    {
        MessageBox(0, 0, L"Can't create socket, error #" + WSAGetLastError(), 0);
        WSACleanup();
        return 0;
    }

    sockaddr_in hint;
    hint.sin_family = AF_INET;
    hint.sin_port = htons(Port);
    inet_pton(AF_INET, Host.c_str(), &hint.sin_addr);

    int connResult = connect(sock, (sockaddr*)&hint, sizeof(hint));
    if (connResult == SOCKET_ERROR)
    {
        MessageBox(0, 0, L"Can't connect to server, error #" + WSAGetLastError(), 0);
        closesocket(sock);
        WSACleanup();
        return 0;
    }

    char buf[4096];
    string userInput;

    sf::RenderWindow window(sf::VideoMode(900, 900), "SFML works!");
    std::vector<zone> zones;
    int painter = zone::painterList::CIRCLE;

    for (int j = 0; j < 3; j++)
    {
        for (int i = 0; i < 3; i++)
        {
            zone newZone;
            newZone.painter = zone::painterList::NONE;
            newZone.coordinates = sf::Vector2f(i * 300, j * 300);
            zones.push_back(newZone);
        }
    }

    int winner = 0;

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        window.clear();
        for (int i = 0; i < 9; i++)
        {
            zones[i].Draw(&window);
        }

        sf::Vertex line[] =
        {
            sf::Vertex(sf::Vector2f(300, 0)),
            sf::Vertex(sf::Vector2f(300, 900))
        };
        window.draw(line, 2, sf::Lines);

        line[0] = sf::Vertex(sf::Vector2f(600, 0));
        line[1] = sf::Vertex(sf::Vector2f(600, 900));
        window.draw(line, 2, sf::Lines);

        line[0] = sf::Vertex(sf::Vector2f(0, 300));
        line[1] = sf::Vertex(sf::Vector2f(900, 300));
        window.draw(line, 2, sf::Lines);

        line[0] = sf::Vertex(sf::Vector2f(0, 600));
        line[1] = sf::Vertex(sf::Vector2f(900, 600));
        window.draw(line, 2, sf::Lines);
        window.display();

        if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
        {
            sf::Vector2i position = sf::Mouse::getPosition(window);
            if (position.x > 0 && position.x < window.getSize().x && position.y > 0 && position.y < window.getSize().y)
            {
                position /= 300;
                if (zones[position.x + position.y * 3].painter == 0)
                {
                    zones[position.x + position.y * 3].painter = painter;

                    // 0 1 2
                    // 3 4 5
                    // 6 7 8
                    // 0,1,2  3,4,5  6,7,8  0,3,6  1,4,7  2,5,8  0,4,8  2,4,6
                    if ((zones[0].painter != 0 && zones[0].painter == zones[1].painter && zones[0].painter == zones[2].painter) ||
                        (zones[3].painter != 0 && zones[3].painter == zones[4].painter && zones[3].painter == zones[5].painter) ||
                        (zones[6].painter != 0 && zones[6].painter == zones[7].painter && zones[6].painter == zones[8].painter) ||
                        (zones[0].painter != 0 && zones[0].painter == zones[3].painter && zones[0].painter == zones[6].painter) ||
                        (zones[1].painter != 0 && zones[1].painter == zones[4].painter && zones[1].painter == zones[7].painter) ||
                        (zones[2].painter != 0 && zones[2].painter == zones[5].painter && zones[2].painter == zones[8].painter) ||
                        (zones[0].painter != 0 && zones[0].painter == zones[4].painter && zones[0].painter == zones[8].painter) ||
                        (zones[2].painter != 0 && zones[2].painter == zones[4].painter && zones[2].painter == zones[6].painter))
                    {
                        winner = painter;
                    }

                    if (winner != 0)
                    {
                        if (winner == -1)
                        {
                            userInput = "W1";
                        }
                        else if (winner == 1)
                        {
                            userInput = "W2";
                        }
                    }
                    else
                    {
                        userInput = "P" + to_string(painter) + "X" + to_string(position.x) + "Y" + to_string(position.y);
                    }

                    int sendResult = send(sock, userInput.c_str(), userInput.size() + 1, 0);
                    if (sendResult != SOCKET_ERROR)
                    {
                        ZeroMemory(buf, 4096);
                        int BytesReceived = recv(sock, buf, 4096, 0);
                        if (BytesReceived)
                        {
                            // Do something with received bytes
                        }
                    }

                    painter *= -1;
                }
            }
        }
        if (winner != 0)
        {
            for (int i = 0; i < 9; i++)
            {
                zones[i].Draw(&window);
            }
            window.display();
            while (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {}
            while (!sf::Mouse::isButtonPressed(sf::Mouse::Left)) {}
            break;
        }
    }

    closesocket(sock);
    WSACleanup();
    return 0;
}