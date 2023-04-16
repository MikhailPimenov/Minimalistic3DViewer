#include <stdio.h>
#include <assert.h>

// #ifdef WINDOWS   // TODO
#include <windows.h>
// #endif WINDOWS   // TODO

#define LEFT -1.0f
#define RIGHT 1.0f
#define UP 1.0f
#define DOWN -1.0f

// #ifdef WINDOWS   // TODO
// CONSOLE===========================================================================
void hidecursor()
{
   HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
   CONSOLE_CURSOR_INFO info;
   info.dwSize = 100;
   info.bVisible = FALSE;
   SetConsoleCursorInfo(consoleHandle, &info);
}

void goto_xy(unsigned x, unsigned y)
{
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), (COORD){x, y});
}
// CONSOLE ENDED======================================================================
// #endif WINDOWS   // TODO



typedef enum Bool {
    False = 0,
    True  = 1,
} Bool;


// field range for x is [-1.0f, 1.0f] and for y is [-1.0f, 1.0f]
typedef struct Field_t {
    char** _data;
    int _rows;
    int _columns;
} Field_t;
// FIELD =====================================================================================
void initialize_field(Field_t* field, char symbol) {
    for (int row = 0; row < field->_rows; ++row)
        for (int column = 0; column < field->_columns; ++column)
            field->_data[row][column] = symbol;
}
void allocate_field(Field_t* field, int rows, int columns) {
    field->_data = malloc(rows * columns * sizeof(char) + rows * sizeof(char*));

    for (int row = 0; row < rows; ++row)
        *(field->_data + row) = (char*)((char**)((char*)field->_data + row * columns) + rows);

    field->_rows = rows;
    field->_columns = columns;

    initialize_field(field, ' ');
}
void deallocate_field(Field_t* field) {
    if (field) {
        if (field->_data)
            free(field->_data);
        field->_rows = 0;
        field->_columns = 0;
    } 
}
// FIELD ENDED================================================================================

typedef struct Point_t {
    float _x;
    float _y;
} Point_t;


// LINE=======================================================================================
typedef struct Line_t {
    Point_t _point1;
    Point_t _point2;
} Line_t;

Bool is_line_vertical(const Line_t* line) {
    // point2._y is always greater than point1._y
    // point2._x is always greater than point1._x

    return (line->_point2._y - line->_point1._y) > (line->_point2._x - line->_point1._x);
}

// int get_column(float x, int max_column_index) {
    // return (x - LEFT) / ((RIGHT - LEFT) / (float)(max_column_index));
// }

void get_horizontal_range(int max_column_index, const Line_t* line, int* begin_column, int* end_column) { 
    *begin_column = (line->_point1._x - LEFT) / ((RIGHT - LEFT) / (float)(max_column_index));

    if (*begin_column < 0)
        *begin_column = 0;
    else if (*begin_column > max_column_index)
        *begin_column = -1;
/*
    // -1.0              0.0               1.0
    //   0   1  2  3  4   5    6  7  8  9  10
    //  0.0              1.0               2.0
    //   0   1  2  3  4   5    6  7  8  9  10

    //  10.0              10.5      10.789         11.0
    //   0   1  2  3  4     5    6     7       8  9   10

    // 10.789 - 10.0 = 0.789
    // 0.789 / 0.1 = 0.789 / (1.0 / 10) = 0.789 / (11.0 - 10.0 / 10) = 0.789 / 0.1 = 7

    //  0.0  0.1 0.2 0.3 0.4  0.5  0.6 0.7 0.8 0.9  1.0
    //   0    1   2   3   4    5    6   7   8   9    10

    // 0.789 -> 7 = 0.789 / 0.1 = 0.789 / (1.0 / 10) = 0.789 / ((LEFT - RIGHT) / (float)max_column_index)


    // -1.0              0.0               1.0
    //   0   1  2  3  4   5    6  7  8  9  10

    // -0.05 -> 4 = (-0.05 - (-1.0)) / ((1.0 - (-1.0)) / 10) = (x - RIGHT) / ((LEFT - RIGHT) / (float)max_column_index)
    // -0.05 -> 4 =  0.95 / (2.0 / 10) = 0.95 / 0.2 = 4._ = 4 

    // -2.0 -> column < 0
    (x - RIGHT) / ((LEFT - RIGHT) / (float)max_column_index)
    (-2.0 - (-1.0))
*/

    *end_column = (line->_point2._x - LEFT) / ((RIGHT - LEFT) / (float)(max_column_index)) + 1;

    if (*end_column > max_column_index + 1)
        *end_column = max_column_index + 1;
    else if (*end_column < 0)
        *end_column = -1;

}

void get_vertical_range(int max_row_index, const Line_t* line, int* start_row, int* end_row) { 
    *start_row = (line->_point1._y - DOWN) / ((UP - DOWN) / (float)(max_row_index));

    if (*start_row < 0)
        *start_row = 0;
    else if (*start_row > max_row_index)
        *start_row = -1;

    *end_row = (line->_point2._y - DOWN) / ((UP - DOWN) / (float)(max_row_index)) + 1;

    if (*end_row > max_row_index + 1)
        *end_row = max_row_index + 1;
    else if (*end_row < 0)
        *end_row = -1;
}

float get_y_from_x(const Line_t* line, float x) {
    return  (x - line->_point1._x) * 
            ((line->_point2._y - line->_point1._y) / 
            (line->_point2._x - line->_point1._x)) + 
            line->_point1._y;
}

float get_x_from_y(const Line_t* line, float y) {
    return  (y - line->_point1._y) * 
            (line->_point2._x - line->_point1._x) / 
            (line->_point2._y - line->_point1._y) + 
            line->_point1._x;
}

int get_column(int columns, float x) {
    return (x - LEFT) / ((LEFT - RIGHT) / (float)(columns - 1));
}

int get_row(int rows, float y) {
    return (y - DOWN) / ((UP - DOWN) / (float)(rows - 1));
}

float get_x(int columns, int column) {
    assert(0 < columns && "Columns is zero!");
    assert(0 <= column && column < columns && "Column is out of range!");
    // const float range = RIGHT - LEFT;
    // const float dx = (columns - 1 > 0) ? (range / (columns - 1)) : range;
    // const float x = dx * column;
    // const float result = LEFT + x;
    
    
    return LEFT + column * ((columns - 1 > 0) ? ((RIGHT - LEFT) / (columns - 1)) : (RIGHT - LEFT));

    // 10.0             10.5             11.0
    //  0    1 2 3 4     5    6 7 8 9     10
    
    // 0.0             0.5             1.0
    //  0    1 2 3 4    5    6 7 8 9   10

    // -1.0            0.0             1.0
    //  0    1 2 3 4    5    6 7 8 9   10
    //  1.0 -> columns - 1
    // -1.0 -> 0
    //  0.0 -> 
}



void draw_horizontal_line(Field_t* field, const Line_t* line, char filled_symbol) {
    int column1 = 0;
    int column2 = 0;
    get_horizontal_range(field->_columns - 1, line, &column1, &column2);

    if (column1 == -1 && column2 == -1)
        return;

    const float x1 = get_x(field->_columns, column1);
    const float x2 = get_x(field->_columns, column2);
    const float dx = (column2 - column1 != 0) ? (x2 - x1) / (column2 - column1) : 0.0f;
    float x = x1;
    for (int column = column1; column < column2; ++column) {
        const float y = get_y_from_x(line, x);
        const int row = get_row(field->_rows, y);

        if (0 <= row && row < field->_rows)
            field->_data[row][column] = filled_symbol;

        x += dx;
    }
}

void draw_vertical_line(Field_t* field, const Line_t* line, char filled_symbol) {
    assert(0);
}

void draw_line(Field_t* field, const Line_t* line, char filled_symbol) {
    if (is_line_vertical(line)) 
        draw_vertical_line(field, line, filled_symbol);
    else
        draw_horizontal_line(field, line, filled_symbol);
}

// LINE ENDED=================================================================================
void output_frame(const Field_t* field) {
    for (int row = 0; row < field->_rows; ++row) {
        for (int column = 0; column < field->_columns; ++column) {
            printf("%c", field->_data[row][column]);
        }
        printf("\n");
    }
}

int main() {    // TODO: arguments including delay between frames
    // hidecursor();

    Field_t field;
    allocate_field(&field, 10, 100);

    initialize_field(&field, '.');

    Line_t h1;
    h1._point1._x = -0.5f;
    h1._point1._y =  0.0f;
    h1._point2._x =  0.5f;
    h1._point2._y =  0.0f;

    Line_t h2;
    h2._point1._x = -0.5f;
    h2._point1._y =  0.0f;
    h2._point2._x =  0.5f;
    h2._point2._y =  0.5f;

    draw_line(&field, &h2, '*');

    for (int i = 0; i < 1; ++i) {
        output_frame(&field);
        // goto_xy(0, 0);
    }

    deallocate_field(&field);
}