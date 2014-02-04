#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

// Indice de una muestra en el conjunto de muestras
typedef size_t index_t;

// Letra en el alfabeto del NFA
typedef uint8_t symbol_t;

// Es un bucket del bitset
typedef uint32_t bucket_t;

// Representa un estado en un NFA
typedef uint16_t state_t;

// Debe poder representar todos los indices de bit dentro de un bucket_t
typedef uint8_t bucket_bit_index_t;

// Conjunto de bits
typedef struct _bitset_t
{
	bucket_t* buckets;
	size_t bucket_count;
} bitset_t;

// Iteratdor para 1s en un bitset
typedef struct _bitset_iterator_t
{
	size_t bit;
	size_t bucket;
	bool end;
} bitset_iterator_t;

// Elimina todos los elementos en un conjunto
void bitset_clear(bitset_t* set)
{
	for(size_t i=0; i < set->bucket_count; i++)
	{
		set->buckets[i] = 0;
	}
}

// Elimina un elemento del conjunto
void bitset_remove(bitset_t* set, size_t i)
{
	size_t bucket = i / (8*sizeof(bucket_t));
	size_t bit = i % (8*sizeof(bucket_t));
	// assert(bucket < set->bucket_count);
	set->buckets[bucket] &= ~(1 << bit);
}

// Elimina un elemento indicado por un iterador del bitset
void bitset_remove(bitset_t* set, bitset_iterator_t i)
{
	// assert(!i.end);
	// assert(i.bucket < set->bucket_count);
	// assert(i.bit < 8*sizeof(bucket_t));
	set->buckets[i.bucket] &= ~(1 << i.bit);
}

// Agrega un elemento a un conjunto
void bitset_add(bitset_t* set, size_t i)
{
	size_t bucket = i / (8*sizeof(bucket_t));
	size_t bit = i % (8*sizeof(bucket_t));
	// assert(bucket < set->bucket_count);
	set->buckets[bucket] |= (1 << bit);
}

// Prueba si un elemento esta contenido en conjunto de bits
bool bitset_contains(bitset_t* set, size_t i)
{
	size_t bucket = i / (8*sizeof(bucket_t));
	size_t bit = i % (8*sizeof(bucket_t));
	// assert(bucket < set->bucket_count);
	return (set->buckets[bucket] >> bit) & 1 ? true : false;
}

// Realiza la union de dos conjuntos
void bitset_union(bitset_t* ra, const bitset_t* b)
{
	// assert(ra->bucket_count == b->bucket_count);
	for(size_t i=0; i < ra->bucket_count; i++)
	{
		ra->buckets[i] |= b->buckets[i];
	}
}

// Realiza la interseccion de dos conjuntos
void bitset_intersect(bitset_t* ra, const bitset_t* b)
{
	// assert(ra->bucket_count == b->bucket_count);
	for(size_t i=0; i < ra->bucket_count; i++)
	{
		ra->buckets[i] &= b->buckets[i];
	}
}

// Comprueba si existe al menos un elemento en el conjunto
bool bitset_any(const bitset_t set)
{
	for(size_t i=0; i<set.bucket_count; i++)
	{
		if(set.buckets[i]) return true;
	}
	return false;
}

// Obtiene el elemento apuntado por un iterador
size_t bitset_bit(const bitset_iterator_t i)
{
	// assert(!i.end);
	return i.bit + i.bucket*sizeof(bucket_t)*8;
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
void bitset_next(const bitset_t set, bitset_iterator_t* r)
{
	for(r->bit++; r->bit < sizeof(bucket_t)*8; r->bit++)
	{		
		bucket_t b = set.buckets[r->bucket] >> r->bit;
		if(b & 1) return;
	}

	for(r->bucket++; r->bucket < set.bucket_count; r->bucket++)
	{		
		for(r->bit=0; r->bit<sizeof(bucket_t)*8; r->bit++)
		{
			bucket_t b = set.buckets[r->bucket] >> r->bit;
			if(b & 1) return;			
		}
	}
	r->end = true;
}

// Comprueba si un iterador ya rebaso el final del conjunto
bool bitset_end(bitset_iterator_t r)
{
	return r.end;
}

// Representa un Non-Deterministic Finite Automata
typedef struct _nfa_t
{
	bitset_t initials;
	bitset_t finals;
	bitset_t forward;
	int bucket_count;
	int states;
	int symbols;
} nfa_t;

nfa_t out_nfa;
nfa_t best_nfa;
nfa_t test_nfa;

// Obtiene los estados en el automata
state_t nfa_get_states(nfa_t nfa)
{
	return nfa.states;
}

// Obtiene el maximo numero de estados que puede contener la
// representacion del automata
state_t nfa_get_max_states(nfa_t nfa)
{
	return (1 << (sizeof(nfa.bucket_count)*8))  -  2;
}

// Obtiene el numero de simbolos asociado al alfabeto del automata
symbol_t nfa_get_symbols(nfa_t nfa)
{
	return nfa.symbols;
}

// Obtiene el conjunto de sucesores de un par estado-simbolo de un automata
void nfa_get_sucessors(nfa_t nfa, state_t state, symbol_t sym, bitset_t* bs)
{
	bs->bucket_count = nfa.bucket_count;
	size_t offset = (state * nfa_get_symbols(nfa) + sym)*bs->bucket_count;
	bs->buckets = nfa.forward.buckets[offset];
}

// Comprueba si el automata reconoce la secuencia suministrada
bool accept_sample(nfa_t nfa, const symbol_t* sample, size_t length)
{
	bitset_t* next;
	bitset_t* current;
	bitset_t tmp;
	
	for(size_t i=0; i<length; i++)
	{
		symbol_t sym = *sample++;
		bitset_clear(next);
		bool any = false;
		bitset_iterator_t j;
		for(j=bitset_first(current); !bitset_end(j); bitset_next(current, &j))
		{
			bitset_remove(current, j);
			state_t state = bitset_bit(j);
			nfa_get_sucessors(&nfa, state, sym, &tmp);
			bitset_union(next, &tmp);
			any = true;
		}
		if(!any) return false;
		swap(&next, &current);
	}

	bitset_intersect(current, &nfa.finals);
	return bitset_any(current);	
}

// Agrega estados para asegurar que el automata puede reconocer
// la secuencia suministrada
void coerce_match_sample(nfa_t* nfa, const sample_t* sample, size_t length)
{

}

// Realiza todas las mezclas de estados posibles
// TODO: Mejorar esta descripcion
void do_all_merges(nfa_t* nfa)
{
}

// Algoritmo que obtiene un automata NFA que puede reconocer un conjunto de
// secuencias y rechazar otro.
void oil(const sample_t* sample_buffer, 
		 const size_t sample_buffer_size,
		 const size_t sample_length,
		 const index_t* pindices, size_t ip_size, 
		 const index_t* nindices, size_t in_size
		 ) 
{
	for(size_t i=0; i<ip_size; i++)
	{
		const index_t pidx = pindices[i];
		if(!accept_sample(out_nfa, &sample_buffer[pidx], sample_length))
		{
			coerce_match_sample(&out_nfa, &sample_buffer[pidx], sample_length);
			do_all_merges(&out_nfa);
		}
	}
}

int main()
{
	return 0;
}
