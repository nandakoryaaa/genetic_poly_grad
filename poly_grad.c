#define SDL_MAIN_HANDLED

#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

#define MAX_IMG_SIZE 1024
#define MAX_CHROMO_SIZE 1024
#define MAX_BLOCKS 32 * 32
#define BLOCK_SIZE 32

#define QUEUE_SIZE 64

typedef struct {
	unsigned char* addr;
	int width;
	unsigned char start_r;
	unsigned char start_g;
	unsigned char start_b;
	unsigned char start_alpha;
	unsigned char end_r;
	unsigned char end_g;
	unsigned char end_b;
	unsigned char end_alpha;
} Workload;

typedef struct ThreadState {
	pthread_t thread_id;
	int cnt;
	volatile int head;
	volatile int tail;
	Workload workbuffer[QUEUE_SIZE];
} ThreadState;

static volatile int working = 1;
static volatile int work_cnt = 0;
static int thread_pos = 0;

ThreadState thread_state1;
ThreadState thread_state2;
ThreadState thread_state3;
ThreadState thread_state4;

static ThreadState* threads[4] = {
	&thread_state1, &thread_state2, &thread_state3, &thread_state4
};

int event_filter_run[1] = { SDL_KEYDOWN };
int event_filter_mask[4] = { SDL_KEYDOWN, SDL_MOUSEMOTION, SDL_MOUSEBUTTONDOWN, SDL_MOUSEBUTTONUP };

void queue(Workload w)
{
	while (1) {
		if (thread_pos > 3) {
			thread_pos = 0;
		}
		ThreadState* st = threads[thread_pos++];

		int next_head = (st->head + 1) & (QUEUE_SIZE - 1);

		if (next_head != st->tail) {
			st->workbuffer[st->head] = w;
			st->head = next_head;
			break;
		}
	}
}

#include "inc/rnd.c"
#include "inc/types.c"
#include "inc/settings.c"
#include "inc/draw.c"
#include "inc/mutations.c"
#include "inc/storage.c"
#include "inc/controllers.c"

int main(int argc, char* argv[])
{
	char window_title[128];
	unsigned long long thr = 0;
	SDL_Event event;
	Poly *poly;
	Chromo chromo;
	Chromo test_chromo;
	int result;
	Settings settings;
	State state;

	if (argc < 2) {
		printf("Usage: %s image.bmp\n", argv[0]);
		return 1;
	}

	state.settings = &settings;
	if (!parse_params(argc, argv, &state)) {
		return 1;
	}

	if (!state.filename) {
		puts("No filename specified\n");
		return 1;
	}
	
	SDL_Surface *bmpSurface = SDL_LoadBMP(state.filename);
	if (!bmpSurface) {
		printf("Error: %s\n", SDL_GetError());
		return 1;
	}

	if (bmpSurface->w > 1024 || bmpSurface->h > 1024 || bmpSurface->w < 32 || bmpSurface->h < 32) {
		printf("Image dimensions must be 32..1024 pixels\n");
		return 1;
	}

	settings.screenWidth = bmpSurface->w;
	settings.screenHeight = bmpSurface->h;
	if (settings.maxPolygons == 0) {
		settings.maxPolygons = (settings.screenWidth * settings.screenHeight * 256) / (400 * 400);
		if (settings.maxPolygons > MAX_CHROMO_SIZE) {
			settings.maxPolygons = MAX_CHROMO_SIZE;
		} else if (settings.maxPolygons < 1) {
			settings.maxPolygons = 1;
		}
		printf("maxPolys set to %d\n", settings.maxPolygons);
	}
	if (settings.minPolygons > settings.maxPolygons) {
		settings.minPolygons = settings.maxPolygons;
		printf("minPolys set to %d\n", settings.minPolygons);
	}


	SDL_Init(SDL_INIT_VIDEO);

	SDL_Window *window = SDL_CreateWindow(
		state.filename,
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		settings.screenWidth,
		settings.screenHeight,
		SDL_WINDOW_ALLOW_HIGHDPI
	);

	if (window == NULL) {
		printf("Could not create window: %s\n", SDL_GetError());
		return 1;
	}

	SDL_Surface *screenSurface = SDL_GetWindowSurface(window);
	SDL_Surface *imgSurface = SDL_ConvertSurface(bmpSurface, screenSurface->format, 0);
	SDL_Surface *chromoSurface = SDL_ConvertSurface(bmpSurface, screenSurface->format, 0);

	if (!imgSurface) {
		puts("Could not convert surface\n");
		return 1;
	}
	SDL_FreeSurface(bmpSurface);

	state.imgSurf = imgSurface;
	state.chromoSurf = chromoSurface;
	state.screenSurf = screenSurface;
	state.rect.x = 0;
	state.rect.y = 0;
	state.rect.w = settings.screenWidth,
	state.rect.h = settings.screenHeight;
	state.cnt = 0;
	state.start_time = time(0);
	state.chromo = &chromo;
	state.test_chromo = &test_chromo;

	if (!state.seed) {
		state.seed = time(0);
	}

	printf("seed: %d\n", state.seed);
	rnd_seed(state.seed);

	memset(&state.masked_blocks, 0, sizeof(state.masked_blocks));

	if (state.load_chromo) {
		if (!load_chromo(&state, state.load_chromo)) {
			printf("Could not load chromo\n");
			return 1;
		}
	} else {
		if (!init_chromo(&chromo, &state)) {
			printf("Could not init chromo\n");
			return 1;
		}
	}

	pthread_create(&thread_state1.thread_id, NULL, draw_line, &thread_state1);
	pthread_create(&thread_state2.thread_id, NULL, draw_line, &thread_state2);
	pthread_create(&thread_state3.thread_id, NULL, draw_line, &thread_state3);
	pthread_create(&thread_state4.thread_id, NULL, draw_line, &thread_state4);
	draw_chromo(chromoSurface, &chromo, &settings);
	chromo.start_rating = calc_rating(&chromo, &state);

	if (settings.mask) {
		set_mode(&state, MODE_MASK, event_filter_mask, sizeof(event_filter_mask));
	} else {
		set_mode(&state, MODE_RUN, event_filter_run, 1);
	}

	while(1) {
		state.update = 0;
		state.cnt++;
		if (state.cnt % 1000 == 0) {
			sprintf(window_title, "%d : %.2fM", chromo.count, (float) chromo.rating / 1000000);
			SDL_SetWindowTitle(window, (const char*) &window_title);
		}

		result = SDL_PollEvent(&event);
		if (result && event.type == SDL_QUIT) {
			break;
		}

		if (state.mode == MODE_RUN) {
			default_controller(&state, &event);
		} else {
			if (result) {
				for (int i = 0; i < state.event_filter_count; i++) {
					if (state.event_filter[i] == event.type) {
						if (state.mode == MODE_MASK) {
							mask_controller(&state, &event);
						} else if (state.mode == MODE_ORIGIN) {
							origin_controller(&state, &event);
						}
						break;
					}
				}
			}
		}

		if (state.update) {
			SDL_UpdateWindowSurface(window);
		}
	}

	working = 0;

	pthread_join(thread_state1.thread_id, NULL);
	pthread_join(thread_state2.thread_id, NULL);
	pthread_join(thread_state3.thread_id, NULL);
	pthread_join(thread_state4.thread_id, NULL);


	SDL_FreeSurface(screenSurface);
	SDL_DestroyWindow(window);
	SDL_Quit();

	printf("Finished %s with poly count=%d, iterations=%d, selected=%d, rating=%llu\n", state.filename, chromo.count, state.cnt, state.selected_cnt, chromo.rating);
	for (int i = 0; i < argc; i++) {
		if (!strcmp("-seed", argv[i])) {
			i++;
			continue;
		}
		printf("%s ", argv[i]);
	}
	printf("-seed %d\n", state.seed);

	//free(chromo.genes);

	return 0;
}
