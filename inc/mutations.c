void update_dirty_rect(Chromo *chromo, Poly *poly) {
	// poly must be normalized
	DirtyRect *rect = &chromo->dirty_rect;

	if (!rect->dirty) {
		rect->x0 = poly->v0.x;
		rect->y0 = poly->v0.y; // set from upper poly vertex
		rect->x1 = poly->v2.x;
		rect->y1 = poly->v2.y; // set from lower poly vertex
		rect->dirty = 1;
	} else {
		if (poly->v0.y < rect->y0) {
			rect->y0 = poly->v0.y; // update from upper poly vertex
		}
		if (poly->v2.y > rect->y1) {
			rect->y1 = poly->v2.y; // update from lower poly vertex
		}
	}
	// now only x margins are left
	if (poly->v0.x < rect->x0) {
		rect->x0 = poly->v0.x;
	}
	if (poly->v0.x > rect->x1) {
		rect->x1 = poly->v0.x;
	}

	if (poly->v1.x < rect->x0) {
		rect->x0 = poly->v1.x;
	}
	if (poly->v1.x > rect->x1) {
		rect->x1 = poly->v1.x;
	}

	if (poly->v2.x < rect->x0) {
		rect->x0 = poly->v2.x;
	}
	if (poly->v2.x > rect->x1) {
		rect->x1 = poly->v2.x;
	}
}

unsigned long long calc_rating(Chromo *chromo, State *state) {
	Settings *s = state->settings;
	unsigned char *chromo_pixels = (unsigned char *) state->chromoSurf->pixels;
	unsigned char *img_pixels = (unsigned char *) state->imgSurf->pixels;
	unsigned char *chromo_offset;
	unsigned char *img_offset;
	int offset;
	int pitch_step = state->imgSurf->pitch;
	int block_x, block_y, block_val, x, y, b_diff, g_diff, r_diff, block_val2;
	int block_addr;
	int limit_y;
	int limit_x;
	int offset_x;
	int offset_y;

	int block_start_x = chromo->dirty_rect.x0 >> 5;
	int block_start_y = chromo->dirty_rect.y0 >> 5;
	int block_end_x = chromo->dirty_rect.x1 >> 5;
	int block_end_y = chromo->dirty_rect.y1 >> 5;

	for (block_y = block_start_y; block_y <= block_end_y; block_y++) {
		block_addr = block_y * state->blocks_w + block_start_x;
		offset_y = block_y << 5;
		limit_y = state->imgSurf->h - offset_y;
		if (limit_y > BLOCK_SIZE) {
			limit_y = BLOCK_SIZE;
		}
		for (block_x = block_start_x; block_x <= block_end_x; block_x++) {
			block_val = chromo->blocks[block_addr];
			chromo->rating -= block_val;
			block_val2 = block_val;
			block_val = 0;

			offset_x = block_x << 5;
			limit_x = state->imgSurf->w - offset_x;
			if (limit_x > BLOCK_SIZE) {
				limit_x =  BLOCK_SIZE;
			}

			offset = offset_y * state->imgSurf->pitch + offset_x * 4;

			for (y = 0; y < limit_y; y++) {
				chromo_offset = chromo_pixels + offset;
				img_offset = img_pixels + offset;
				for (x = 0; x < limit_x; x++) {
					b_diff = chromo_offset[0] - img_offset[0];
					g_diff = chromo_offset[1] - img_offset[1];
					r_diff = chromo_offset[2] - img_offset[2];
					block_val += r_diff * r_diff + g_diff * g_diff + b_diff * b_diff;
					chromo_offset += 4;
					img_offset += 4;
				}
				offset += pitch_step;
			}

			chromo->blocks[block_addr++] = block_val;
			chromo->rating += block_val;
		}
	}

	return chromo->rating;
}

void normalize_poly(Poly *poly, Settings *s) {
	Vertex *vtmp;
	Vertex v;

	for (int i = 0; i < 3; i++) {
		vtmp = &((Vertex *)&poly->v0)[i];
		if (vtmp->x < 0) {
			vtmp->x = 0;
		} else if (vtmp->x >= s->screenWidth) {
			vtmp->x = s->screenWidth - 1;
		}
		if (vtmp->y < 0) {
			vtmp->y = 0;
		} else if (vtmp->y >= s->screenHeight) {
			vtmp->y = s->screenHeight - 1;
		}
	}

	if (poly->v0.y > poly->v1.y) {
		v.x = poly->v0.x;
		v.y = poly->v0.y;
		v.pixel.value = poly->v0.pixel.value;
		poly->v0.x = poly->v1.x;
		poly->v0.y = poly->v1.y;
		poly->v0.pixel.value = poly->v1.pixel.value;
		poly->v1.x = v.x;
		poly->v1.y = v.y;
		poly->v1.pixel.value = v.pixel.value;
	}

	if (poly->v0.y > poly->v2.y) {
		v.x = poly->v0.x;
		v.y = poly->v0.y;
		v.pixel.value = poly->v0.pixel.value;
		poly->v0.x = poly->v2.x;
		poly->v0.y = poly->v2.y;
		poly->v0.pixel.value = poly->v2.pixel.value;
		poly->v2.x = v.x;
		poly->v2.y = v.y;
		poly->v2.pixel.value = v.pixel.value;
	}

	if (poly->v1.y > poly->v2.y) {
		v.x = poly->v1.x;
		v.y = poly->v1.y;
		v.pixel.value = poly->v1.pixel.value;
		poly->v1.x = poly->v2.x;
		poly->v1.y = poly->v2.y;
		poly->v1.pixel.value = poly->v2.pixel.value;
		poly->v2.x = v.x;
		poly->v2.y = v.y;
		poly->v2.pixel.value = v.pixel.value;
	}

	poly->normalized = 1;
}

void generate_vertex_rgba(Vertex *src_v, Vertex *dst_v, Settings *s) {
	if (s->flatAlpha && src_v) {
		dst_v->pixel.rgba.alpha = src_v->pixel.rgba.alpha;
	} else {
		dst_v->pixel.rgba.alpha = rndi(s->minAlpha, s->maxAlpha);
	}
	if (s->flatColor && src_v) {
		dst_v->pixel.rgba.r = src_v->pixel.rgba.r;
		dst_v->pixel.rgba.g = src_v->pixel.rgba.g;
		dst_v->pixel.rgba.b = src_v->pixel.rgba.b;
	} else {
		if (s->initGray) {
			int col = rnd(256);
			dst_v->pixel.rgba.r = col;
			dst_v->pixel.rgba.g = col;
			dst_v->pixel.rgba.b = col;
		} else {
			dst_v->pixel.rgba.r = rnd(256);
			dst_v->pixel.rgba.g = rnd(256);
			dst_v->pixel.rgba.b = rnd(256);
		}
	}
}

void init_poly(Chromo *chromo, int num, State *state) {
	unsigned char col;
	Settings *s = state->settings;

	short int w = s->screenWidth;
	short int h = s->screenHeight;
	short int x = 0;
	short int y = 0;

	Poly *poly = &chromo->genes[num];

	if (s->overdrive && chromo->count >= s->overdrive) {
		int max_index = 0;
		int max_rating = 0;
		for (int i = 1; i < state->blocks_w * state->blocks_h; i++) {
			if (!state->masked_blocks[i]) {
				int block_rating = chromo->blocks[i];
				if (block_rating > max_rating) {
					max_rating = block_rating;
					max_index = i;
				}
			}
		}
		x = (max_index % state->blocks_w) * BLOCK_SIZE;
		y = (max_index / state->blocks_w) * BLOCK_SIZE;
		if (x > 0) {
			x -= BLOCK_SIZE / 4;
		}
		if (y > 0) {
			y -= BLOCK_SIZE / 4;
		}
		int tw = s->screenWidth - x;
		int th = s->screenHeight - y;
		if (tw > BLOCK_SIZE + BLOCK_SIZE / 2) {
			tw = BLOCK_SIZE + BLOCK_SIZE / 2;
		}
		if (th > BLOCK_SIZE + BLOCK_SIZE / 2) {
			th = BLOCK_SIZE + BLOCK_SIZE / 2;
		}

		if (tw > 2) {
			w = 2 + (tw - 2) * chromo->rating / chromo->start_rating;
		}
		if (th > 2) {
			h = 2 + (th - 2) * chromo->rating / chromo->start_rating;
		}
		if (w < tw) {
			x += rnd(tw - w);
		}
		if (h < th) {
			y += rnd(th - h);
		}

	} else {
		if (chromo->start_rating) {
			w = 2 + (s->screenWidth - 2) * chromo->rating / chromo->start_rating;
			h = 2 + (s->screenHeight - 2) * chromo->rating / chromo->start_rating;
		}

		if (w < s->screenWidth) {
			x = rnd(s->screenWidth - w);
		}
		if (h < s->screenHeight) {
			y = rnd(s->screenHeight - h);
		}
	}

	poly->v0.x = x + rnd(w);
	poly->v0.y = y + rnd(h);

	poly->v1.x = x + rnd(w);
	poly->v1.y = y + rnd(h);

	poly->v2.x = x + rnd(w);
	poly->v2.y = y + rnd(h);

	generate_vertex_rgba(NULL, &poly->v0, s);
	generate_vertex_rgba(&poly->v0, &poly->v1, s);
	generate_vertex_rgba(&poly->v0, &poly->v2, s);

	poly->dirty = 1;
	normalize_poly(poly, s);

	memcpy(&poly->origin_v0, &poly->v0, sizeof(Vertex) * 3);
}

void reset_dirty_rect(Chromo *chromo, Settings *s) {
	chromo->dirty_rect.x0 = 0;
	chromo->dirty_rect.y0 = 0;
	chromo->dirty_rect.x1 = s->screenWidth - 1;
	chromo->dirty_rect.y1 = s->screenHeight - 1;
}

void reset_rating(Chromo *chromo, State *state) {
	chromo->rating = 0;
	reset_dirty_rect(chromo, state->settings);
	memset(&chromo->blocks, 0, sizeof(unsigned int) * state->blocks_w * state->blocks_h);
}

int init_chromo(Chromo *chromo, State *state) {
	Settings *s = state->settings;

	chromo->count = s->minPolygons;
	chromo->pos = 0;

	for (int i = 0; i < chromo->count; i++) {
		init_poly(chromo, i, state);
	}

	state->blocks_w = s->screenWidth >> 5;
	state->blocks_h = s->screenHeight >> 5;

	if (s->screenWidth & 31) {
		state->blocks_w++;
	}
	if (s->screenHeight & 31) {
		state->blocks_h++;
	}

	if (state->blocks_w * state->blocks_h > MAX_BLOCKS) {
		return 0;
	}
	
	reset_dirty_rect(chromo, s);
	reset_rating(chromo, state);

	return 1;
}

int mutate_vertex_alpha(Vertex *v, Settings *s) {
	if (rndc(s->alphaMutation)) {
		v->pixel.rgba.alpha = rndi(s->minAlpha, s->maxAlpha);
		return DIRTY_ALPHA;
	}

	return 0;
}

int mutate_vertex_color(Vertex *v, Settings *s) {
	int dirty = 0;

	if (rndc(s->componentMutation)) {
		v->pixel.rgba.r += rnds(20);
		dirty = DIRTY_COLOR;
	}
	if (rndc(s->componentMutation)) {
		v->pixel.rgba.g += rnds(20);
		dirty = DIRTY_COLOR;
	}
	if (rndc(s->componentMutation)) {
		v->pixel.rgba.b += rnds(20);
		dirty = DIRTY_COLOR;
	}

	return dirty;
}

int mutate_vertex_position(Poly *poly, Vertex *v, Settings *s) {
	int dirty = 0;

	if (rndc(s->pointMaxMutation)) {
		v->x = rnd(s->screenWidth);
		v->y = rnd(s->screenHeight);
		poly->normalized = 0;
		dirty = DIRTY_VERTEX;
	}

	if (rndc(s->pointMidMutation)) {
		v->x += rnds(s->pointMidMovement);
		v->y += rnds(s->pointMidMovement);
		poly->normalized = 0;
		dirty = DIRTY_VERTEX;
	}

	if (rndc(s->pointMinMutation)) {
		v->x += rnds(s->pointSmallMovement);
		v->y += rnds(s->pointSmallMovement);
		poly->normalized = 0;
		dirty = DIRTY_VERTEX;
	}

	return dirty;
};

int mutate_poly(Poly *poly, State *state) {
	int dirty = 0;

	dirty |= mutate_vertex_alpha(&poly->v0, state->settings);
	dirty |= mutate_vertex_color(&poly->v0, state->settings);

	if (state->settings->flatAlpha) {
		if (dirty | DIRTY_ALPHA) {
			poly->v1.pixel.rgba.alpha = poly->v0.pixel.rgba.alpha;
			poly->v2.pixel.rgba.alpha = poly->v0.pixel.rgba.alpha;
		}
	} else {
		dirty |= mutate_vertex_alpha(&poly->v1, state->settings);
		dirty |= mutate_vertex_alpha(&poly->v2, state->settings);
	}

	if (state->settings->flatColor) {
		if (dirty | DIRTY_COLOR) {
			poly->v1.pixel.rgba.r = poly->v0.pixel.rgba.r;
			poly->v1.pixel.rgba.g = poly->v0.pixel.rgba.g;
			poly->v1.pixel.rgba.b = poly->v0.pixel.rgba.b;
			poly->v2.pixel.rgba.r = poly->v0.pixel.rgba.r;
			poly->v2.pixel.rgba.g = poly->v0.pixel.rgba.g;
			poly->v2.pixel.rgba.b = poly->v0.pixel.rgba.b;
		}
	} else {
		dirty |= mutate_vertex_color(&poly->v1, state->settings);
		dirty |= mutate_vertex_color(&poly->v2, state->settings);
	}

	if (rndc(state->settings->scalePolyMutation)) {
		int scale = 100 + rnds(10);
		switch(rnd(3)) {
			case 0:
				poly->v1.x = poly->v1.x + (poly->v1.x - poly->v0.x) * scale / 100;
				poly->v1.y = poly->v1.y + (poly->v1.y - poly->v0.y) * scale / 100;
				poly->v2.x = poly->v2.x + (poly->v2.x - poly->v0.x) * scale / 100;
				poly->v2.y = poly->v2.y + (poly->v2.y - poly->v0.y) * scale / 100;
				break;
			case 1:
				poly->v0.x = poly->v0.x + (poly->v0.x - poly->v1.x) * scale / 100;
				poly->v0.y = poly->v0.y + (poly->v0.y - poly->v1.y) * scale / 100;
				poly->v2.x = poly->v2.x + (poly->v2.x - poly->v1.x) * scale / 100;
				poly->v2.y = poly->v2.y + (poly->v2.y - poly->v1.y) * scale / 100;
				break;
			case 2:
				poly->v0.x = poly->v0.x + (poly->v0.x - poly->v2.x) * scale / 100;
				poly->v0.y = poly->v0.y + (poly->v0.y - poly->v2.y) * scale / 100;
				poly->v1.x = poly->v1.x + (poly->v1.x - poly->v2.x) * scale / 100;
				poly->v1.y = poly->v1.y + (poly->v1.y - poly->v2.y) * scale / 100;
				break;
		}
		dirty |= DIRTY_SCALE;
		poly->normalized = 0;
	} else if (rndc(state->settings->shiftPolyMutation)) {
		int shift_x = rnds(state->settings->pointSmallMovement);
		int shift_y = rnds(state->settings->pointSmallMovement);
		poly->v0.x += shift_x;
		poly->v0.y += shift_y;
		poly->v1.x += shift_x;
		poly->v1.y += shift_y;
		poly->v2.x += shift_x;
		poly->v2.y += shift_y;
		dirty |= DIRTY_SHIFT;
		poly->normalized = 0;
	} else {
		dirty |= mutate_vertex_position(poly, &poly->v0, state->settings);
		dirty |= mutate_vertex_position(poly, &poly->v1, state->settings);
		dirty |= mutate_vertex_position(poly, &poly->v2, state->settings);

	}

	if (!poly->normalized) {
		normalize_poly(poly, state->settings);
	}

	poly->dirty = dirty;

	return dirty;
}

bool delete_poly(Chromo *chromo, int num) {
	int idx;
	if (num == -1) {
		idx = rnd(chromo->count);
	} else {
		idx = num;
	}
	update_dirty_rect(chromo, &chromo->genes[idx]);
	chromo->count--;
	if (idx < chromo->count) {
		memcpy(
			&chromo->genes[idx],
			&chromo->genes[idx + 1],
			(chromo->count - idx) * sizeof(Poly)
		);
	}

	return 1;
}

int add_poly(Chromo *chromo, State *state) {

	init_poly(chromo, chromo->count, state);
	update_dirty_rect(chromo, &chromo->genes[chromo->count]);
	chromo->count++;

	return 1;
}

int move_poly(Chromo *chromo) {
	int src = rnd(chromo->count);
	int dst = rnd(chromo->count);
	Poly poly;

	if (src == dst) {
		return 0;
	}

	chromo->genes[src].dirty = 1;
	memcpy(&poly, &chromo->genes[src], sizeof(Poly));
	update_dirty_rect(chromo, &poly);

	int count = chromo->count - 1;
	if (src < count) {		
		memcpy(&chromo->genes[src], &chromo->genes[src + 1], (count - src) * sizeof(Poly));
	}
	if (dst < count) {
		memmove(&chromo->genes[dst + 1], &chromo->genes[dst], (count - dst) * sizeof(Poly));
	}

	memcpy(&chromo->genes[dst], &poly, sizeof(Poly));

	return 1;
}

void copy_chromo(Chromo *dst, Chromo *src, State *state) {
	memcpy(dst, src, (char *) src->genes - (char *) src + sizeof(Poly) * src->count);
	memcpy(&dst->blocks, &src->blocks, state->blocks_w * state->blocks_h * sizeof(unsigned int));
} 

int mutate_chromo(Chromo *chromo, State *state) {
	int dirty = 0;
	int res;
	int chance;
	Poly tmp_poly;
	Settings *s = state->settings;

	chromo->dirty_rect.dirty = 0;

	if (chromo->count < s->maxPolygons) {
		chance = s->addPolyMutationFast + (s->addPolyMutationSlow - s->addPolyMutationFast) * chromo->rating / chromo->start_rating;
		if (rndc(chance)) {
			add_poly(chromo, state);
			dirty = 1;
		}
	}

	if (rndc(s->delPolyMutation) && chromo->count > s->minPolygons) {
		delete_poly(chromo, -1);
		dirty = 1;
	}

	if (rndc(s->movePolyMutation)) {
		dirty |= move_poly(chromo);
	}

	for (int i = 0; i < chromo->count; i++) {
		chromo->genes[i].dirty = 0;
		memcpy(&tmp_poly, &chromo->genes[i], sizeof(Poly));
		if (mutate_poly(&tmp_poly, state)) {
			dirty = 1;
			update_dirty_rect(chromo, &chromo->genes[i]);
			update_dirty_rect(chromo, &tmp_poly);
			memcpy(&chromo->genes[i], &tmp_poly, sizeof(Poly));
		}
	}

	return dirty;
}

void process_chromo(State *state) {

	Chromo *chromo = state->chromo;
	Chromo *test_chromo = state->test_chromo;
	Poly *poly;

	copy_chromo(test_chromo, chromo, state);

	while(!mutate_chromo(test_chromo, state)) {
		if (state->settings->greedy) {
			if (chromo->pos >= chromo->count) {
				chromo->pos = 0;
			}
			poly = &test_chromo->genes[chromo->pos];
			if (mutate_poly(poly, state)) {
				update_dirty_rect(test_chromo, poly);
				update_dirty_rect(test_chromo, &chromo->genes[chromo->pos]);
				chromo->pos++;
				break;
			}
		}
	}

	draw_chromo(state->chromoSurf, test_chromo, state->settings);
	calc_rating(test_chromo, state);

	if (test_chromo->rating < chromo->rating) {
		copy_chromo(chromo, test_chromo, state);

		state->selected_cnt++;
		SDL_LowerBlit(state->chromoSurf, &state->rect, state->screenSurf, &state->rect);
		if (state->showBounds) {
			draw_dirty_rect(state->screenSurf, &chromo->dirty_rect, 0xff0000);
		}
		for (int i = 0; i < chromo->count; i++) {
			poly = &chromo->genes[i];
			if (poly->dirty) {
				if (state->showPolys) {
					draw_poly(state->screenSurf, poly, 0xffffff);
				}
				state->scale_cnt += (poly->dirty & DIRTY_SCALE) / DIRTY_SCALE;
				state->shift_cnt += (poly->dirty & DIRTY_SHIFT) / DIRTY_SHIFT;
				state->pos_cnt += (poly->dirty & DIRTY_VERTEX) / DIRTY_VERTEX;
			}
		}

		state->update = 1;
	}
}

void get_block_rect(State *state, int block_offset, SDL_Rect *rect) {
	rect->x = (block_offset % state->blocks_w) * BLOCK_SIZE;
	rect->y = (block_offset / state->blocks_w) * BLOCK_SIZE;
	rect->w = state->settings->screenWidth - rect->x;
	rect->h = state->settings->screenHeight - rect->y;
	if (rect->w > BLOCK_SIZE) {
		rect->w = BLOCK_SIZE;
	}
	if (rect->h > BLOCK_SIZE) {
		rect->h = BLOCK_SIZE;
	}
}
