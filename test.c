// test.c

// Implementacion del algoritmo OIL usando lenguaje C con el fin de
// ser sintetizable en hardware.
// Este archivo contiene metodos de prueba para asegurar la correcta 
// funcionalidad del algoritmo.
// OIL es un algoritmo publicado por vez primera en P. Garcia, M. 
// Vazquez de Parga, G. I. Alvarez, and J. Ruiz, "Universal automata 
// and NFA learning," Theoretical Computer Science, vol. 407, no. 1–3,
// pp. 192–202, Nov. 2008. [http://dx.doi.org/10.1016/j.tcs.2008.05.017]

// 2014, Jairo Andres Velasco R, [jairov_at_javerianacali.edu.co]
// Grupo de investigacion DESTINO
// Pontificia Universidad Javeriana Cali
#include <stdlib.h>
#include <stdio.h>
#include "nfa.h"
#include "oil.h"

/////////////////////////////////////////////////////////////////////////////
// TEST

void test(void)
{
	nfa_t nfa;
	symbol_t sample_buffer[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 };
	size_t buffer_size = 12;
	size_t sample_length = 3;
	index_t pindices[] = { 3, 8 };
	size_t psize = 2;
	index_t nindices[] = { 0, 1, 2, 4, 5, 6, 7, 9 };
	size_t nsize = 8;
	oil(sample_buffer, buffer_size, sample_length, 
		13,
		pindices, psize, 
		nindices, nsize, 
		&nfa);
	nfa_print(&nfa);
}

/////////////////////////////////////////////////////////////////////////////
// MAIN

int main(void)
{
	_conformance_check_bitset();
	_conformance_check_nfa();
	test();
	return 0;
}

