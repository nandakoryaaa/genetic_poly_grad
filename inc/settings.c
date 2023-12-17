int parse_params(int argc, char **argv, State *state)
{
	for (int i = 1; i < argc; i++) {
		char *param = argv[i];
		if (*param == '-') {
			param++;
			if (!strcmp(param, "flatcolor")) {
				state->settings->flatColor = 1;
			} else if (!strcmp(param, "flatalpha")) {
				state->settings->flatAlpha = 1;
			} else if (!strcmp(param, "initgray")) {
				state->settings->initGray = 1;
			} else if (!strcmp(param, "mask")) {
				state->settings->mask = 1;
			} else if (!strcmp(param, "maxalpha") && i < argc - 1) {
				unsigned char alpha = atoi(argv[++i]);
				if (alpha < 1 || alpha > 255) {
					alpha = 255;
				}
				state->settings->maxAlpha = alpha;
			} else if (!strcmp(param, "minalpha") && i < argc - 1) {
				unsigned char alpha = atoi(argv[++i]);
				if (alpha > 255) {
					alpha = 0;
				}
				state->settings->minAlpha = alpha;
			} else if (!strcmp(param, "growrate")) {
				state->settings->growRate = atoi(argv[++i]);
			} else if (!strcmp(param, "minpolys")) {
				state->settings->minPolygons = atoi(argv[++i]);
				if (state->settings->minPolygons < 1) {
					state->settings->minPolygons = 1;
				}
			} else if (!strcmp(param, "maxpolys")) {
				state->settings->maxPolygons = atoi(argv[++i]);
				if (state->settings->maxPolygons < 1) {
					state->settings->maxPolygons = 1;
				}
				if (state->settings->maxPolygons > MAX_CHROMO_SIZE) {
					state->settings->maxPolygons = MAX_CHROMO_SIZE;
				}
			} else if (!strcmp(param, "seed") && i < argc - 1) {
				state->seed = atoi(argv[++i]);
			} else if (!strcmp(param, "overdrive") && i < argc - 1) {
				state->settings->overdrive = atoi(argv[++i]);
			} else if (!strcmp(param, "load") && i < argc - 1) {
				state->load_chromo = argv[++i];
			} else {
				printf("Unknown param: %s\n", param);
				return 0;
			}
		} else {
			state->filename = argv[i];
		}
	}

	printf("chromo size: %d\n", state->settings->maxPolygons);
	printf("min polys: %d\n", state->settings->minPolygons);
	printf("flat color: %d\n", state->settings->flatColor);
	printf("flat alpha: %d\n", state->settings->flatAlpha);
	printf("init gray: %d\n", state->settings->initGray);
	printf("min alpha: %d\n", state->settings->minAlpha);
	printf("max alpha: %d\n", state->settings->maxAlpha);
	printf("grow rate: %d\n", state->settings->growRate);
	printf("overdrive: %d\n", state->settings->overdrive);

	return 1;
}
