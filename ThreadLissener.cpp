#include "ThreadLissener.h"
#include <iostream>
#include <thread>
#include <SFML/System/Thread.hpp>
using namespace std;
void Connexions() {
        // Nouvelle connexion détectée, faire quelque chose avec la socket (par exemple, créer un thread dédié pour gérer le joueur)
    for (int i = 0; i < 5; ++i)
    {
        std::cout << "Dans le thread, connexion: " << i << std::endl;
    }
}

void ThreadPlay()
{
    thread Threadplay (Connexions); 
}