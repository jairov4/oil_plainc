// oil.c

// Implementacion del algoritmo OIL usando lenguaje C con el fin de
// ser sintetizable en hardware.
// OIL es un algoritmo publicado por vez primera en P. Garcia, M. 
// Vazquez de Parga, G. I. Alvarez, and J. Ruiz, "Universal automata 
// and NFA learning," Theoretical Computer Science, vol. 407, no. 1–3,
// pp. 192–202, Nov. 2008. [http://dx.doi.org/10.1016/j.tcs.2008.05.017]

// 2014, Jairo Andres Velasco R, [jairov_at_javerianacali.edu.co]
// Grupo de investigacion DESTINO
// Pontificia Universidad Javeriana Cali

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

// Obtiene el maximo numero entero sin signo representable con el tipo de dato X
#define MAX_OF_TYPE(X) ((1 << (8*sizeof(X))) - 1)

// Obtiene el numero de bits que mide el tipo de dato X
#define BITS_OF_TYPE(X) (8*sizeof(X))

// Indice de una muestra en el conjunto de muestras
typedef size_t index_t;

//------------------------------------------------------------------------------

// Es un bucket del bitset
typedef uint32_t bucket_t;

// Debe poder representar todos los indices de bit dentro de un bucket_t
typedef uint8_t bucket_bit_index_t;

// Debe poder representar todos los indices de bucket dentro de un bitset_t
typedef uint8_t bucket_index_t;

// Debe poder representar todos los indices de un bit dentro de bitset_t
typedef uint8_t bitset_element_index_t;

// Este valor ajusta el tamano del buffer interno de un bitset
// MAX_BUCKETS debe poder ser representable con bucket_index_t
#define MAX_BUCKETS 2

void _conformance_check_bitset(void)
{
	const size_t MAX_TOTAL_BITS = BITS_OF_TYPE(bucket_t) * MAX_BUCKETS;
	assert(MAX_BUCKETS <= MAX_OF_TYPE(bucket_index_t));
	assert(BITS_OF_TYPE(bucket_t) - 1 <= MAX_OF_TYPE(bucket_bit_index_t));
	assert(MAX_TOTAL_BITS <= MAX_OF_TYPE(bitset_element_index_t));
}

//------------------------------------------------------------------------------

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

void _conformance_check_nfa(void)
{
	assert(MAX_STATES <= MAX_OF_TYPE(state_t));
	assert(MAX_STATES <= BITS_OF_TYPE(bucket_t) * MAX_BUCKETS);
	assert(MAX_SYMBOLS <= MAX_OF_TYPE(symbol_t));
}

/////////////////////////////////////////////////////////////////////////////
// Bitset
// Requiere configurar:
// - bucket_t
// - MAX_BUCKETS
// - bucket_bit_index_t

// Conjunto de bits
typedef struct _bitset_t
{
	bucket_t buckets[MAX_BUCKETS];
	bucket_index_t bucket_count;
} bitset_t;

// Iterador para elementos en un conjunto
typedef struct _bitset_iterator_t
{
	bucket_bit_index_t bit;
	bucket_index_t bucket;
	bool end;
} bitset_iterator_t;

// Elimina todos los elementos en un conjunto
void bitset_clear(bitset_t* set)
{
	assert(set->bucket_count <= MAX_BUCKETS);

	for (size_t i = 0; i < set->bucket_count; i++)
	{
		set->buckets[i] = 0;
	}
}

// Inicializa el conjunto, queda sin contenido
void bitset_init(bitset_t* set)
{
	set->bucket_count = MAX_BUCKETS;
	bitset_clear(set);
}

// Elimina un elemento del conjunto
void bitset_remove(bitset_t* set, bitset_element_index_t i)
{
	assert(set->bucket_count <= MAX_BUCKETS);

	bucket_index_t bucket = i / BITS_OF_TYPE(bucket_t);
	bucket_bit_index_t bit = i % BITS_OF_TYPE(bucket_t);

	assert(bucket < set->bucket_count);

	set->buckets[bucket] &= ~(1 << bit);
}

// Elimina un elemento indicado por un iterador del bitset
void bitset_remove_iterator(bitset_t* set, bitset_iterator_t i)
{
	assert(set->bucket_count <= MAX_BUCKETS);
	assert(!i.end);
	assert(i.bucket < set->bucket_count);
	assert(i.bit < BITS_OF_TYPE(bucket_t));

	set->buckets[i.bucket] &= ~(1 << i.bit);
}

// Agrega un elemento a un conjunto
void bitset_add(bitset_t* set, bitset_element_index_t i)
{
	assert(set->bucket_count <= MAX_BUCKETS);

	bucket_index_t bucket = i / BITS_OF_TYPE(bucket_t);
	bucket_bit_index_t bit = i % BITS_OF_TYPE(bucket_t);

	assert(bucket < set->bucket_count);

	set->buckets[bucket] |= (1 << bit);
}

// Agrega un rango de elementos al conjunto
void bitset_add_range(bitset_t* set, bitset_element_index_t begin, bitset_element_index_t len)
{	
	while (len--)
	{
		bitset_add(set, begin++);
	}
}

// Agrega un elemento indicado por un iterador
void bitset_add_iterator(bitset_t* set, bitset_iterator_t i)
{
	assert(set->bucket_count <= MAX_BUCKETS);
	assert(!i.end);
	assert(i.bucket < set->bucket_count);
	assert(i.bit < BITS_OF_TYPE(bucket_t));

	set->buckets[i.bucket] |= (1 << i.bit);
}

// Prueba si un elemento esta contenido en conjunto de bits
bool bitset_contains(const bitset_t* set, size_t i)
{
	bucket_index_t bucket = i / BITS_OF_TYPE(bucket_t);
	bucket_bit_index_t bit = i % BITS_OF_TYPE(bucket_t);

	assert(bucket < set->bucket_count);

	return (set->buckets[bucket] >> bit) & 1 ? true : false;
}

// Realiza la union de dos conjuntos
void bitset_union(bitset_t* ra, const bitset_t* b)
{
	assert(ra->bucket_count == b->bucket_count);

	for (bucket_index_t i = 0; i < ra->bucket_count; i++)
	{
		ra->buckets[i] |= b->buckets[i];
	}
}

// Realiza la interseccion de dos conjuntos
void bitset_intersect(bitset_t* ra, const bitset_t* b)
{
	assert(ra->bucket_count == b->bucket_count);

	for (bucket_index_t i = 0; i < ra->bucket_count; i++)
	{
		ra->buckets[i] &= b->buckets[i];
	}
}

// Comprueba si existe al menos un elemento en el conjunto
bool bitset_any(const bitset_t* set)
{
	for (bucket_index_t i = 0; i < set->bucket_count; i++)
	{
		if (set->buckets[i]) return true;
	}
	return false;
}

// Obtiene el elemento apuntado por un iterador
bitset_element_index_t bitset_element(const bitset_iterator_t i)
{
	assert(!i.end);

	return i.bit + i.bucket*BITS_OF_TYPE(bucket_t);
}

// Obtiene un iterador apuntando al primer elemento en un conjunto
bitset_iterator_t bitset_first(const bitset_t* set)
{
	bitset_iterator_t r;
	r.end = false;
	for (r.bucket = 0; r.bucket < set->bucket_count; r.bucket++)
	{
		bucket_t b = set->buckets[r.bucket];
		for (r.bit = 0; r.bit < sizeof(bucket_t)* 8; r.bit++)
		{
			bucket_t t = b >> r.bit;
			if (t & 1) return r;
		}
	}
	r.end = true;
	return r;
}

// Avanza un iterador al siguiente elemento en el conjunto
bitset_iterator_t bitset_next(const bitset_t* set, bitset_iterator_t r)
{
	assert(!r.end);
	assert(r.bit < sizeof(bucket_t)* 8);
	assert(r.bucket < set->bucket_count);

	for (r.bit++; r.bit < sizeof(bucket_t)* 8; r.bit++)
	{
		bucket_t b = set->buckets[r.bucket] >> r.bit;
		if (b & 1) return r;
	}

	for (r.bucket++; r.bucket < set->bucket_count; r.bucket++)
	{
		for (r.bit = 0; r.bit < sizeof(bucket_t)* 8; r.bit++)
		{
			bucket_t b = set->buckets[r.bucket] >> r.bit;
			if (b & 1) return r;
		}
	}
	r.end = true;
	return r;
}

// Comprueba si un iterador ya rebaso el final del conjunto
bool bitset_end(bitset_iterator_t r)
{
	return r.end;
}

/////////////////////////////////////////////////////////////////////////////
// NFA
// Requiere configurar
// - <bitset>
// - symbol_t
// - state_t
// - MAX_STATES
// - MAX_SYMBOLS

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
	for (size_t i = 0; i < symbols*MAX_STATES; i++)
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
	for (symbol_t c = 0; c < nfa->symbols; c++)
	{
		bitset_t bs;

		nfa_get_predecessors(nfa, q2, c, &bs);
		for (bitset_iterator_t i = bitset_first(&bs); !bitset_end(i); i = bitset_next(&bs, i))
		{
			nfa_add_transition(nfa, bitset_element(i), q1, c);
			nfa_remove_transition(nfa, bitset_element(i), q2, c);
		}

		nfa_get_sucessors(nfa, q2, c, &bs);
		for (bitset_iterator_t i = bitset_first(&bs); !bitset_end(i); i = bitset_next(&bs, i))
		{
			nfa_add_transition(nfa, q1, bitset_element(i), c);
			nfa_remove_transition(nfa, q2, bitset_element(i), c);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// NFA UTILS

// Comprueba si el automata reconoce la secuencia suministrada
bool nfa_accept_sample(const nfa_t* nfa, const symbol_t* sample, size_t length)
{
	bitset_t next;
	bitset_t current;
	bitset_t tmp;

	bitset_init(&next);
	nfa_get_initials(nfa, &current);
	for (size_t i = 0; i < length; i++)
	{
		symbol_t sym = *sample++;
		bitset_clear(&next);
		bool any = false;
		bitset_iterator_t j;
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
	const symbol_t* sample_buffer,
	const size_t sample_buffer_length,
	const size_t sample_length,
	const index_t* indices, const size_t i_size)
{
	for (size_t i = 0; i < i_size; i++)
	{
		assert(indices[i] < sample_buffer_length);
		if (nfa_accept_sample(nfa, sample_buffer + indices[i], sample_length))
		{
			return true;
		}
	}
	return false;
}

// Indica si el NFA acepta todas las muestras
bool nfa_accept_all_samples(const nfa_t* nfa,
	const symbol_t* sample_buffer,
	const size_t sample_buffer_length,
	const size_t sample_length,
	const index_t* indices, const size_t i_size)
{
	for (size_t i = 0; i < i_size; i++)
	{
		assert(indices[i] < sample_buffer_length);
		if (!nfa_accept_sample(nfa, sample_buffer + indices[i], sample_length))
		{
			return false;
		}
	}
	return true;
}

// Indica cuantas muestras el NFA acepta
int nfa_accept_samples(const nfa_t* nfa,
	const symbol_t* sample_buffer,
	const size_t sample_buffer_length,
	const size_t sample_length,
	const index_t* indices, const size_t i_size)
{
	int c = 0;
	for (size_t i = 0; i < i_size; i++)
	{
		assert(indices[i] < sample_buffer_length);
		if (nfa_accept_sample(nfa, sample_buffer + indices[i], sample_length))
		{
			c++;
		}
	}
	return c;
}

void nfa_print(const nfa_t* nfa)
{
	for (state_t q = 0; q < nfa_get_states(nfa); q++)
	{
		bool has_sucessors = false;
		for (symbol_t a = 0; a < nfa_get_symbols(nfa); a++)
		{
			bitset_t suc;
			nfa_get_sucessors(nfa, q, a, &suc);
			has_sucessors = bitset_any(&suc);
			if (has_sucessors) break;
		}
		if (!has_sucessors) continue;

		printf("%u%s%s", q, nfa_is_initial(nfa, q) ? "I" : "", nfa_is_final(nfa, q) ? "F" : "");
		for (symbol_t a = 0; a < nfa_get_symbols(nfa); a++)
		{
			printf(" |%u>", a);

			bitset_t suc;
			nfa_get_sucessors(nfa, q, a, &suc);
			for (bitset_iterator_t qt = bitset_first(&suc); !bitset_end(qt); qt = bitset_next(&suc, qt))
			{
				printf("%u", bitset_element(qt));
				if (!bitset_end(bitset_next(&suc, qt))) printf(", ");
			}			
		}
		printf("\n");
	}
}

/////////////////////////////////////////////////////////////////////////////
// OIL

typedef struct _oil_state_t
{
	// Vector de estados aleatorio
	state_t pool[MAX_STATES];

	// Tamano del vector de estados aleatorio
	state_t pool_size;

	// conjunto de estados aislados o sin usar en el NFA
	bitset_t unused_states;

	// NFA hipotesis
	nfa_t* nfa;

	// Ejecuta el algoritmo de manera que no utiliza orden aleatorio
	bool no_random_sort;

	// Si se habilita, OIL se conformara con la primera mezcla de estados
	// que se considere valida
	bool skip_search_best;

	// Estados en el automata
	state_t states;

	// Indice en el vector de estados aleatorio a partir del cual se encuentran
	// los nuevos estados a ser combinados
	state_t new_states_begin;

	// Indice de la muestra positiva en la que se encuentra el procesamiento
	size_t sample_index;

	// Contador de mezclas exitosas realizadas
	int merge_counter;

	bool print_merge_alternatives;
	bool print_merges;
	bool print_progress;
} oil_state_t;

// Aplica orden aleatorio a una secuencia de estados
void oil_random_shuffle(state_t* buffer, state_t len)
{
	for (state_t i = len - 1; i > 0; i--)
	{
		state_t j = rand() % (i + 1);
		// swap
		state_t tmp = buffer[i];
		buffer[i] = buffer[j];
		buffer[j] = tmp;
	}
}

// Agrega estados para asegurar que el automata puede reconocer
// la secuencia suministrada
void oil_coerce_match_sample(oil_state_t* state, const symbol_t* sample, size_t length)
{
	assert(state->states + length + 1 <= state->pool_size);

	state->new_states_begin = state->states;
	state_t* new_state = &state->pool[state->new_states_begin];

	// El primer estado es inicial
	bitset_iterator_t i = bitset_first(&state->unused_states);
	state_t qi = bitset_element(i);
	nfa_add_initial(state->nfa, qi);	
	*new_state++ = qi;
	bitset_remove_iterator(&state->unused_states, i);
	
	// añadir una transicion por cada simbolo
	for (const symbol_t* s = sample; s < sample + length; s++)
	{
		bitset_iterator_t j = bitset_next(&state->unused_states, i);
		bitset_remove_iterator(&state->unused_states, j);		
		state_t qt = bitset_element(j);
		nfa_add_transition(state->nfa, qi, qt, *s);
		*new_state++ = qt;
		i = j; qi = qt;
	}

	// el ultimo de la cadena es final
	nfa_add_final(state->nfa, qi);
	
	state->states += length + 1;
	assert(nfa_accept_sample(state->nfa, sample, length));
}

// Realiza todas las mezclas de estados que sean posibles. 
// Solo se considera posible una mezcla de estados donde el NFA resultante 
// reconoce las mismas muestras positivas tenidas en cuenta hasta el momento y
// rechaza todas las muestras negativas disponibles.
void oil_do_all_merges(oil_state_t* state,
	const symbol_t* sample_buffer,
	const size_t sample_buffer_size,
	const size_t sample_length,
	const index_t* pindices, const size_t ip_size,
	const index_t* nindices, const size_t in_size
	)
{
	size_t next_sample_index = state->sample_index + 1;
	if (!state->no_random_sort)
	{
		state_t begin = state->new_states_begin;
		state_t len = state->states - begin;
		oil_random_shuffle(state->pool + begin, len);
	}

	for (state_t i = state->new_states_begin; i < state->states;)
	{
		int best_score = -1;
		int best_j = -1;
		state_t s1 = state->pool[i];
		nfa_t best_nfa;

		for (state_t j = 0; j < i; j++)
		{
			state_t s2 = state->pool[j];
			nfa_t lnfa;
			nfa_clone(&lnfa, state->nfa);
			nfa_merge_states(&lnfa, s2, s1);

			bool anyNegMatch = nfa_accept_any_sample(&lnfa,
				sample_buffer,
				sample_buffer_size,
				sample_length,
				nindices, in_size
				);
			if (anyNegMatch) continue;

			int score = nfa_accept_samples(&lnfa,
				sample_buffer,
				sample_buffer_size,
				sample_length,
				&pindices[next_sample_index], // indices
				ip_size - next_sample_index); // index buffer length

			if (score > best_score)
			{
				best_score = score;
				best_j = j;
				nfa_clone(&best_nfa, &lnfa);
				if (state->skip_search_best) break;
				if (state->print_merge_alternatives)
				{
					printf("merge alternative: %u %u (states: %u %u) [score: %d]\n", 
						i, j, s1, s2, score);
				}
			}
		}

		// si se encontro mezcla exitosa
		// eliminamos el estado eliminado del vector de estados aleatorio
		if (best_score != -1)
		{
			state->merge_counter++;
			bitset_add(&state->unused_states, state->pool[i]);
			if (state->print_merges)
			{
				printf("merge: %u %u (states %u %u) [score: %d]\n", 
					i, best_j, s1, state->pool[i], best_score);
			}
			if (state->no_random_sort)
			{
				memmove(state->pool + i, state->pool + i + 1,
					state->states - state->new_states_begin);
			}
			else
			{
				state->pool[i] = state->pool[state->states - 1];
			}
			state->states--;
			nfa_clone(state->nfa, &best_nfa);
		}
		else
		{
			i++;
		}
	}

	assert(!nfa_accept_any_sample(state->nfa,
		sample_buffer, sample_buffer_size, sample_length,
		nindices, in_size));
	assert(nfa_accept_all_samples(state->nfa,
		sample_buffer, sample_buffer_size, sample_length,
		pindices, state->sample_index + 1));
}

// Algoritmo que obtiene un automata NFA que puede reconocer un conjunto de
// secuencias y rechazar otro.
void oil(const symbol_t* sample_buffer,
	const size_t sample_buffer_size,
	const size_t sample_length,
	const symbol_t symbols,
	const index_t* pindices, const size_t ip_size,
	const index_t* nindices, const size_t in_size,
	nfa_t* nfa
	)
{
	oil_state_t state;
	state.nfa = nfa;
	state.pool_size = MAX_STATES;
	state.states = 0;
	state.no_random_sort = false;
	state.skip_search_best = false;
	state.new_states_begin = 0;
	state.merge_counter = 0;

	// print debug info
	state.print_merges = true;
	state.print_progress = true;
	state.print_merge_alternatives = true;

	// inicializa los estados no usados
	bitset_init(&state.unused_states);
	bitset_add_range(&state.unused_states, 0, MAX_STATES);
	
	nfa_init(nfa, symbols);

	if (state.print_progress)
	{
		printf("oil start. sample_length: %u. ip_size: %u, in_size: %u, symbols: %u\n", 
			sample_length, ip_size, in_size, symbols);
	}

	for (state.sample_index = 0; state.sample_index < ip_size; state.sample_index++)
	{
		const index_t pidx = pindices[state.sample_index];
		if (!nfa_accept_sample(nfa, &sample_buffer[pidx], sample_length))
		{
			oil_coerce_match_sample(&state, &sample_buffer[pidx], sample_length);
			oil_do_all_merges(&state,
				sample_buffer,
				sample_buffer_size,
				sample_length,
				pindices, ip_size,
				nindices, in_size
				);
			if (state.print_progress)
			{
				printf("progress: %0.1f%% sample: %u/%u [states: %u]\n",
					((state.sample_index+1)*100.0f/ip_size),
					state.sample_index+1, ip_size,
					state.states);
			}
		}
	}
}

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

