// bitset.c

// Este archivo hace parte de la implementacion del algoritmo OIL usando
// lenguaje C con el fin de ser sintetizable en hardware.
// Este archivo contiene la implementacion de un conjunto de bits
// OIL es un algoritmo publicado por vez primera en P. Garcia, M. 
// Vazquez de Parga, G. I. Alvarez, and J. Ruiz, "Universal automata 
// and NFA learning," Theoretical Computer Science, vol. 407, no. 1–3,
// pp. 192–202, Nov. 2008. [http://dx.doi.org/10.1016/j.tcs.2008.05.017]

// 2014, Jairo Andres Velasco R, [jairov_at_javerianacali.edu.co]
// Grupo de investigacion DESTINO
// Pontificia Universidad Javeriana Cali
#include "util.h"
#include "bitset.h"
#include <assert.h>

void _conformance_check_bitset(void)
{
	const size_t MAX_TOTAL_BITS = BITS_OF_TYPE(bucket_t) * MAX_BUCKETS;
	assert(MAX_BUCKETS <= MAX_OF_TYPE(bucket_index_t));
	assert(BITS_OF_TYPE(bucket_t) - 1 <= MAX_OF_TYPE(bucket_bit_index_t));
	assert(MAX_TOTAL_BITS <= MAX_OF_TYPE(bitset_element_index_t));
}

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

