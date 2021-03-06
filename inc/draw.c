void bres_line(SDL_Surface *surf, int x0, int y0, int x1, int y1, long int color) {
	int dx = x1 - x0;
	int dy = y1 - y0;
	int dist_x = abs(dx);
	int dist_y = abs(dy);
	int dist = dist_x;
	if (dist_y > dist_x) dist = dist_y;
	int step_x = 4;
	int step_y = surf->pitch;
	if (dx < 0) step_x = -step_x;
	if (dy < 0) step_y = -step_y;
	int err_x = 0;
	int err_y = 0;
	int dst = dist + 1;
	
	unsigned char *addr = (unsigned char *) surf->pixels + y0 * surf->pitch + x0 * 4;

	while(dst--) {
		//addr = (unsigned long int *)((unsigned char *) surf->pixels + y0 * surf->pitch)// + x0 * 4);
		*((unsigned long *) addr) = color;
		err_x += dist_x;
		err_y += dist_y;
		if (err_x >= dist) {
			err_x -= dist;
			//x0 += step_x;
			addr += step_x;
		}
		if (err_y >= dist) {
			err_y -= dist;
			//y0 += step_y;
			addr += step_y;
		}
	}
}

void draw_poly(SDL_Surface *surf, Poly *poly, int color) {
	bres_line(surf, poly->v0.x, poly->v0.y, poly->v1.x, poly->v1.y, color);
	bres_line(surf, poly->v1.x, poly->v1.y, poly->v2.x, poly->v2.y, color);
	bres_line(surf, poly->v2.x, poly->v2.y, poly->v0.x, poly->v0.y, color);
}
	
void draw_dirty_rect(SDL_Surface *surf, DirtyRect *rect, int color) {
	bres_line(surf, rect->x0, rect->y0, rect->x1, rect->y0, color);
	bres_line(surf, rect->x0, rect->y1, rect->x1, rect->y1, color);
	bres_line(surf, rect->x0, rect->y0, rect->x0, rect->y1, color);
	bres_line(surf, rect->x1, rect->y0, rect->x1, rect->y1, color);
}

void draw_rect(SDL_Surface *surf, SDL_Rect *rect, int color) {
	bres_line(surf, rect->x, rect->y, rect->x + rect->w - 1, rect->y, color);
	bres_line(surf, rect->x, rect->y + rect->h - 1, rect->x + rect->w - 1, rect->y + rect->h - 1, color);
	bres_line(surf, rect->x, rect->y, rect->x, rect->y + rect->h - 1, color);
	bres_line(surf, rect->x + rect->w - 1, rect->y, rect->x + rect->w - 1, rect->y + rect->h - 1, color);
}

int fill_poly(SDL_Surface *surf, Poly *t) {

	// poly must be normalized

	int cross_x1, cross_x2, start_x, width;
	int progress_y;
	unsigned char start_r, start_g, start_b, start_alpha, end_r, end_g, end_b, end_alpha;
	int range_r, range_g, range_b, range_alpha, ar, ag, ab;

	int dx1 = t->v1.x - t->v0.x;
	int dy1 = t->v1.y - t->v0.y;
	int dx2 = t->v2.x - t->v0.x;
	int dy2 = t->v2.y - t->v0.y;
	int top_y = t->v0.y;

	unsigned char v0_r = t->v0.pixel.rgba.r;
	unsigned char v0_g = t->v0.pixel.rgba.g;
	unsigned char v0_b = t->v0.pixel.rgba.b;
	unsigned char v0_alpha = t->v0.pixel.rgba.alpha;

	unsigned char v1_r = t->v1.pixel.rgba.r;
	unsigned char v1_g = t->v1.pixel.rgba.g;
	unsigned char v1_b = t->v1.pixel.rgba.b;
	unsigned char v1_alpha = t->v1.pixel.rgba.alpha;

	unsigned char v2_r = t->v2.pixel.rgba.r;
	unsigned char v2_g = t->v2.pixel.rgba.g;
	unsigned char v2_b = t->v2.pixel.rgba.b;
	unsigned char v2_alpha = t->v2.pixel.rgba.alpha;

	unsigned int src_alpha;

	if (dy1) {
		while(top_y <= t->v1.y) {
			progress_y = top_y - t->v0.y;
			cross_x1 = t->v0.x + dx1 * progress_y / dy1;
			cross_x2 = t->v0.x + dx2 * progress_y / dy2;

			if (cross_x1 > cross_x2) {
				start_x = cross_x2;
				width = (cross_x1 - cross_x2) + 1;

				start_r = v0_r + (v2_r - v0_r) * progress_y / dy2;
				start_g = v0_g + (v2_g - v0_g) * progress_y / dy2;
				start_b = v0_b + (v2_b - v0_b) * progress_y / dy2;
				start_alpha = v0_alpha + (v2_alpha - v0_alpha) * progress_y / dy2;

				end_r = v0_r + (v1_r - v0_r) * progress_y / dy1;
				end_g = v0_g + (v1_g - v0_g) * progress_y / dy1;
				end_b = v0_b + (v1_b - v0_b) * progress_y / dy1;
				end_alpha = v0_alpha + (v1_alpha - v0_alpha) * progress_y / dy1;			
			} else {
				start_x = cross_x1;
				width = (cross_x2 - cross_x1) + 1;

				start_r = v0_r + (v1_r - v0_r) * progress_y / dy1;
				start_g = v0_g + (v1_g - v0_g) * progress_y / dy1;
				start_b = v0_b + (v1_b - v0_b) * progress_y / dy1;
				start_alpha = v0_alpha + (v1_alpha - v0_alpha) * progress_y / dy1;			

				end_r = v0_r + (v2_r - v0_r) * progress_y / dy2;
				end_g = v0_g + (v2_g - v0_g) * progress_y / dy2;
				end_b = v0_b + (v2_b - v0_b) * progress_y / dy2;
				end_alpha = v0_alpha + (v2_alpha - v0_alpha) * progress_y / dy2;
			}

			range_r = end_r - start_r;
			range_g = end_g - start_g;
			range_b = end_b - start_b;
			range_alpha = end_alpha - start_alpha; 

			unsigned char *addr = (unsigned char *)surf->pixels + top_y * surf->pitch + start_x * 4;
			for (int i = 0; i < width; i++) {
				src_alpha = start_alpha + range_alpha * i / width;

				ab = (start_b + range_b * i / width) * src_alpha;
				ag = (start_g + range_g * i / width) * src_alpha;
				ar = (start_r + range_r * i / width) * src_alpha;

				addr[0] = (ab + addr[0] * (255 - src_alpha)) / 255;
				addr[1] = (ag + addr[1] * (255 - src_alpha)) / 255;
				addr[2] = (ar + addr[2] * (255 - src_alpha)) / 255;

				addr += 4;
			}

			top_y++;
		}
	}

	dx1 = t->v2.x - t->v1.x;
	dy1 = t->v2.y - t->v1.y;

	if (dy1) {
		while(top_y <= t->v2.y) {
			progress_y = top_y - t->v0.y;
			int progress_y1 = top_y - t->v1.y;
			cross_x1 = t->v1.x + dx1 * progress_y1 / dy1;
			cross_x2 = t->v0.x + dx2 * progress_y / dy2;
			if (cross_x1 > cross_x2) {
				start_x = cross_x2;
				width = (cross_x1 - cross_x2) + 1;

				start_r = v0_r + (v2_r - v0_r) * progress_y / dy2;
				start_g = v0_g + (v2_g - v0_g) * progress_y / dy2;
				start_b = v0_b + (v2_b - v0_b) * progress_y / dy2;
				start_alpha = v0_alpha + (v2_alpha - v0_alpha) * progress_y / dy2;

				end_r = v1_r + (v2_r - v1_r) * progress_y1 / dy1;
				end_g = v1_g + (v2_g - v1_g) * progress_y1 / dy1;
				end_b = v1_b + (v2_b - v1_b) * progress_y1 / dy1;
				end_alpha = v1_alpha + (v2_alpha - v1_alpha) * progress_y1 / dy1;
			} else {
				start_x = cross_x1;
				width = (cross_x2 - cross_x1) + 1;

				start_r = v1_r + (v2_r - v1_r) * progress_y1 / dy1;
				start_g = v1_g + (v2_g - v1_g) * progress_y1 / dy1;
				start_b = v1_b + (v2_b - v1_b) * progress_y1 / dy1;
				start_alpha = v1_alpha + (v2_alpha - v1_alpha) * progress_y1 / dy1;

				end_r = v0_r + (v2_r - v0_r) * progress_y / dy2;
				end_g = v0_g + (v2_g - v0_g) * progress_y / dy2;
				end_b = v0_b + (v2_b - v0_b) * progress_y / dy2;
				end_alpha = v0_alpha + (v2_alpha - v0_alpha) * progress_y / dy2;
			}

			range_r = end_r - start_r;
			range_g = end_g - start_g;
			range_b = end_b - start_b; 
			range_alpha = end_alpha - start_alpha;

			unsigned char *addr = (unsigned char *)surf->pixels + top_y * surf->pitch + start_x * 4;
			for (int i = 0; i < width; i++) {
				src_alpha = start_alpha + range_alpha * i / width;

				ab = (start_b + range_b * i / width) * src_alpha;
				ag = (start_g + range_g * i / width) * src_alpha;
				ar = (start_r + range_r * i / width) * src_alpha;

				addr[0] = (ab + addr[0] * (255 - src_alpha)) / 255;
				addr[1] = (ag + addr[1] * (255 - src_alpha)) / 255;
				addr[2] = (ar + addr[2] * (255 - src_alpha)) / 255;

				addr += 4;
			}
			top_y++;
		}
	}
}

void draw_chromo(SDL_Surface *surf, Chromo *chromo, Settings *s) {
	SDL_Rect rect = {0, 0, surf->w, surf->h};
	SDL_FillRect(surf, &rect, 0);

	for (int i = 0; i < chromo->count; i++) {
		fill_poly(surf, &chromo->genes[i]);
	}
}
