#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

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
typedef uint16_t bitset_element_index_t;

// Este valor ajusta el tamano del buffer interno de un bitset
// MAX_BUCKETS debe poder ser representable con bucket_index_t
#define MAX_BUCKETS 10

void _conformance_check_bitset(void)
{
	const size_t MAX_TOTAL_BITS = BITS_OF_TYPE(bucket_t) * MAX_BUCKETS;
	assert(MAX_BUCKETS <= MAX_OF_TYPE(bucket_index_t)-1);
	assert(BITS_OF_TYPE(bucket_t) <= MAX_OF_TYPE(bucket_bit_index_t));
	assert(MAX_TOTAL_BITS <= MAX_OF_TYPE(bitset_element_index_t)-1);
}

//------------------------------------------------------------------------------

// Estos valores ajustan las cantidades maximas para estados y simbolos que se 
// usaran en el NFA. Ya que las transiciones de estado en el NFA se representan
// con bitset_t, ajuste de tal manera que pase _conformance_check_nfa().
// Nota: si es necesario vuelva a ajustar MAX_BUCKETS y el tipo de dato de bucket_t
#define MAX_STATES 30
#define MAX_SYMBOLS 254

// Letra en el alfabeto del NFA
typedef uint8_t symbol_t;

// Representa un estado en un NFA
// debido a la implementacion con bitset_t, es recomendable que coincida con el
// tipo de bitset_bit_index_t
typedef bitset_element_index_t state_t;

void _conformance_check_nfa(void)
{
	assert(MAX_STATES == BITS_OF_TYPE(bucket_t) * MAX_BUCKETS - 2);
	assert(MAX_SYMBOLS < MAX_OF_TYPE(symbol_t) - 1);
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

	for(size_t i=0; i < set->bucket_count; i++)
	{
		set->buckets[i] = 0;
	}
}

// Elimina un elemento del conjunto
void bitset_remove(bitset_t* set, bitset_element_index_t i)
{
	assert(set->bucket_count <= MAX_BUCKETS);

	bucket_index_t bucket =  i / BITS_OF_TYPE(bucket_t);
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

	bucket_index_t bucket =  i / BITS_OF_TYPE(bucket_t);
	bucket_bit_index_t bit = i % BITS_OF_TYPE(bucket_t);

	assert(bucket < set->bucket_count);

	set->buckets[bucket] |= (1 << bit);
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
bool bitset_contains(bitset_t set, size_t i)
{
	bucket_index_t bucket =  i / BITS_OF_TYPE(bucket_t);
	bucket_bit_index_t bit = i % BITS_OF_TYPE(bucket_t);

	assert(bucket < set.bucket_count);

	return (set.buckets[bucket] >> bit) & 1 ? true : false;
}

// Realiza la union de dos conjuntos
void bitset_union(bitset_t* ra, const bitset_t b)
{
	assert(ra->bucket_count == b.bucket_count);

	for(bucket_index_t i=0; i < ra->bucket_count; i++)
	{
		ra->buckets[i] |= b.buckets[i];
	}
}

// Realiza la interseccion de dos conjuntos
void bitset_intersect(bitset_t* ra, const bitset_t b)
{
	assert(ra->bucket_count == b.bucket_count);

	for(bucket_index_t i=0; i < ra->bucket_count; i++)
	{
		ra->buckets[i] &= b.buckets[i];
	}
}

// Comprueba si existe al menos un elemento en el conjunto
bool bitset_any(const bitset_t set)
{
	for(bucket_index_t i=0; i<set.bucket_count; i++)
	{
		if(set.buckets[i]) return true;
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
bitset_iterator_t bitset_first(const bitset_t set)
{
	bitset_iterator_t r;
	r.end = false;
	for(r.bucket=0; r.bucket < set.bucket_count; r.bucket++)
	{
		bucket_t b = set.buckets[r.bucket];
		for(r.bit=0; r.bit < sizeof(bucket_t)*8; r.bit++)
		{
			bucket_t t = b >> r.bit;
			if(t & 1) return r;
		}
	}
	r.end = true;
	return r;
}

// Avanza un iterador al siguiente elemento en el conjunto
bitset_iterator_t bitset_next(const bitset_t set, bitset_iterator_t r)
{
	assert(!r.end);
	assert(r.bit < sizeof(bucket_t)*8);
	assert(r.bucket < set.bucket_count);

	for(r.bit++; r.bit < sizeof(bucket_t)*8; r.bit++)
	{		
		bucket_t b = set.buckets[r.bucket] >> r.bit;
		if(b & 1) return r;
	}

	for(r.bucket++; r.bucket < set.bucket_count; r.bucket++)
	{		
		for(r.bit=0; r.bit<sizeof(bucket_t)*8; r.bit++)
		{
			bucket_t b = set.buckets[r.bucket] >> r.bit;
			if(b & 1) return r;
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
	int states;
	int symbols;
} nfa_t;

// Obtiene los estados en el automata
state_t nfa_get_states(nfa_t nfa)
{
	assert(nfa.states <= MAX_STATES);
	return nfa.states;
}

// Obtiene el numero de simbolos asociado al alfabeto del automata
symbol_t nfa_get_symbols(nfa_t nfa)
{
	return nfa.symbols;
}

// Obtiene el conjunto de sucesores de un par estado-simbolo de un automata
void nfa_get_sucessors(nfa_t nfa, state_t state, symbol_t sym, bitset_t* bs)
{
	size_t offset = (state * nfa_get_symbols(nfa) + sym);
	*bs = nfa.forward[offset];
}

// Inicializa un NFA de manera que queda sin estados ni transiciones
void nfa_clear(nfa_t* nfa)
{
	bitset_clear(&nfa->initials);
	bitset_clear(&nfa->finals);
	for(size_t i=0; i<MAX_STATES*MAX_SYMBOLS; i++)
	{
		bitset_clear(&nfa->forward[i]);		
	}
	nfa->states = 0;
	nfa->symbols = 0;
}

// Agrega una transition entre dos estados con un simbolo.
// El estado destino esta representado con iterador de bitset_t.
// La transicion es de q0 -> q1 (usando el simbolo a)
void nfa_add_transition_bsi(nfa_t* nfa, 
							state_t q0, 
							bitset_iterator_t q1,
							symbol_t a)
{
	size_t offset = (q0 * nfa_get_symbols(*nfa) + a);
	bitset_add_iterator(&nfa->forward[offset], q1);

	// TODO: Necesitamos backward?
}

/////////////////////////////////////////////////////////////////////////////
// NFA UTILS

// Comprueba si el automata reconoce la secuencia suministrada
bool accept_sample(nfa_t nfa, const symbol_t* sample, size_t length)
{
	bitset_t next;
	bitset_t current;
	bitset_t tmp;
	
	for(size_t i=0; i<length; i++)
	{
		symbol_t sym = *sample++;
		bitset_clear(&next);
		bool any = false;
		bitset_iterator_t j;
		for(j=bitset_first(current); !bitset_end(j); j=bitset_next(current, j))
		{
			bitset_remove_iterator(&current, j);
			state_t state = bitset_element(j);
			nfa_get_sucessors(nfa, state, sym, &tmp);
			bitset_union(&next, tmp);
			any = true;
		}
		if(!any) return false;
		// swap
		tmp = next;
		next = current;
		current = tmp;
	}

	bitset_intersect(&current, nfa.finals);
	return bitset_any(current);	
}

/////////////////////////////////////////////////////////////////////////////
// OIL

typedef struct _oil_state_t
{
	state_t randomIds[MAX_STATES];
	state_t randomIdsSize;
	state_t randomIdsUsed;
	bitset_t unusedStates;
	nfa_t* nfa;
	bool noRandomSort;
	bool skipBestSearch;
	state_t statesAddedBegin;
} oil_state_t;

// Agrega estados para asegurar que el automata puede reconocer
// la secuencia suministrada
void coerce_match_sample(oil_state_t* state, const symbol_t* sample, size_t length)
{
	assert(state->randomIdsUsed + length < state->randomIdsSize);
		
	// El primer estado es inicial
	bitset_iterator_t i = bitset_first(state->unusedStates);
	bitset_add_iterator(&state->nfa->initials, i);
	bitset_remove_iterator(&state->unusedStates, i);
	state->randomIds[state->randomIdsUsed] = bitset_element(i);
	
	state->statesAddedBegin = state->randomIdsUsed;

	// añadir una transicion por cada simbolo
	for(const symbol_t* s=sample; s<sample+length; s++)
	{
		// TODO: Requiere inicializar unusedStates desde antes
		bitset_iterator_t j = bitset_next(state->unusedStates, i);
		nfa_add_transition_bsi(state->nfa, 
			state->randomIds[state->randomIdsUsed], // q0
			j, // q1
			*s // letter
		);
		state->randomIds[++state->randomIdsUsed] = bitset_element(j);
		i = j;
	}
	
	// el ultimo de la cadena es final
	bitset_add_iterator(&state->nfa->finals, i);
	state->randomIdsUsed++;
	
	assert(accept_sample(*state->nfa, sample, length));
}

// Realiza todas las mezclas de estados posibles
// TODO: Mejorar esta descripcion
void do_all_merges(oil_state_t* state)
{
	
}

// Algoritmo que obtiene un automata NFA que puede reconocer un conjunto de
// secuencias y rechazar otro.
void oil(const symbol_t* sample_buffer, 
		 const size_t sample_buffer_size,
		 const size_t sample_length,
		 const index_t* pindices, size_t ip_size, 
		 const index_t* nindices, size_t in_size,
		 nfa_t* nfa
		 ) 
{
	oil_state_t state;
	state.nfa = nfa;
	state.randomIdsSize = MAX_STATES;
	state.randomIdsUsed = 0;
	state.noRandomSort = false;
	state.skipBestSearch = false;
	state.statesAddedBegin = 0;
	bitset_clear(&state.unusedStates);

	nfa_clear(nfa);

	for(size_t i=0; i<ip_size; i++)
	{
		const index_t pidx = pindices[i];
		if(!accept_sample(*nfa, &sample_buffer[pidx], sample_length))
		{
			coerce_match_sample(&state, &sample_buffer[pidx], sample_length);
			do_all_merges(&state);
		}
	}
}

int main()
{
	_conformance_check_bitset();
	_conformance_check_nfa();
	return 0;
}
