#ifndef HOUGH_H
#define HOUGH_H

#define startThreshold 20

#define endThreshold 20

struct Point {
    float x;
    float y;
};

struct Line {
    float rho;
    float theta;
    struct Point start;
    struct Point end;
};

struct DetectedLines{
    struct Line* lines;
    int count;
};


 struct Linegroup {
    struct Line average;
    int numlines;
} ;

struct Squares {
    struct Point topleft;
    struct Point topright;
    struct Point bottomleft;
    struct Point bottomright;
};



void printvalues(struct Line* lines, int len,SDL_Surface* original_image);

struct Squares findbestsquare(SDL_Surface* original_image,
		struct Line* vertical, struct Line* horizon, int len);


struct Squares* drawsquares(struct Line* lines, int len,
		struct Line* horizon, struct Line* vertical );

void get_sudoku_lines(struct Line *lines, int lineCount,
		struct Line *topLines, int topCount) ;
struct DetectedLines averagearray(struct Line* Line, int len);

void drawl(struct Line* line, int len,SDL_Renderer* renderer,
        SDL_Texture* texture);


struct DetectedLines performHoughTransform(SDL_Surface *surface);

void extract_and_save_squares(SDL_Surface* original_image,
		struct Squares* squares, int num_squares,struct Squares s);
//rot
struct DetectedLines auto_performHoughTransform(SDL_Surface *surface);
SDL_Surface* RotateImage(SDL_Surface* image, double angledegree);
double calculate_angle(	struct DetectedLines result );


#endif
