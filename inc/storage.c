void save_bmp(State *state)
{
	char filename[] = "img_0000000000.bmp";
	sprintf(filename + 4, "%010d.bmp", state->cnt);
	draw_chromo(state->chromoSurf, state->chromo, state->settings);
	if (!SDL_SaveBMP(state->chromoSurf, filename)) {
		printf("saved image %s\n", filename);
	} else {
		printf("could not save image %s\n", filename);
	}
}

void save_chromo(State* state)
{
	char filename[] = "chromo_0000000000";
	sprintf(filename + 7, "%010d", state->cnt);
	FILE *fp = fopen(filename, "wb");
	if (!fp) {
		printf("could not create file %s\n", filename);
		return;
	}

	fwrite(&state->settings->screenWidth, sizeof(short int), 1, fp);
	fwrite(&state->settings->screenHeight, sizeof(short int), 1, fp);
	fwrite(&state->chromo->count, sizeof(short int), 1, fp);
	short int size = sizeof(Poly);
	fwrite(&size, sizeof(short int), 1, fp);
	fwrite(state->chromo->genes, sizeof(Poly) * state->chromo->count, 1, fp);

	char buf[MAX_BLOCKS / 8];
	memset(buf, 0, sizeof(buf));
	for (int i = 0; i < MAX_BLOCKS; i++) {
		buf[i >> 3] |= (state->masked_blocks[i] << (i & 7));
	}

	fwrite(buf, sizeof(buf), 1, fp);
	fclose(fp);
	printf("saved chromo %s\n", filename);
}

int load_chromo(State *state, const char *filename)
{
	FILE *fp = fopen(filename, "rb");
	if (!fp) {
		printf("could not load %s\n", filename);
		return 0;
	}

	short int count, size, w, h;

	fread(&w, sizeof(short int), 1, fp);
	fread(&h, sizeof(short int), 1, fp);

	if (w != state->settings->screenWidth || h != state->settings->screenHeight) {
		printf("saved dimensions don't match: %d x %d\n", w, h);
		return 0;
	}

	fread(&count, sizeof(short int), 1, fp);
	fread(&size, sizeof(short int), 1, fp);

	if (size != sizeof(Poly) || count > MAX_CHROMO_SIZE) {
		printf("wrong save format: cnt=%d, size=%d, expected max cnt=%d, size=%d\n", count, size, MAX_CHROMO_SIZE, sizeof(Poly));
		return 0;
	}
		
	state->chromo->count = count;
	fread(&state->chromo->genes, size * count, 1, fp);

	char buf[MAX_BLOCKS / 8];
	fread(buf, sizeof(buf), 1, fp);
	for (int i = 0; i < MAX_BLOCKS; i++) {
		state->masked_blocks[i] = (buf[i >> 3] >> (i & 7)) & 1;
	}

	fclose(fp);

	return 1;
}
