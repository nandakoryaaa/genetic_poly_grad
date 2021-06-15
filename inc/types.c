typedef enum {
	DIRTY_VERTEX = 1,
	DIRTY_ALPHA = 2,
	DIRTY_COLOR = 4,
	DIRTY_SCALE = 8,
	DIRTY_SHIFT = 16,
	DIRTY_VERTEXSHIFT = 32,
	DIRTY_POLY_DELETED = 64
} DIRTY_MASK;

typedef enum {
	MODE_RUN, MODE_MASK, MODE_ORIGIN
} Mode;

typedef struct {
	unsigned char b, g, r, alpha;
} RGBA;

typedef struct {
	int dirty, x0, y0, x1, y1;
} DirtyRect;

typedef struct {
	int x, y;
	union {
		RGBA rgba; 
		int value;
	} pixel;
} Vertex;

typedef struct {
	int dirty;
	int normalized;
	Vertex v0;
	Vertex v1;
	Vertex v2;
	Vertex origin_v0;
	Vertex origin_v1;
	Vertex origin_v2;
} Poly;

typedef struct {
	unsigned long long start_rating;
	unsigned long long rating;
	unsigned int count;
	unsigned int pos;
	DirtyRect dirty_rect;
	Poly genes[MAX_CHROMO_SIZE];
	unsigned int blocks[MAX_BLOCKS];
} Chromo;

typedef struct {
	int screenWidth;
	int screenHeight;

	int minPolygons = 1;
	int maxPolygons = MAX_CHROMO_SIZE;
	int minAlpha = 0;
	int maxAlpha = 255;
	int pointMidMovement = 20;
	int pointSmallMovement = 3;
	int overdrive = 0;

	int addPolyMutationFast = 300;
	int addPolyMutationSlow = 12000;
	int delPolyMutation = 1500;
	int movePolyMutation = 700;

	int scalePolyMutation = 1500;
	int rotatePolyMutation = 1500;
	int shiftPolyMutation = 1500;
	int shiftVertexMutation = 1500;

	int pointMinMutation = 1500;
	int pointMidMutation = 1500;
	int pointMaxMutation = 1500;
	int alphaMutation = 1500;
	int componentMutation = 1500;

	char flatColor = 0;
	char flatAlpha = 0;
	char initGray = 0;
	char greedy = 0;
} Settings;

typedef struct {
	char *filename = NULL;
	Settings *settings;
	SDL_Surface *imgSurf;
	SDL_Surface *chromoSurf;
	SDL_Surface *screenSurf;
	Chromo *chromo;
	Chromo *test_chromo;
	SDL_Rect rect;
	Mode mode;
	char masked_blocks[MAX_BLOCKS];
	char *load_chromo = NULL;
	int blocks_w;
	int blocks_h;
	int *event_filter;
	int event_filter_count;
	int cnt = 0;
	int selected_cnt = 0;
	int shift_cnt = 0;
	int scale_cnt = 0;
	int pos_cnt = 0;
	int start_time;
	int selected_block = 0;
	int min_block = 0;
	int max_block = 0;
	int seed;
	char showBounds = 0;
	char showPolys = 0;
	char phase = 0;
	char update = 0;
} State;
