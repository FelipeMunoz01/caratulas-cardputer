#include "Lista.h"

#include <algorithm>

namespace caratulas {
namespace {
const std::string kVacio;

char minuscula(char c) { return (c >= 'A' && c <= 'Z') ? c - 'A' + 'a' : c; }
}  // namespace

bool esCaratula(const std::string& nombre) {
    if (nombre.size() < 5) return false;   // "a.gif" es el mínimo
    if (nombre[0] == '.') return false;    // ._loquesea y amigos
    const std::string fin = nombre.substr(nombre.size() - 4);
    std::string baja;
    for (char c : fin) baja += minuscula(c);
    return baja == ".gif";
}

bool Lista::agregar(const std::string& nombre) {
    if (!esCaratula(nombre)) return false;
    if (cantidad() >= kMaxCaratulas) return false;
    nombres_.push_back(nombre);
    return true;
}

void Lista::ordenar() {
    std::sort(nombres_.begin(), nombres_.end(),
              [](const std::string& a, const std::string& b) {
                  std::string ba, bb;
                  for (char c : a) ba += minuscula(c);
                  for (char c : b) bb += minuscula(c);
                  return ba < bb;
              });
}

const std::string& Lista::actual() const {
    if (nombres_.empty()) return kVacio;
    return nombres_[indice_];
}

void Lista::siguiente() {
    if (nombres_.empty()) return;
    indice_ = (indice_ + 1) % cantidad();
}

void Lista::anterior() {
    if (nombres_.empty()) return;
    indice_ = (indice_ + cantidad() - 1) % cantidad();
}

void Lista::recuperar(const std::string& nombre) {
    for (int n = 0; n < cantidad(); ++n) {
        if (nombres_[n] == nombre) {
            indice_ = n;
            return;
        }
    }
    indice_ = 0;
}

}  // namespace caratulas
