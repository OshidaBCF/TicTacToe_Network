#include "Zone.h"
#include <SFML/Window/Mouse.hpp>
#include <string>
#include <vector>
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WS2tcpip.h>
#include <processthreadsapi.h>
#pragma comment(lib, "ws2_32.lib")

using namespace std;

#define WEB_SOCKET_EVENT (WM_USER + 1)

SOCKET sock;
int winner = 0;
char buf[4096];
string userInput;
std::vector<zone> zones;
int painter = 0;
sf::RenderWindow window(sf::VideoMode(900, 900), "Amongus");

void readNotification();

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    SOCKET Accept;
    switch (uMsg) {
    case WM_CLOSE:
        // G�rer l'�v�nement de fermeture de la fen�tre
        MessageBox(NULL, L"Fermeture de la fen�tre cach�e.", L"�v�nement", MB_ICONINFORMATION);
        DestroyWindow(hwnd);
        break;
    case WM_DESTROY:
        // G�rer la destruction de la fen�tre
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
        // Laisser les autres messages �tre g�r�s par la proc�dure par d�faut
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
            }
            else
            {
                winner = zone::painterList::CROSS;
            }
        }
    }
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    // Creation de la window class
    HINSTANCE hiddenHInstance = GetModuleHandle(NULL);

    // D�finir la classe de la fen�tre
    WNDCLASS windowClass = {};
    windowClass.lpfnWndProc = WindowProc;
    windowClass.hInstance = hiddenHInstance;
    windowClass.lpszClassName = L"MyHiddenWindowClass";

    // Enregistrer la classe de fen�tre
    RegisterClass(&windowClass);

    // Cr�er la fen�tre cach�e
    HWND hiddenWindow = CreateWindowEx(
        0,                              // Styles �tendus
        L"MyHiddenWindowClass",        // Nom de la classe
        L"MyHiddenWindow",              // Titre de la fen�tre
        WS_OVERLAPPEDWINDOW,// Style de la fen�tre (fen�tre cach�e)
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        NULL, NULL, hiddenHInstance, NULL);

    // V�rifier si la fen�tre a �t� cr��e avec succ�s
    if (hiddenWindow == NULL) {
        MessageBox(NULL, L"Erreur lors de la cr�ation de la fen�tre cach�e.", L"Erreur", MB_ICONERROR);
        return 1;
    }

    // Afficher la fen�tre cach�e (si n�cessaire)
    // ShowWindow(hiddenWindow, SW_SHOWNORMAL);

    // cout << "Server Main thread running...\n";

    string Host = "127.0.0.1"; // Server IP
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
            while (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {}
            sf::Vector2i position = sf::Mouse::getPosition(window);
            if (position.x > 0 && position.x < window.getSize().x && position.y > 0 && position.y < window.getSize().y)
            {
                position /= 300;
                if (zones[position.x + position.y * 3].painter == zone::painterList::NONE)
                {
                    userInput = "P" + to_string(painter) + "X" + to_string(position.x) + "Y" + to_string(position.y);

                    int sendResult = send(sock, userInput.c_str(), userInput.size() + 1, 0);
                    if (sendResult != SOCKET_ERROR)
                    {
                        ZeroMemory(buf, 4096);
                        int BytesReceived = recv(sock, buf, 4096, 0);
                        if (BytesReceived)
                        {
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
                                }
                                else
                                {
                                    winner = zone::painterList::CROSS;
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