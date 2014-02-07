// nfa.h

// Este archivo hace parte de la implementacion del algoritmo OIL usando
// lenguaje C con el fin de ser sintetizable en hardware.
// Este archivo contiene la implementacion de un automata no determinista
// finito que sera usado por el metodo principal.
// OIL es un algoritmo publicado por vez primera en P. Garcia, M. 
// Vazquez de Parga, G. I. Alvarez, and J. Ruiz, "Universal automata 
// and NFA learning," Theoretical Computer Science, vol. 407, no. 1–3,
// pp. 192–202, Nov. 2008. [http://dx.doi.org/10.1016/j.tcs.2008.05.017]

// 2014, Jairo Andres Velasco R, [jairov_at_javerianacali.edu.co]
// Grupo de investigacion DESTINO
// Pontificia Universidad Javeriana Cali
//------------------------------------------------------------------------------
#pragma once
#include "bitset.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

/////////////////////////////////////////////////////////////////////////////
// NFA
// Requiere configurar
// - <bitset>
// - symbol_t
// - state_t
// - MAX_STATES
// - MAX_SYMBOLS


// Estos valores ajustan las cantidades maximas para estados y simbolos que se 
// usaran en el NFA. Ya que las transiciones de estado en el NFA se representan
// con bitset_t, ajuste de tal manera que pase _conformance_check_nfa().
// Nota: si es necesario vuelva a ajustar MAX_BUCKETS y el tipo de dato de bucket_t
#define MAX_STATES 63u
#define MAX_SYMBOLS 255u

// Letra en el alfabeto del NFA
typedef uint8_t symbol_t;

// Representa un estado en un NFA
// debido a la implementacion con bitset_t, es recomendable que coincida con el
// tipo de bitset_bit_index_t
typedef bitset_element_index_t state_t;

// Indice de una muestra en el conjunto de muestras
typedef size_t index_t;

// Representa un Non-Deterministic Finite Automata
typedef struct _nfa_t
{
	bitset_t initials;
	bitset_t finals;
	bitset_t forward[MAX_STATES*MAX_SYMBOLS];
	bitset_t backward[MAX_STATES*MAX_SYMBOLS];
//	int states;
	int symbols;
} nfa_t;

void _conformance_check_nfa(void);

// Obtiene los estados en el automata
state_t nfa_get_states(const nfa_t* nfa);

void nfa_add_initial(nfa_t* nfa, state_t q);

void nfa_remove_initial(nfa_t* nfa, state_t q);

bool nfa_is_initial(const nfa_t* nfa, state_t q);

void nfa_get_initials(const nfa_t* nfa, bitset_t* initials);

void nfa_add_final(nfa_t* nfa, state_t q);

void nfa_remove_final(nfa_t* nfa, state_t q);

bool nfa_is_final(const nfa_t* nfa, state_t q);

void nfa_get_finals(const nfa_t* nfa, bitset_t* finals);

// Obtiene el numero de simbolos asociado al alfabeto del automata
symbol_t nfa_get_symbols(const nfa_t* nfa);

// Obtiene el conjunto de sucesores de un par estado-simbolo de un automata
void nfa_get_sucessors(const nfa_t* nfa, state_t state, symbol_t sym, bitset_t* bs);

// Obtiene el conjunto de predecesores de un par estado-simbolo de un automata
void nfa_get_predecessors(const nfa_t* nfa, state_t state, symbol_t sym, bitset_t* bs);

// Inicializa un NFA de manera que queda sin estados ni transiciones
void nfa_init(nfa_t* nfa, symbol_t symbols);

// Agrega una transition entre dos estados con un simbolo.
// El estado destino esta representado con iterador de bitset_t.
// La transicion es de q0 -> q1 (usando el simbolo a)
void nfa_add_transition(nfa_t* nfa,
	state_t q0,
	state_t q1,
	symbol_t a);

// Elimina una transition entre dos estados con un simbolo.
// El estado destino esta representado con iterador de bitset_t.
// La transicion era de q0 -> q1 (usando el simbolo a)
void nfa_remove_transition(nfa_t* nfa,
	state_t q0,
	state_t q1,
	symbol_t a);

// Copia el NFA de fuente en destino
void nfa_clone(nfa_t* dest, const nfa_t* src);

// Combina dos estados en un automata, el estado Q2 queda aislado
void nfa_merge_states(nfa_t* nfa, state_t q1, state_t q2);

/////////////////////////////////////////////////////////////////////////////
// NFA UTILS

// Comprueba si el automata reconoce la secuencia suministrada
bool nfa_accept_sample(const nfa_t* nfa, const symbol_t* sample, size_t length);

// Indica si e NFA acepta al menos una muestra
bool nfa_accept_any_sample(const nfa_t* nfa,
	const symbol_t* sample_buffer,
	const size_t sample_buffer_length,
	const size_t sample_length,
	const index_t* indices, const size_t i_size);

// Indica si el NFA acepta todas las muestras
bool nfa_accept_all_samples(const nfa_t* nfa,
	const symbol_t* sample_buffer,
	const size_t sample_buffer_length,
	const size_t sample_length,
	const index_t* indices, const size_t i_size);

// Indica cuantas muestras el NFA acepta
int nfa_accept_samples(const nfa_t* nfa,
	const symbol_t* sample_buffer,
	const size_t sample_buffer_length,
	const size_t sample_length,
	const index_t* indices, const size_t i_size);

void nfa_print(const nfa_t* nfa);
