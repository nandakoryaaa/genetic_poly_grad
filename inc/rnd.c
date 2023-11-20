unsigned long int xoshiro_s[4];

static inline unsigned long int rotl(const unsigned long int x, int k)
{
	return (x << k) | (x >> (32 - k));
}

void xoshiro_seed(unsigned long int *s)
{
	s[0] = (rand() << 16) + rand();
	s[1] = (rand() << 16) + rand();
	s[2] = (rand() << 16) + rand();
	s[3] = (rand() << 16) + rand();
}

unsigned long int xoshiro_next(unsigned long int *s)
{
	const unsigned long int result = rotl(s[1] * 5, 7) * 9;
	const unsigned long int t = s[1] << 9;

	s[2] ^= s[0];
	s[3] ^= s[1];
	s[1] ^= s[2];
	s[0] ^= s[3];

	s[2] ^= t;
	s[3] = rotl(s[3], 11);

	return result;
}

void rnd_seed(int seed)
{
	srand(seed);
	xoshiro_seed(xoshiro_s);
}

int rnd(int range)
{
	int t = (unsigned long long) xoshiro_next(xoshiro_s) * range / 0xffffffff;
	return t;
}

int rnds(int range)
{
	return rnd(range + range + 1) - range;
}

// Chance is for example 300, meaning each 300th number of unsigned long will match
// there
int rndc(int chance)
{
	//return (xoshiro_next(xoshiro_s) % chance) == 1;
	return xoshiro_next(xoshiro_s) < 0xffffffff / chance;
}

int rndi(int min, int max)
{
	return min + rnd(max + 1 - min);
}
