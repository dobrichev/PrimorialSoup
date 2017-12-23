#ifndef BITMASK768_H_INCLUDED

#define BITMASK768_H_INCLUDED

#include <stdlib.h>
#include <limits.h>
#include <assert.h>
#include <immintrin.h>
#include <cmath>
#include <new>
#include <algorithm>
#include <cstring>

//#define NO_AVX

typedef union alignas(32) {
#ifndef NO_AVX
	__m256i b256;
#endif
	__m128i b128[2];
	uint64_t u64[4];
	uint16_t u16[16];
    static void* operator new(std::size_t sz) {
        void* p(NULL);
        if(posix_memalign(&p, 32, sz)) return NULL;
        std::memset(p, 0, sz);
        return p;
    }
    static void* operator new[](std::size_t sz) {
    	return operator new(sz);
    }
    static void operator delete(void* ptr) {
    	free(ptr);
    }
    static void operator delete[](void* ptr) {
    	free(ptr);
    }
} bm256;

struct bitMap {
	size_t numElements;
	size_t numWords;
	bm256* words;
	bitMap() {
		numElements = 0;
		numWords = 0;
		words = NULL;
	}
	bitMap(const bitMap& old) {
		numElements = old.numElements;
		numWords = old.numWords;
		words = (bm256*)bm256::operator new(numWords * sizeof(bm256));
		for(size_t i = 0; i < numWords; i++) {
			this->words[i] = old.words[i];
		}
	}
	bitMap(size_t size) {
		numElements = size;
		numWords = (numElements + 255) / 256;
		//words = new bm256[numWords];
		words = (bm256*)bm256::operator new(numWords * sizeof(bm256));
	}
	~bitMap() {
		if(words != NULL) {
			//delete[] words;
			bm256::operator delete[](words);
		}
	}
	void size(size_t size) {
		if(numElements) return;
		numElements = size;
		numWords = (numElements + 255) / 256;
		//words = new bm256[numWords];
		words = (bm256*)bm256::operator new(numWords * sizeof(bm256));
	}
	size_t size() const {
		return numElements;
	}
	size_t count() const {
		size_t n = 0;
		for(size_t i = 0; i < (numWords << 2); i++) {
			if(words[0].u64[i]) {
				n += __builtin_popcountl(words[0].u64[i]);
			}
		}
		return n;
	}
	size_t intersectionCount(const bitMap& other) const {
		//if(this->numElements != other.numElements) exit(2); // does a side effect of this the cache prefetch?
		size_t n = 0;
		for(size_t i = 0; i < (numWords << 2); i++) {
			//if(other.words[0].u64[i] & words[0].u64[i]) {
				n += __builtin_popcountl(other.words[0].u64[i] & words[0].u64[i]);
				//n++;
			//}
		}
		return n;
	}
	size_t intersectionCount(const bitMap& other, const size_t stopAbove) const {
		//if(this->numElements != other.numElements) exit(2); // does a side effect of this the cache prefetch?
		size_t n = 0;
		for(size_t i = 0; i < (numWords << 2); i++) {
			//if(other.words[0].u64[i] & words[0].u64[i]) {
				n += __builtin_popcountl(other.words[0].u64[i] & words[0].u64[i]);
				if(n >= stopAbove) break;
			//}
		}
		return n;
	}
	bool operator[](size_t i) const {
		size_t w64 = i >> 6;
		size_t o = i & 63;
		return words[0].u64[w64] & ((uint64_t)1 << o);
	}
	bitMap& operator=(const bitMap& other) {
//		if(this->numElements != other.numElements) exit(3);
//		std::memcpy(this->words, other.words, 32 * this->numElements);
#ifdef NO_AVX
		for(size_t i = 0; i < (numWords << 1); i++) {
			words[0].b128[i] = other.words[0].b128[i];
		}
#else
		for(size_t i = 0; i < numWords; i++) {
			words[i].b256 = other.words[i].b256;
		}
#endif
		return *this;
	}
	bitMap& operator&=(const bitMap& other) {
                //if(this->numElements != other.numElements) exit(4);
#ifdef NO_AVX
		for(size_t i = 0; i < (numWords << 1); i++) {
			words[0].b128[i] = _mm_and_si128(words[0].b128[i], other.words[0].b128[i]);
		}
#else
		for(size_t i = 0; i < numWords; i++) {
			words[i].b256 = _mm256_castpd_si256(_mm256_and_pd(_mm256_castsi256_pd(words[i].b256), _mm256_castsi256_pd(other.words[i].b256)));
		}
#endif
		return *this;
	}
	bitMap& clearBits(const bitMap& other) {
                if(this->numElements != other.numElements) exit(5);  //prefetch??
#ifdef NO_AVX
		for(size_t i = 0; i < (numWords << 1); i++) {
			words[0].b128[i] = _mm_andnot_si128(other.words[0].b128[i], words[0].b128[i]);
		}
#else
		for(size_t i = 0; i < numWords; i++) {
			//if(_mm256_testz_si256(words[i].b256, words[i].b256)) continue; //already zero
			words[i].b256 = _mm256_castpd_si256(_mm256_andnot_pd(_mm256_castsi256_pd(other.words[i].b256), _mm256_castsi256_pd(words[i].b256)));
		}
#endif
		return *this;
	}
	void set(size_t i) {
//		size_t w256 = i >> 8;
//		size_t w64 = (i >> 6) & 3;
//		size_t o = i & 63;
//		words[w256].u64[w64] |= ((uint64_t)1 << o);
		size_t w64 = i >> 6;
		size_t o = i & 63;
		words[0].u64[w64] |= ((uint64_t)1 << o);
	}
	void set() {
		//set all words but the final one
#ifdef NO_AVX
		register __m128i x = _mm_undefined_si128();
		register __m128i all1 = _mm_cmpeq_epi32(x, x);
		for(size_t i = 0; i < (numWords << 1); i++) {
			words[0].b128[i] = all1;
		}
#else
		register __m256d x = _mm256_undefined_pd ();
		register __m256i all1 = _mm256_castpd_si256(_mm256_cmp_pd(x, x, _CMP_EQ_UQ));
		for(size_t i = 0; i < numWords; i++) {
			words[i].b256 = all1;
		}
#endif
		//set the final word, possibly partially
		if(numElements & 192) { //bits 6,7
			//(3,2,1,0)={?,-1,-1,-1},{-1,-1,-1,-1}...{-1,-1,-1,-1}
			words[numWords - 1].u64[3] = (((size_t)1) << (numElements & 63)) - 1;
		}
		else if(numElements & 48) {
			//(3,2,1,0)={0,?,-1,-1},{-1,-1,-1,-1}...{-1,-1,-1,-1}
			words[numWords - 1].u64[3] = 0;
			words[numWords - 1].u64[2] = (((size_t)1) << (numElements & 63)) - 1;
		}
		else if(numElements & 12) {
			//(3,2,1,0)={0,0,?,-1},{-1,-1,-1,-1}...{-1,-1,-1,-1}
			words[numWords - 1].u64[3] = words[numWords - 1].u64[2] = 0;
			words[numWords - 1].u64[1] = (((size_t)1) << (numElements & 63)) - 1;
		}
		else if(numElements & 3) {
			//(3,2,1,0)={0,0,0,?},{-1,-1,-1,-1}...{-1,-1,-1,-1}
			words[numWords - 1].u64[3] = words[numWords - 1].u64[2] = words[numWords - 1].u64[1] = 0;
			words[numWords - 1].u64[3] = (((size_t)1) << (numElements & 63)) - 1;
		}
	}
	void reset(size_t i) {
//		size_t w256 = i >> 8;
//		size_t w64 = (i >> 6) & 3;
//		size_t o = i & 63;
//		words[w256].u64[w64] &= ~((uint64_t)1 << o);
		size_t w64 = i >> 6;
		size_t o = i & 63;
		words[0].u64[w64] &= ~((uint64_t)1 << o);
	}
	void reset() {
#ifdef NO_AVX
		for(size_t i = 0; i < (numWords << 1); i++) {
			words[0].b128[i] = _mm_setzero_si128();
		}
#else
		for(size_t i = 0; i < numWords; i++) {
			words[i].b256 = _mm256_setzero_si256();
		}
#endif
	}
	inline size_t first() const {
		for(size_t i = 0; i < (numWords << 2); i++) {
			if(words[0].u64[i]) {
				return std::min(i * 64 + __builtin_ctzll(words[0].u64[i]), numElements);
			}
		}
		return numElements;
	}
	bool clearsAll(const bitMap& other) const {
//		if(this->numElements != other.numElements) exit(6);
#ifdef NO_AVX
		for(size_t w = 0; w < (numWords << 2); w++) {
			if(words[0].u64[w] & other.words[0].u64[w]) return false;
		}
#else
		for(size_t w = 0; w < numWords; w++) {
			if(!_mm256_testz_si256(words[w].b256, other.words[w].b256)) return false;
		}
#endif
		return true;
	}
	inline size_t next(const size_t e) const {
		size_t i = e + 1;
		if(i >= numElements) return numElements;
		size_t w64 = (i >> 6);
		size_t o = i & 63;
		uint64_t w0 = words[0].u64[w64];
		w0 &= (((uint64_t)-1) << o);
		if(w0) {
			return std::min(w64 * 64 + __builtin_ctzll(w0), numElements);
		}
		for(size_t i = w64 + 1; i < (numWords << 2); i++) {
			if(words[0].u64[i]) {
				return std::min(i * 64 + __builtin_ctzll(words[0].u64[i]), numElements);
			}
		}
		return numElements;
	}
//	bitMap& hasNoneOf(const bitMap* otherBitmaps, const bitMap& indices) {
//		for(size_t i = indices.first(); i < indices.numElements; i = indices.next(i)) {
//			this->clearBits(otherBitmaps[i]);
//		}
//		return *this;
//	}
	bitMap& hasNoneOf(const bitMap* otherBitmaps, const size_t* indices) {
//		if(this->numElements != otherBitmaps[0].numElements) {
//			std::cerr << "Sizes are (" << this->numElements << "," << otherBitmaps[0].numElements << ")\n";
//		exit(7);
//                }
#ifdef NO_AVX
		for(size_t i = 0; indices[i] != (size_t)-1; i++) {
			this->clearBits(otherBitmaps[indices[i]]);
		}
#else
		for(size_t w = 0; w < numWords; w++) {
			//if(_mm256_testz_si256(words[w].b256, words[w].b256)) continue; //already zero
			__m256d x = _mm256_castsi256_pd(words[w].b256);
			for(size_t i = 0; indices[i] != (size_t)-1; i++) {
				x = _mm256_andnot_pd(_mm256_castsi256_pd(otherBitmaps[indices[i]].words[w].b256), x);
			}
			words[w].b256 = _mm256_castpd_si256(x);
		}
#endif
		return *this;
	}
//	bitMap& hasExactlyOneOf(const bitMap* otherBitmaps, const bitMap& indices) {
//		bitMap singles(this->numElements); //has one or more of the indices
//		bitMap duplicates(this->numElements); //has two or more of the indices
//		for(size_t i = indices.first(); i < indices.numElements; i = indices.next(i)) {
//#ifdef NO_AVX
//			for(size_t w = 0; w < (numWords << 1); w++) {
//				duplicates.words[0].b128[w] = _mm_or_si128(duplicates.words[0].b128[w], _mm_and_si128(otherBitmaps[i].words[0].b128[w], singles.words[0].b128[w]));
//				singles.words[0].b128[w] = _mm_or_si128(singles.words[0].b128[w], otherBitmaps[i].words[0].b128[w]);
//			}
//#else
//			for(size_t w = 0; w < numWords; w++) {
//				duplicates.words[w].b256 = _mm256_castpd_si256(_mm256_or_pd(_mm256_castsi256_pd(duplicates.words[w].b256), _mm256_and_pd(_mm256_castsi256_pd(otherBitmaps[i].words[w].b256), _mm256_castsi256_pd(singles.words[w].b256))));
//				singles.words[w].b256 = _mm256_castpd_si256(_mm256_or_pd(_mm256_castsi256_pd(singles.words[w].b256), _mm256_castsi256_pd(otherBitmaps[i].words[w].b256)));
//			}
//#endif
//		}
//		this->clearBits(duplicates); //exclude if it has more than one of otherBitmaps[i]
//		*this &= singles; //exclude if it has none of otherBitmaps[i]
//		return *this;
//	}
	bitMap& hasExactlyOneOf(const bitMap* otherBitmaps, const size_t* indices) {
//		if(this->numElements != otherBitmaps->numElements) exit(8);
#ifdef NO_AVX
		for(size_t w = 0; w < (numWords << 1); w++) {
			__m128i singles = _mm_setzero_si128();
			__m128i duplicates = _mm_setzero_si128();
			for(size_t i = 0; indices[i] != (size_t)-1; i++) {
				duplicates = _mm_or_si128(duplicates, _mm_and_si128(otherBitmaps[indices[i]].words[0].b128[w], singles));
				singles = _mm_or_si128(singles, otherBitmaps[indices[i]].words[0].b128[w]);
			}
			words[0].b128[w] = _mm_andnot_si128(duplicates, words[0].b128[w]);
			words[0].b128[w] = _mm_and_si128(singles, words[0].b128[w]);
		}
#else
		for(size_t w = 0; w < numWords; w++) {
			//if(_mm256_testz_si256(words[w].b256, words[w].b256)) continue; //already zero
			__m256d singles = _mm256_setzero_pd();
			__m256d duplicates = _mm256_setzero_pd();
			for(size_t i = 0; indices[i] != (size_t)-1; i++) {
				duplicates = _mm256_or_pd(duplicates, _mm256_and_pd(_mm256_castsi256_pd(otherBitmaps[indices[i]].words[w].b256), singles));
				singles = _mm256_or_pd(singles, _mm256_castsi256_pd(otherBitmaps[indices[i]].words[w].b256));
			}
			words[w].b256 = _mm256_castpd_si256(_mm256_andnot_pd(duplicates, _mm256_castsi256_pd(words[w].b256)));
			words[w].b256 = _mm256_castpd_si256(_mm256_and_pd(singles, _mm256_castsi256_pd(words[w].b256)));
		}
#endif
		return *this;
	}
//	bitMap& hasMaxOneOf(const bitMap* otherBitmaps, const bitMap& indices) {
//		bitMap singles(this->numElements); //has one or more of the indices
//		bitMap duplicates(this->numElements); //has two or more of the indices
//		for(size_t i = indices.first(); i < indices.numElements; i = indices.next(i)) {
//			for(size_t w = 0; w < numWords; w++) {
//				duplicates.words[w].b256 = _mm256_castpd_si256(_mm256_or_pd(_mm256_castsi256_pd(duplicates.words[w].b256), _mm256_and_pd(_mm256_castsi256_pd(otherBitmaps[i].words[w].b256), _mm256_castsi256_pd(singles.words[w].b256))));
//				singles.words[w].b256 = _mm256_castpd_si256(_mm256_or_pd(_mm256_castsi256_pd(singles.words[w].b256), _mm256_castsi256_pd(otherBitmaps[i].words[w].b256)));
//			}
//		}
//		this->clearBits(duplicates); //exclude if it has more than one of otherBitmaps[i]
//		return *this;
//	}
};
struct bitMaps {
	size_t numMaps;
	size_t numElements;
	bitMap* maps;
	bitMaps(size_t nMaps, size_t nElements) {
		numMaps = nMaps;
		numElements = nElements;
		maps = new bitMap[numMaps];
		for(size_t i = 0; i < numMaps; i++) {
			maps[i].size(numElements);
		}
	}
	~bitMaps() {
//		for(size_t i = 0; i < numMaps; i++) {
//			maps[i].~bitMap();
//		}
		delete[] maps;
	}
};

#endif //BITMASK768_H_INCLUDED
