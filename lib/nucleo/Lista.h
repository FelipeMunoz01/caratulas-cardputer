// La lista de carátulas: qué archivos hay, cuál se está viendo, cómo se pasa
// a la siguiente.
//
// Sin Arduino a propósito, para poder probarla en el Mac. Leer la tarjeta es
// cosa de src/Tarjeta.cpp.
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace caratulas {

constexpr int kMaxCaratulas = 64;

class Lista {
public:
    // Devuelve false si el nombre no sirve o si ya no cabe ninguna más.
    bool agregar(const std::string& nombre);

    void ordenar();  // alfabético, para que el orden no dependa de la tarjeta

    bool vacia() const { return nombres_.empty(); }
    int  cantidad() const { return static_cast<int>(nombres_.size()); }
    int  indice() const { return indice_; }

    const std::string& actual() const;

    void siguiente();
    void anterior();

    // Vuelve a la que estaba la vez pasada. Si ese nombre ya no está en la
    // tarjeta, se queda en la primera en vez de fallar.
    void recuperar(const std::string& nombre);

private:
    std::vector<std::string> nombres_;
    int indice_ = 0;
};

// Un nombre sirve si termina en .gif, sin importar mayúsculas, y no empieza
// con punto: la basura que deja macOS en las tarjetas (._algo, .Spotlight)
// aparecería como carátula si no se filtra.
bool esCaratula(const std::string& nombre);

}  // namespace caratulas
