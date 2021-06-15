void set_mode(State *state, Mode mode, int event_filter[], int filter_count) {
	state->mode = mode;
	state->event_filter = event_filter;
	state->event_filter_count = filter_count;
}

void draw_masked_blocks(State *state) {
	SDL_Rect rect;
	int block_index = 0;
	SDL_LowerBlit(state->imgSurf, &state->rect, state->screenSurf, &state->rect);
	for (int y = 0; y < state->blocks_h; y++) {
		for (int x = 0; x < state->blocks_w; x++) {
			if (state->masked_blocks[block_index]) {
				get_block_rect(state, block_index, &rect);
				if (rect.w > 2) {
					rect.x++;
					rect.w -= 2;
				}
				if (rect.h > 2) {
					rect.y++;
					rect.h -= 2;
				}
				draw_rect(state->screenSurf, &rect, 0xffff00);
			//	bres_line(state->screenSurf, rect.x, rect.y, rect.x + rect.w - 1, rect.y + rect.h - 1, 0xffff00);
			//	bres_line(state->screenSurf, rect.x, rect.y + rect.h - 1, rect.x + rect.w - 1, rect.y, 0xffff00);
			}
			block_index++;
		}
	}

	get_block_rect(state, state->selected_block, &rect);
	draw_rect(state->screenSurf, &rect, 0xffff00);

	state->update = 1;
}

void draw_origin_polys(State *state, int color) {
	SDL_LowerBlit(state->imgSurf, &state->rect, state->screenSurf, &state->rect);
	for (int i = 0; i < state->chromo->count; i++) {
		Poly *poly = &state->chromo->genes[i];
		bres_line(state->screenSurf, poly->origin_v0.x, poly->origin_v0.y, poly->origin_v1.x, poly->origin_v1.y, color);
		bres_line(state->screenSurf, poly->origin_v1.x, poly->origin_v1.y, poly->origin_v2.x, poly->origin_v2.y, color);
		bres_line(state->screenSurf, poly->origin_v2.x, poly->origin_v2.y, poly->origin_v0.x, poly->origin_v0.y, color);
	}
	state->update = 1;
}

void print_chromo(Chromo *chromo) {
	printf("------------- chromo rating = %llu ---------------\n", chromo->rating);
	for (int i = 0; i < chromo->count; i++) {
		Poly *p = &chromo->genes[i];
		printf("%d: %d %d - %d %d - %d %d\n", i, p->v0.x, p->v0.y, p->v1.x, p->v1.y, p->v2.x, p->v2.y);
	}
	printf("dirty rect: %d %d - %d %d\n", chromo->dirty_rect.x0, chromo->dirty_rect.y0, chromo->dirty_rect.x1, chromo->dirty_rect.y1);
}

unsigned long long print_blocks(Chromo *chromo, State *state) {
	unsigned long long sum = 0;
	for (int y = 0; y < state->blocks_h; y++) {
		for (int x = 0; x < state->blocks_w; x++) {
			printf("%9d ", chromo->blocks[y * state->blocks_w + x]);
			sum += chromo->blocks[y * state->blocks_w + x];
		}
		printf("\n");
	}
	printf("---------- sum: %llu\n", sum);
	return sum;
}

void mask_controller(State *state, SDL_Event *event) {
	Chromo *chromo = state->chromo;
	if (event->type == SDL_KEYDOWN) {
		if (event->key.keysym.sym == ' ') {
			set_mode(state, MODE_RUN, event_filter_run, sizeof(event_filter_run));
		}
	} else if (event->type == SDL_MOUSEMOTION) {
		int block_offset = (event->button.y / BLOCK_SIZE) * state->blocks_w + (event->button.x / BLOCK_SIZE);
		if (block_offset != state->selected_block) {
			state->selected_block = block_offset;
			draw_masked_blocks(state);
		}
	} else if (event->type == SDL_MOUSEBUTTONUP) {
		state->masked_blocks[state->selected_block] = 1 - state->masked_blocks[state->selected_block];
		draw_masked_blocks(state);
	}
}

void origin_controller(State *state, SDL_Event *event) {
	if (event->key.keysym.sym == 'o') {
		set_mode(state, MODE_RUN, event_filter_run, sizeof(event_filter_run));
	}
}

void default_controller(State *state, SDL_Event *event) {
	char m;

	process_chromo(state);

	if (event->type == SDL_KEYDOWN) {
		switch(event->key.keysym.sym) {
			case 'd':
				m = (state->showBounds << 1) + state->showPolys;
				m++;
				state->showBounds = (m >> 1) & 1;
				state->showPolys = m & 1;
				break;
			case 'b':
				print_blocks(state->chromo, state);
				break;
			case 'r':
				printf("rating reset: old=%llu", state->chromo->rating);
				reset_rating(state->chromo, state);
				draw_chromo(state->chromoSurf, state->chromo, state->settings);
				calc_rating(state->chromo, state);
				printf(" new=%llu\n", state->chromo->rating);
				break;
			case 'p':
				print_chromo(state->chromo);
				break;
			case ' ':
				set_mode(state, MODE_MASK, event_filter_mask, sizeof(event_filter_mask));
				draw_masked_blocks(state);
				break;
			case 'o':
				set_mode(state, MODE_ORIGIN, event_filter_run, sizeof(event_filter_run));
				draw_origin_polys(state, 0xffffff);
				break;
			case 's':
				save_chromo(state);
				break;
			case 'i':
				save_bmp(state);
				break;
			case 'w':
				unsigned int elapsed = time(0) - state->start_time;
				printf("%d: sel.%d cnt.%d rat.%llu scaled %d shifted %d wiggled %d %02d:%02d\n",
					state->cnt, state->selected_cnt, state->chromo->count, state->chromo->rating,
					state->scale_cnt, state->shift_cnt, state->pos_cnt, elapsed / 3600, (elapsed % 3600) / 60
				);
				break;
		}
	}
}
