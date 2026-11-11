// Pruebas de la lista de carátulas. Corren en el Mac, sin equipo ni tarjeta.
#include <unity.h>

#include "Lista.h"

using namespace caratulas;

void setUp() {}
void tearDown() {}

void test_reconoce_los_gif_sin_importar_mayusculas() {
    TEST_ASSERT_TRUE(esCaratula("lobo.gif"));
    TEST_ASSERT_TRUE(esCaratula("LOBO.GIF"));
    TEST_ASSERT_TRUE(esCaratula("Metal Slug.Gif"));
}

void test_descarta_lo_que_no_es_caratula() {
    TEST_ASSERT_FALSE(esCaratula("lobo.png"));
    TEST_ASSERT_FALSE(esCaratula("lobo.gif.txt"));
    TEST_ASSERT_FALSE(esCaratula(".gif"));
    TEST_ASSERT_FALSE(esCaratula(""));
}

void test_descarta_la_basura_que_deja_macos() {
    // Al copiar archivos desde el Mac quedan estos acompañantes, y sin
    // filtrarlos aparecerian como caratulas en negro.
    TEST_ASSERT_FALSE(esCaratula("._lobo.gif"));
    TEST_ASSERT_FALSE(esCaratula(".Spotlight-V100.gif"));
}

void test_pasar_de_una_a_otra_da_la_vuelta() {
    Lista l;
    l.agregar("a.gif");
    l.agregar("b.gif");
    l.agregar("c.gif");
    TEST_ASSERT_EQUAL_STRING("a.gif", l.actual().c_str());
    l.siguiente();
    l.siguiente();
    TEST_ASSERT_EQUAL_STRING("c.gif", l.actual().c_str());
    l.siguiente();
    TEST_ASSERT_EQUAL_STRING("a.gif", l.actual().c_str());
    l.anterior();
    TEST_ASSERT_EQUAL_STRING("c.gif", l.actual().c_str());
}

void test_una_lista_vacia_no_revienta() {
    Lista l;
    TEST_ASSERT_TRUE(l.vacia());
    TEST_ASSERT_EQUAL_STRING("", l.actual().c_str());
    l.siguiente();   // no debe hacer nada ni caerse
    l.anterior();
    TEST_ASSERT_EQUAL_INT(0, l.indice());
}

void test_el_orden_no_depende_de_la_tarjeta() {
    Lista l;
    l.agregar("zorro.gif");
    l.agregar("Avion.gif");
    l.agregar("lobo.gif");
    l.ordenar();
    TEST_ASSERT_EQUAL_STRING("Avion.gif", l.actual().c_str());
    l.siguiente();
    TEST_ASSERT_EQUAL_STRING("lobo.gif", l.actual().c_str());
}

void test_recupera_la_ultima_vista() {
    Lista l;
    l.agregar("a.gif");
    l.agregar("b.gif");
    l.agregar("c.gif");
    l.recuperar("c.gif");
    TEST_ASSERT_EQUAL_STRING("c.gif", l.actual().c_str());
}

void test_si_la_ultima_ya_no_esta_vuelve_a_la_primera() {
    Lista l;
    l.agregar("a.gif");
    l.agregar("b.gif");
    l.recuperar("borrada.gif");
    TEST_ASSERT_EQUAL_STRING("a.gif", l.actual().c_str());
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_reconoce_los_gif_sin_importar_mayusculas);
    RUN_TEST(test_descarta_lo_que_no_es_caratula);
    RUN_TEST(test_descarta_la_basura_que_deja_macos);
    RUN_TEST(test_pasar_de_una_a_otra_da_la_vuelta);
    RUN_TEST(test_una_lista_vacia_no_revienta);
    RUN_TEST(test_el_orden_no_depende_de_la_tarjeta);
    RUN_TEST(test_recupera_la_ultima_vista);
    RUN_TEST(test_si_la_ultima_ya_no_esta_vuelve_a_la_primera);
    return UNITY_END();
}
