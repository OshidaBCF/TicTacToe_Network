#include "Zone.h"
#include <SFML/Window/Mouse.hpp>
#include <string>
#include <iostream>
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
        MessageBox(NULL, (L"Can't start winsocket, error #" + to_wstring(wsResult)).c_str(), 0, MB_ICONWARNING);
    }

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET)
    {
        MessageBox(NULL, (L"Can't create socket, error #" + to_wstring( WSAGetLastError())).c_str(), 0, MB_ICONWARNING);
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
        MessageBox(NULL, (L"Can't connect to server, error #" + to_wstring(WSAGetLastError())).c_str(), 0, MB_ICONWARNING);
        closesocket(sock);
        WSACleanup();
        return 0;
    }

    char buf[4096];
    string userInput;

    sf::RenderWindow window(sf::VideoMode(1800, 900), "TicTacToe online!");
    std::vector<zone> zones;
    int painter = zone::painterList::CIRCLE;

    for (int j = 0; j < 3; j++)
    {
        for (int i = 0; i < 3; i++)
        {
            zone newZone(sf::Vector2f(i * 250, j * 250));
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
        sf::Font font;
  
        if (font.loadFromFile("Roboto-Black.ttf"))
        {
            return EXIT_FAILURE;
        }
       

        sf::Text Player1;
        Player1.setFont(font);
        Player1.setString("Player 1");
        Player1.setCharacterSize(15);
        Player1.setFillColor(sf::Color::Blue);
        Player1.setStyle(sf::Text::Bold | sf::Text::Underlined);
        Player1.setPosition(900, 100);
        window.draw(Player1);

        sf::Text Player2;
        Player2.setFont(font);
        Player2.setString("Player 2");
        Player2.setCharacterSize(15);
        Player2.setFillColor(sf::Color::Red);
        Player1.setStyle(sf::Text::Bold | sf::Text::Underlined);
        Player1.setPosition(900, 100);
        window.draw(Player2);

        sf::Vertex line[] =
        {
            // vecteur lignes
            sf::Vertex(sf::Vector2f(300, 0)),
            sf::Vertex(sf::Vector2f(300, 900)),
        };
        window.draw(line, 2, sf::Lines);

        line[0] = sf::Vertex(sf::Vector2f(600, 0)); // ligne vertical
        line[1] = sf::Vertex(sf::Vector2f(600, 900));
        window.draw(line, 2, sf::Lines);

        line[0] = sf::Vertex(sf::Vector2f(0, 300)); // ligne horizontal
        line[1] = sf::Vertex(sf::Vector2f(900, 300));
        window.draw(line, 2, sf::Lines);

        // Dessiner la troisième colonne verticale
        line[0] = sf::Vertex(sf::Vector2f(900, 0));
        line[1] = sf::Vertex(sf::Vector2f(900, 900));
        window.draw(line, 2, sf::Lines);

        // ligne fermant la colonne.
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
                    userInput = "P" + to_string(painter) + "X" + to_string(position.x) + "Y" + to_string(position.y);

                    int sendResult = send(sock, userInput.c_str(), userInput.size() + 1, 0);
                    if (sendResult != SOCKET_ERROR)
                    {
                        ZeroMemory(buf, 4096);
                        int BytesReceived = recv(sock, buf, 4096, 0);
                        if (BytesReceived)
                        {
                            if (buf[0] == 'P')
                            {
                                if (buf[1] == '-')
                                {
                                    painter = -1;
                                    zones[int(buf[4]) - '0' + (int(buf[6]) - '0') * 3].painter = painter;
                                }
                                else
                                {
                                    painter = 1;
                                    zones[int(buf[3]) - '0' + (int(buf[5]) - '0') * 3].painter = painter;
                                }
                            }
                            else if (buf[0] == 'W')
                            {
                                if (buf[1] == '1')
                                {
                                    winner = -1;
                                }
                                else
                                {
                                    winner = 1;
                                }
                            }
                        }
                    }
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
            while (!sf::Mouse::isButtonPressed(sf::Mouse::Left)) {}
            break;
        }
    }

    closesocket(sock);
    WSACleanup();
    return 0;
}
