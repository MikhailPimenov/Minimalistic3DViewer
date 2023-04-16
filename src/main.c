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
    // point2._x is always greater than point1._x
    return (line->_point2._y - line->_point1._y) > (line->_point2._x - line->_point1._x);
}

void normalize(Line_t* line) {
    if (line->_point2._x < line->_point1._x) {
        const float x = line->_point2._x;
        line->_point2._x = line->_point1._x;
        line->_point1._x = x;

        const float y = line->_point2._y;
        line->_point2._y = line->_point1._y;
        line->_point1._y = y;
    }
}

void get_horizontal_range(int columns, const Line_t* line, int* begin_column, int* end_column) { 
    assert(0 < columns - 1 && "Columns is zero!");
    *begin_column = (line->_point1._x - LEFT) / ((RIGHT - LEFT) / (float)(columns - 1));

    if (*begin_column < 0)
        *begin_column = 0;
    else if (*begin_column > columns - 1)
        *begin_column = -1;

    *end_column = (line->_point2._x - LEFT) / ((RIGHT - LEFT) / (float)(columns - 1)) + 1;

    if (*end_column > columns)
        *end_column = columns;
    else if (*end_column < 0)
        *end_column = -1;
}

void get_vertical_range(int rows, const Line_t* line, int* begin_row, int* end_row) { 
    assert(0 < rows - 1 && "Rows is zero!");
    *begin_row = (line->_point1._y - DOWN) / ((UP - DOWN) / (float)(rows - 1));

    if (*begin_row < 0)
        *begin_row = 0;
    else if (*begin_row > rows - 1)
        *begin_row = -1;

    *end_row = (line->_point2._y - DOWN) / ((UP - DOWN) / (float)(rows - 1)) + 1;

    if (*end_row > rows)
        *end_row = rows;
    else if (*end_row < 0)
        *end_row = -1;
}

float get_y_from_x(const Line_t* line, float x) {
    // assert(line->_point2._x - line->_point1._x != 0 && "Zero-division error!");
    return  (x - line->_point1._x) * 
            ((line->_point2._y - line->_point1._y) / 
            (line->_point2._x - line->_point1._x)) + 
            line->_point1._y;
}

float get_x_from_y(const Line_t* line, float y) {
    // assert(line->_point2._y - line->_point1._y != 0 && "Zero-division error!");
    return  (y - line->_point1._y) * 
            (line->_point2._x - line->_point1._x) / 
            (line->_point2._y - line->_point1._y) + 
            line->_point1._x;
}

int get_column(int columns, float x) {
    // assert(0 < columns - 1 && "Columns is zero or negative! Zero-division error!");
    return (x - LEFT) / ((RIGHT - LEFT) / (float)(columns - 1));
}

int get_row(int rows, float y) {
    // assert(0 < rows - 1 && "Rows is zero or negative! Zero-division error!");
    return (y - DOWN) / ((UP - DOWN) / (float)(rows - 1));
}

float get_x(int columns, int column) {
    assert(0 < columns && "Columns is zero!");
    assert(0 <= column && column < columns && "Column is out of range!");
    return LEFT + column * ((columns - 1 > 0) ? ((RIGHT - LEFT) / (columns - 1)) : (RIGHT - LEFT));
}

float get_y(int rows, int row) {
    assert(0 < rows && "Rows is zero!");
    assert(0 <= row && row < rows && "Row is out of range!");  
    return DOWN + row * ((rows - 1 > 0) ? ((UP - DOWN) / (rows - 1)) : (UP - DOWN));
}



void draw_horizontal_line(Field_t* field, const Line_t* line, char filled_symbol) {
    int column1 = 0;
    int column2 = 0;
    get_horizontal_range(field->_columns, line, &column1, &column2);

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
    int row1 = 0;
    int row2 = 0;
    get_vertical_range(field->_rows, line, &row1, &row2);

    if (row1 == -1 && row2 == -1)
        return;

    const float y1 = get_y(field->_rows, row1);
    const float y2 = get_y(field->_rows, row2);
    const float dy = (row2 - row1 != 0) ? (y2 - y1) / (row2 - row1) : 0.0f;
    float y = y1;
    for (int row = row1; row < row2; ++row) {
        const float x = get_x_from_y(line, y);
        const int column = get_column(field->_columns, x);

        if (0 <= column && column < field->_columns)
            field->_data[row][column] = filled_symbol;

        y += dy;
    }
}

void draw_line(Field_t* field, const Line_t* line, char filled_symbol) {
    assert(0 < field->_rows && "Rows is not valid!");
    assert(0 < field->_columns && "Columns is not valid!");
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

    Line_t v1;
    v1._point1._x = -0.05f;
    v1._point1._y = -0.75f;
    v1._point2._x =  0.05f;
    v1._point2._y =  0.75f;

    Line_t v2;
    v2._point1._x = -0.05f;
    v2._point1._y = -0.75f;
    v2._point2._x = -0.05f;
    v2._point2._y =  0.75f;

    draw_line(&field, &v2, '*');

    for (int i = 0; i < 1; ++i) {
        output_frame(&field);
        // goto_xy(0, 0);
    }

    deallocate_field(&field);
}