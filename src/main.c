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

void get_horizontal_range(int max_column_index, const Line_t* line, int* left_column, int* right_column) {
    if (line->_point1._x < LEFT) {
        *left_column = 0;
    } else {
        // -1.0f -> 0
        //  0.0f -> max_column_index / 2
        //  1.0f -> max_column_index
        *left_column = (line->_point1._x - RIGHT) / (float)(max_column_index + 1); 
        // YOU ENDED HERE
    }
}

void draw_horizontal_line(Field_t* field, const Line_t* line, char filled_symbol) {
    int column1 = 0;
    int column2 = 0;
    get_horizontal_range(field->_columns, line, &column1, &column2);

    for (int column = column1; column <= column2; ++column) {

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
    hidecursor();

    Field_t field;
    allocate_field(&field, 6, 60);

    initialize_field(&field, '.');

    for (int i = 0; i < 10; ++i) {
        output_frame(&field);
        goto_xy(0, 0);
    }

    deallocate_field(&field);
}