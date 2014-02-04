#include <stdint.h>
#include <stdlib.h>
//#include <stdbool.h>

typedef size_t index_t;
typedef uint8_t symbol_t;
typedef uint32_t bucket_t;
typedef uint16_t state_t;

typedef struct _bitset_t
{
	bucket_t* buckets;
	size_t bucket_count;
} bitset_t;

typedef struct _bitset_iterator_t
{
	size_t bit;
	size_t bucket;
	bool end;
} bitset_iterator_t;

void bitset_clear(bitset_t* set)
{
	for(size_t i=0; i<set->bucket_count; i++)
	{
		set->buckets[i] = 0;
	}
}

void bitset_remove(bitset_t* set, size_t i)
{
	size_t bucket = i / (8*sizeof(bucket_t));
	size_t bit = i % (8*sizeof(bucket_t));
	set->buckets[bucket] &= ~(1 << bit);
}

void bitset_remove(bitset_t* set, bitset_iterator_t i)
{
	set->buckets[i.bucket] &= ~(1 << i.bit);
}

void bitset_add(bitset_t* set, size_t i)
{
	size_t bucket = i / (8*sizeof(bucket_t));
	size_t bit = i % (8*sizeof(bucket_t));
	set->buckets[bucket] |= (1 << bit);
}

bool bitset_contains(bitset_t* set, size_t i)
{
	size_t bucket = i / (8*sizeof(bucket_t));
	size_t bit = i % (8*sizeof(bucket_t));
	return (set->buckets[bucket] >> bit) & 1 ? true : false;
}

void bitset_union(bitset_t* ra, const bitset_t* b)
{
	for(size_t i=0; i < ra->bucket_count; i++)
	{
		ra->buckets[i] |= b->buckets[i];
	}
}

void bitset_intersect(bitset_t* ra, const bitset_t* b)
{
	for(size_t i=0; i < ra->bucket_count; i++)
	{
		ra->buckets[i] &= b->buckets[i];
	}
}

bool bitset_any(const bitset_t set)
{
	for(size_t i=0; i<set.bucket_count; i++)
	{
		if(set.buckets[i]) return true;
	}
	return false;
}

size_t bitset_bit(const bitset_iterator_t i)
{
	return i.bit + i.bucket*sizeof(bucket_t)*8;
}

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

bool bitset_end(bitset_iterator_t r)
{
	return r.end;
}

typedef struct _nfa_t
{
	bitset_t initials;
	bitset_t finals;
	bitset_t forward;
	int states_bits;
	int symbols_bits;
} nfa_t;

nfa_t out_nfa;
nfa_t best_nfa;
nfa_t test_nfa;

state_t nfa_get_states(nfa_t nfa)
{
	return 1 << nfa.states_bits;
}

symbol_t nfa_get_symbols(nfa_t nfa)
{
	return 1 << nfa.symbols_bits;
}

void nfa_get_sucessors(nfa_t nfa, state_t state, symbol_t sym, bitset_t* bs)
{
	bs->buckets = nfa.forward.buckets[]
	bs->bucket_count = nfa_get_states(nfa) / (sizeof(bucket_t)*8);
}

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

void coerce_match_sample(nfa_t* nfa, const sample_t* sample, size_t length)
{

}

void do_all_merges(nfa_t* nfa)
{
}

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