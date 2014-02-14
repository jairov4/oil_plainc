// nfa.c

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
#include "util.h"
#include "nfa.h"
#include <assert.h>
#include <stdio.h>

void _conformance_check_nfa(void)
{
	assert(MAX_STATES <= MAX_OF_TYPE(state_t));
	assert(MAX_STATES <= BITS_OF_TYPE(bucket_t) * MAX_BUCKETS);
	assert(MAX_SYMBOLS <= MAX_OF_TYPE(symbol_t));
}

// Obtiene los estados en el automata
state_t nfa_get_states(const nfa_t* nfa)
{
	//assert(nfa.states <= MAX_STATES);
	//return nfa.states;
	return MAX_STATES;
}

void nfa_add_initial(nfa_t* nfa, state_t q)
{
	assert(q < nfa_get_states(nfa));

	bitset_add(&nfa->initials, q);
}

void nfa_remove_initial(nfa_t* nfa, state_t q)
{
	assert(q < nfa_get_states(nfa));

	bitset_remove(&nfa->initials, q);
}

bool nfa_is_initial(const nfa_t* nfa, state_t q)
{
	assert(q < nfa_get_states(nfa));

	return bitset_contains(&nfa->initials, q);
}

void nfa_get_initials(const nfa_t* nfa, bitset_t* initials)
{
	*initials = nfa->initials;
}

void nfa_add_final(nfa_t* nfa, state_t q)
{
	assert(q < nfa_get_states(nfa));

	bitset_add(&nfa->finals, q);
}

void nfa_remove_final(nfa_t* nfa, state_t q)
{
	assert(q < nfa_get_states(nfa));

	bitset_remove(&nfa->finals, q);
}

bool nfa_is_final(const nfa_t* nfa, state_t q)
{
	assert(q < nfa_get_states(nfa));

	return bitset_contains(&nfa->finals, q);
}

void nfa_get_finals(const nfa_t* nfa, bitset_t* finals)
{
	*finals = nfa->finals;
}

// Obtiene el numero de simbolos asociado al alfabeto del automata
symbol_t nfa_get_symbols(const nfa_t* nfa)
{
	return nfa->symbols;
}

// Obtiene el conjunto de sucesores de un par estado-simbolo de un automata
void nfa_get_sucessors(const nfa_t* nfa, state_t state, symbol_t sym, bitset_t* bs)
{
	assert(state < nfa_get_states(nfa));
	assert(sym < nfa_get_symbols(nfa));

	size_t offset = (state * nfa_get_symbols(nfa) + sym);
	*bs = nfa->forward[offset];
}

// Obtiene el conjunto de predecesores de un par estado-simbolo de un automata
void nfa_get_predecessors(const nfa_t* nfa, state_t state, symbol_t sym, bitset_t* bs)
{
	assert(state < nfa_get_states(nfa));
	assert(sym < nfa_get_symbols(nfa));

	size_t offset = (state * nfa_get_symbols(nfa) + sym);
	*bs = nfa->backward[offset];
}

// Inicializa un NFA de manera que queda sin estados ni transiciones
void nfa_init(nfa_t* nfa, symbol_t symbols)
{
	assert(symbols <= MAX_SYMBOLS);

	bitset_init(&nfa->initials);
	bitset_init(&nfa->finals);
	nfa->symbols = symbols;

	size_t i;
	for (i = 0; i < symbols*MAX_STATES; i++)
	{
		bitset_init(&nfa->forward[i]);
		bitset_init(&nfa->backward[i]);
	}
}

// Agrega una transition entre dos estados con un simbolo.
// El estado destino esta representado con iterador de bitset_t.
// La transicion es de q0 -> q1 (usando el simbolo a)
void nfa_add_transition(nfa_t* nfa,
	state_t q0,
	state_t q1,
	symbol_t a)
{
	assert(a < nfa_get_symbols(nfa));
	assert(q0 < nfa_get_states(nfa));
	assert(q1 < nfa_get_states(nfa));

	size_t offset;
	// successor
	offset = q0 * nfa_get_symbols(nfa) + a;
	bitset_add(&nfa->forward[offset], q1);
	// predecessor
	offset = q1 * nfa_get_symbols(nfa) + a;
	bitset_add(&nfa->backward[offset], q0);
}

// Elimina una transition entre dos estados con un simbolo.
// El estado destino esta representado con iterador de bitset_t.
// La transicion era de q0 -> q1 (usando el simbolo a)
void nfa_remove_transition(nfa_t* nfa,
	state_t q0,
	state_t q1,
	symbol_t a)
{
	assert(a < nfa_get_symbols(nfa));
	assert(q0 < nfa_get_states(nfa));
	assert(q1 < nfa_get_states(nfa));

	size_t offset;
	// successor
	offset = q0 * nfa_get_symbols(nfa) + a;
	bitset_remove(&nfa->forward[offset], q1);
	// predecessor
	offset = q1 * nfa_get_symbols(nfa) + a;
	bitset_remove(&nfa->backward[offset], q0);
}

// Copia el NFA de fuente en destino
void nfa_clone(nfa_t* dest, const nfa_t* src)
{
	*dest = *src;
}

// Combina dos estados en un automata, el estado Q2 queda aislado
void nfa_merge_states(nfa_t* nfa, state_t q1, state_t q2)
{
	assert(q1 < nfa_get_states(nfa));
	assert(q2 < nfa_get_states(nfa));

	if (nfa_is_initial(nfa, q2))
	{
		nfa_add_initial(nfa, q1);
		nfa_remove_initial(nfa, q2);
	}
	if (nfa_is_final(nfa, q2))
	{
		nfa_add_final(nfa, q1);
		nfa_remove_final(nfa, q2);
	}
	symbol_t c;
	for (c = 0; c < nfa->symbols; c++)
	{
		bitset_t bs;

		nfa_get_predecessors(nfa, q2, c, &bs);
		bitset_iterator_t i;
		for (i = bitset_first(&bs); !bitset_end(i); i = bitset_next(&bs, i))
		{
			nfa_add_transition(nfa, bitset_element(i), q1, c);
			nfa_remove_transition(nfa, bitset_element(i), q2, c);
		}

		nfa_get_sucessors(nfa, q2, c, &bs);
		bitset_iterator_t j;
		for (j = bitset_first(&bs); !bitset_end(j); j = bitset_next(&bs, j))
		{
			nfa_add_transition(nfa, q1, bitset_element(j), c);
			nfa_remove_transition(nfa, q2, bitset_element(j), c);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// NFA UTILS

sample_iterator_t sample_iterator_begin(void)
{
	sample_iterator_t r;
	r.index = 0;
	r.sample = 0;
	return r;
}

sample_iterator_t sample_iterator_end(uint16_t length)
{
	sample_iterator_t r;
	r.index = length;
	r.sample = 0;
	return r;
}

sample_iterator_t sample_iterator_next(const index_t indices[MAX_INDICES],
		sample_iterator_t i)
{
	if(i.sample < indices[i.index].samples - 1)
	{
		i.sample++;
	} else {
		i.index++;
		i.sample = 0;
	}
	return i;
}

bool sample_iterator_equals(sample_iterator_t a, sample_iterator_t b)
{
	return (a.sample == b.sample) && (a.index == b.index);
}

// Comprueba si el automata reconoce la secuencia suministrada
bool nfa_accept_sample(const nfa_t* nfa,
	const symbol_t sample[MAX_SAMPLE_LENGTH],
	uint16_t length)
{
#pragma HLS ARRAY_MAP variable=nfa->initials instance=initials_finals horizontal
#pragma HLS ARRAY_MAP variable=nfa->finals instance=initials_finals horizontal
#pragma HLS RESOURCE variable=nfa->initials core=ROM_1P
#pragma HLS RESOURCE variable=nfa->finals core=ROM_1P
#pragma HLS INTERFACE ap_bus port=nfa->forward
#pragma HLS INTERFACE ap_fifo port=sample

	bitset_t next;
	bitset_t current;
	bitset_t tmp;

	bitset_init(&next);
	nfa_get_initials(nfa, &current);

	uint16_t i;
nfa_accept_sample_1_sym:
	for (i = 0; i < length; i++)
	{
		symbol_t sym = sample[i];
		bitset_clear(&next);
		bool any = false;

		bitset_iterator_t j;
nfa_accept_sample_2_bsf:
		for (j = bitset_first(&current); !bitset_end(j); j = bitset_next(&current, j))
		{
			state_t state = bitset_element(j);
			nfa_get_sucessors(nfa, state, sym, &tmp);
			bitset_union(&next, &tmp);
			any = true;
		}
		if (!any) return false;
		// swap
		tmp = next;
		next = current;
		current = tmp;
	}

	nfa_get_finals(nfa, &tmp);
	bitset_intersect(&current, &tmp);
	return bitset_any(&current);
}

// Indica si e NFA acepta al menos una muestra
bool nfa_accept_any_sample(const nfa_t* nfa,
	const symbol_t sample_buffer[MAX_SAMPLE_BUFFER],
	const uint32_t sample_buffer_length,
	const uint16_t sample_length,
	const index_t indices[MAX_INDICES], const uint16_t i_size,
	sample_iterator_t begin, sample_iterator_t end)
{
	sample_iterator_t i;
	for(i = begin; !sample_iterator_equals(i, end); i = sample_iterator_next(indices, i))
	{
		index_t desc = indices[i.index];
		uint32_t offset = desc.stride * i.sample;
		if (nfa_accept_sample(nfa, sample_buffer + offset, sample_length))
		{
			return true;
		}
	}
	return false;
}

#define UNITS 1024
int nfa_accept_samples_generic_hw(const nfa_t* nfa,
	const symbol_t sample_buffer[MAX_SAMPLE_BUFFER],
	const uint32_t sample_buffer_length,
	const uint16_t sample_length,
	const uint32_t offset[UNITS],
	int samples,
	bool stop_on_first, bool accept)
{
#pragma HLS RESOURCE variable=nfa core=ROM_1P
#pragma HLS INTERFACE ap_bus port=nfa->forward
	int i;
	int c = 0;
	for(i=0; i<samples; i++)
	{
		uint32_t begin = offset[i];
		bool r = nfa_accept_sample(nfa, sample_buffer + begin, sample_length);
		if((r && accept) || (!r && !accept))
		{
			if(stop_on_first) return 1;
			c++;
		}
	}
	return c;
}


int nfa_accept_samples_generic(const nfa_t* nfa,
	const symbol_t sample_buffer[MAX_SAMPLE_BUFFER],
	const uint32_t sample_buffer_length,
	const uint16_t sample_length,
	const index_t indices[MAX_INDICES], const uint16_t i_size,
	sample_iterator_t begin, sample_iterator_t end,
	bool stop_on_first, bool accept)
{
	int c = 0;
	sample_iterator_t i;
	for(i = begin; !sample_iterator_equals(i,end); i = sample_iterator_next(indices, i))
	{
		index_t desc = indices[i.index];
		uint32_t offset = desc.stride * i.sample;
		bool r = nfa_accept_sample(nfa, sample_buffer + offset, sample_length);
		if((r && accept) || (!r && !accept))
		{
			c++;
			if(stop_on_first) break;
		}
	}
	return c;
}

// Indica si el NFA acepta todas las muestras
bool nfa_accept_all_samples(const nfa_t* nfa,
	const symbol_t sample_buffer[MAX_SAMPLE_BUFFER],
	const uint32_t sample_buffer_length,
	const uint16_t sample_length,
	const index_t indices[MAX_INDICES], const uint16_t i_size,
	sample_iterator_t begin, sample_iterator_t end)
{
	sample_iterator_t i;
	for(i = begin; !sample_iterator_equals(i, end); i = sample_iterator_next(indices, i))
	{
		index_t desc = indices[i.index];
		uint32_t offset = desc.stride * i.sample;
		if (!nfa_accept_sample(nfa, sample_buffer + offset, sample_length))
		{
			return false;
		}
	}
	return true;
}

// Indica cuantas muestras el NFA acepta
int nfa_accept_samples(const nfa_t* nfa,
	const symbol_t sample_buffer[MAX_SAMPLE_BUFFER],
	const uint32_t sample_buffer_length,
	const uint16_t sample_length,
	const index_t indices[MAX_INDICES], const uint16_t i_size,
	sample_iterator_t begin, sample_iterator_t end)
{
	int c = 0;
	sample_iterator_t i;
	for(i = begin; !sample_iterator_equals(i, end); i = sample_iterator_next(indices, i))
	{
		index_t desc = indices[i.index];
		uint32_t offset = desc.stride * i.sample;
		if (!nfa_accept_sample(nfa, sample_buffer + offset, sample_length))
		{
			c++;
		}
	}
	return c;
}

void nfa_print(const nfa_t* nfa)
{
	state_t q;
	for (q = 0; q < nfa_get_states(nfa); q++)
	{
		bool has_sucessors = false;
		symbol_t a;
		for (a = 0; a < nfa_get_symbols(nfa); a++)
		{
			bitset_t suc;
			nfa_get_sucessors(nfa, q, a, &suc);
			has_sucessors = bitset_any(&suc);
			if (has_sucessors) break;
		}
		if (!has_sucessors) continue;

		printf("%u%s%s", q, nfa_is_initial(nfa, q) ? "I" : "", nfa_is_final(nfa, q) ? "F" : "");
		symbol_t b;
		for (b = 0; b < nfa_get_symbols(nfa); b++)
		{
			printf(" |%u>", b);

			bitset_t suc;
			nfa_get_sucessors(nfa, q, b, &suc);
			bitset_iterator_t qt;
			for (qt = bitset_first(&suc); !bitset_end(qt); qt = bitset_next(&suc, qt))
			{
				printf("%u", bitset_element(qt));
				if (!bitset_end(bitset_next(&suc, qt))) printf(", ");
			}			
		}
		printf("\n");
	}
}
