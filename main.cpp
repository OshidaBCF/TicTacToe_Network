#include "Zone.h"
#include <SFML/Window/Mouse.hpp>
#include <string>
#include <iostream>
#include <vector>
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WS2tcpip.h>
#include <processthreadsapi.h>
#pragma comment(lib, "ws2_32.lib")

using namespace std;

#define WEB_SOCKET_EVENT (WM_USER + 1)

enum textStatusList
{
    NONE = 0,
    DRAW,
    P1TURN,
    P2TURN,
    P1WIN,
    P2WIN
};

SOCKET sock;
int winner = 0;
char buf[4096];
string userInput;
std::vector<zone> zones;
int painter = 0;
sf::RenderWindow window(sf::VideoMode(1800, 900), "TicTacToe online!");
textStatusList textStatus = textStatusList::P1TURN;

string pseudo1 = "";
string pseudo2 = "";

bool isPseudoEntered = false;

void readNotification();

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    SOCKET Accept = 0;
    switch (uMsg) {
    case WM_CLOSE:
        // Gérer l'événement de fermeture de la fenêtre
        MessageBox(NULL, L"Fermeture de la fenêtre cachée.", L"Événement", MB_ICONINFORMATION);
        DestroyWindow(hwnd);
        break;
    case WM_DESTROY:
        // Gérer la destruction de la fenêtre
        PostQuitMessage(0);
        break;
    
    case WEB_SOCKET_EVENT:
    {
        switch (WSAGETSELECTEVENT(lParam))
        {
        case FD_READ:
        {
            readNotification();
        }
        break;
        case FD_CLOSE:
            closesocket((SOCKET)wParam);
            break;
        }
    }
    break;
    default:
        // Laisser les autres messages être gérés par la procédure par défaut
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return 0;
}

void readNotification()
{
    ZeroMemory(buf, 4096);
    int BytesReceived = recv(sock, buf, 4096, 0);
    if (BytesReceived)
    {
        if (buf[0] == 'Q')
        {
            painter = int(buf[1]) - '0';
            if (painter == zone::painterList::NONE)
            {
                isPseudoEntered = true;
            }

            for (int j = 0; j < 3; j++)
            {
                for (int i = 0; i < 3; i++)
                {
                    zones[i + j * 3].painter = int(buf[3 + (i + j * 3)]) - '0';
                }
            }
            for (int i = 0; i < 9; i++)
            {
                zones[i].Draw(&window);
            }
            window.display();
        }
        if (buf[0] == 'M')
        {
            for (int i = 2; i < BytesReceived; i++)
            {
                if (int(buf[1]) - '0' == zone::painterList::CIRCLE)
                    pseudo1 += buf[i];
                else
                    pseudo2 += buf[i];
            }
        }
        if (buf[0] == 'S')
        {
            for (int j = 0; j < 3; j++)
            {
                for (int i = 0; i < 3; i++)
                {
                    zones[i + j * 3].painter = int(buf[1 + (i + j * 3)]) - '0';
                }
            }
            for (int i = 0; i < 9; i++)
            {
                zones[i].Draw(&window);
            }
            window.display();
            if (textStatus == textStatusList::P2TURN)
                textStatus = textStatusList::P1TURN;
            else
                textStatus = textStatusList::P2TURN;
        }
        if (buf[10] == 'W')
        {
            if (int(buf[11]) - '0' == zone::painterList::CIRCLE)
            {
                winner = zone::painterList::CIRCLE;
                textStatus = textStatusList::P1WIN;
            }
            else
            {
                winner = zone::painterList::CROSS;
                textStatus = textStatusList::P2WIN;
            }
        }
    }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    // Creation de la window class
    HINSTANCE hiddenHInstance = GetModuleHandle(NULL);

    // Définir la classe de la fenêtre
    WNDCLASS windowClass = {};
    windowClass.lpfnWndProc = WindowProc;
    windowClass.hInstance = hiddenHInstance;
    windowClass.lpszClassName = L"MyHiddenWindowClass";

    // Enregistrer la classe de fenêtre
    RegisterClass(&windowClass);

    // Créer la fenêtre cachée
    HWND hiddenWindow = CreateWindowEx(
        0,                              // Styles étendus
        L"MyHiddenWindowClass",        // Nom de la classe
        L"MyHiddenWindow",              // Titre de la fenêtre
        WS_OVERLAPPEDWINDOW,// Style de la fenêtre (fenêtre cachée)
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        NULL, NULL, hiddenHInstance, NULL);

    // Vérifier si la fenêtre a été créée avec succès
    if (hiddenWindow == NULL) {
        MessageBox(NULL, L"Erreur lors de la création de la fenêtre cachée.", L"Erreur", MB_ICONERROR);
        return 1;
    }

    // Afficher la fenêtre cachée (si nécessaire)
    // ShowWindow(hiddenWindow, SW_SHOWNORMAL);

    // cout << "Server Main thread running...\n";

    //string Host = "10.1.170.34"; // Server IP
    string Host = "127.0.0.1";
    int Port = 5004; // Server Port

    WSAData data;
    WORD ver = MAKEWORD(2, 2);
    int wsResult = WSAStartup(ver, &data);

    if (wsResult != 0)
    {
        MessageBox(NULL, (L"Can't start winsocket, error #" + to_wstring(wsResult)).c_str(), 0, MB_ICONWARNING);
    }

    sock = socket(AF_INET, SOCK_STREAM, 0);
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

    WSAAsyncSelect(sock, hiddenWindow, WEB_SOCKET_EVENT, FD_READ | FD_CLOSE);

    for (int j = 0; j < 3; j++)
    {
        for (int i = 0; i < 3; i++)
        {
            zone newZone(sf::Vector2f(i * 300, j * 300));
            zones.push_back(newZone);
        }
    }

    sf::Font font;
    if (!font.loadFromFile("Roboto-Black.ttf"))
    {
        return EXIT_FAILURE;
    }

    sf::Text askPseudo;
    askPseudo.setFont(font);
    askPseudo.setString("Choisissez votre pseudonyme.");
    askPseudo.setCharacterSize(60);
    askPseudo.setFillColor(sf::Color::White);
    askPseudo.setStyle(sf::Text::Bold);
    askPseudo.setPosition(window.getSize().x / 4, window.getSize().y /2 -100);

    sf::Text enterPseudo;
    enterPseudo.setFont(font);
    enterPseudo.setCharacterSize(60);
    enterPseudo.setFillColor(sf::Color::White);
    enterPseudo.setStyle(sf::Text::Bold);
    enterPseudo.setPosition(window.getSize().x / 4, window.getSize().y /2 + 100);

    sf::Text Player1;
    Player1.setFont(font);
    Player1.setCharacterSize(30);
    Player1.setFillColor(sf::Color::Blue);
    Player1.setStyle(sf::Text::Bold | sf::Text::Underlined);
    Player1.setPosition(1000, 100);

    sf::Text Player2;
    Player2.setFont(font);
    Player2.setCharacterSize(30);
    Player2.setFillColor(sf::Color::Red);
    Player2.setStyle(sf::Text::Bold | sf::Text::Underlined);
    Player2.setPosition(1500, 100);

    sf::Text Text;
    Text.setFont(font);
    Text.setCharacterSize(30);
    Text.setFillColor(sf::Color::White);
    Text.setStyle(sf::Text::Bold);
    Text.setPosition(1050, 500);

    //============================================

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }

            // Gestion de la saisie du clavier
            if (event.type == sf::Event::TextEntered && !isPseudoEntered)
            {
                if (event.text.unicode < 128) // Vérification si le caractère est valide
                {
                    if (event.text.unicode == '\b') // Vérification de la touche Backspace
                    {
                        if (!userInput.empty())
                        {
                            userInput.pop_back();
                            enterPseudo.setString(userInput);
                        }
                    }
                    else if (event.text.unicode == '\r') // Touche "Entrée" appuyée
                    {
                        isPseudoEntered = true; // Marquer que le pseudo est entré
                        string message = "M" + to_string(painter) + userInput;
                        send(sock, message.c_str(), message.size(), 0);
                    }
                    else if (userInput.size() < 20) // Limite de caractères pour le pseudonyme
                    {
                        userInput += static_cast<char>(event.text.unicode);
                        enterPseudo.setString(userInput);
                    }
                }
            }
        }

        window.clear();

        //============================
        
        if (!isPseudoEntered)
        {
            window.clear();
            window.draw(askPseudo);
            window.draw(enterPseudo);
            window.display();
            continue;
        }

        //============================

        for (int i = 0; i < 9; i++)
        {
            zones[i].Draw(&window);
        }
        Player1.setString(pseudo1);
        Player2.setString(pseudo2);

        // Turn line;
        switch (textStatus)
        {
        case textStatusList::NONE:
        {
            Text.setString("");
        }
        break;
        case textStatusList::DRAW:
        {
            Text.setString("Match Nul");
        }
        break;
        case textStatusList::P1TURN:
        {
            Text.setString("C'est aux tour des Cercles");
        }
        break;
        case textStatusList::P2TURN:
        {
            Text.setString("C'est aux tour des Croix");
        }
        break;
        case textStatusList::P1WIN:
        {
            Text.setString("Les Cercles gagne");
        }
        break;
        case textStatusList::P2WIN:
        {
            Text.setString("Les Croix gagne");
        }
        break;
        }
        window.draw(Player1);
        window.draw(Player2);
        window.draw(Text);

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

        // Dessiner la troisieme colonne verticale
        line[0] = sf::Vertex(sf::Vector2f(900, 0));
        line[1] = sf::Vertex(sf::Vector2f(900, 900));
        window.draw(line, 2, sf::Lines);

        // ligne fermant la colonne.
        line[0] = sf::Vertex(sf::Vector2f(0, 600));
        line[1] = sf::Vertex(sf::Vector2f(900, 600));
        window.draw(line, 2, sf::Lines);

        window.display();

        if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && winner == zone::painterList::NONE)
        {
            while (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {}
            sf::Vector2i position = sf::Mouse::getPosition(window);
            if (position.x > 0 && position.x < 900 && position.y > 0 && position.y < 900)
            {
                position /= 300;
                if (zones[position.x + position.y * 3].painter == zone::painterList::NONE)
                {
                    userInput = "P" + to_string(painter) + "X" + to_string(position.x) + "Y" + to_string(position.y);

                    int sendResult = send(sock, userInput.c_str(), userInput.size(), 0);
                    if (sendResult != SOCKET_ERROR)
                    {
                        ZeroMemory(buf, 4096);
                        int BytesReceived = recv(sock, buf, 4096, 0);
                        if (BytesReceived)
                        {
                            /*if (buf[0] != 'N')
                            {
                                if (painter == zone::painterList::CIRCLE)
                                    textStatus = textStatusList::P1TURN;
                                else
                                    textStatus = textStatusList::P2TURN;
                            }
                            else
                            {
                                if (painter == zone::painterList::CIRCLE)
                                    textStatus = textStatusList::P1TURN;
                                else
                                    textStatus = textStatusList::P2TURN;
                            }*/
                            if (buf[0] == 'S')
                            {
                                for (int j = 0; j < 3; j++)
                                {
                                    for (int i = 0; i < 3; i++)
                                    {
                                        zones[i + j * 3].painter = int(buf[1 + (i + j * 3)]) - '0';
                                    }
                                }
                                for (int i = 0; i < 9; i++)
                                {
                                    zones[i].Draw(&window);
                                }
                                window.display();
                            }
                            if (buf[0] == 'W')
                            {
                                if (int(buf[1]) - '0' == zone::painterList::CIRCLE)
                                {
                                    winner = zone::painterList::CIRCLE;
                                    textStatus = textStatusList::P1WIN;
                                }
                                else
                                {
                                    winner = zone::painterList::CROSS;
                                    textStatus = textStatusList::P2WIN;
                                }
                            }
                        }
                    }
                }
            }
        }
        /*if (winner != 0)
        {
            for (int i = 0; i < 9; i++)
            {
                zones[i].Draw(&window);
            }
            window.display();
            while (!sf::Mouse::isButtonPressed(sf::Mouse::Left)) {}
            break;
        }*/
    }

    closesocket(sock);
    WSACleanup();
    return 0;
}
