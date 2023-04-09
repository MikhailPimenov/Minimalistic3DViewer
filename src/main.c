#include <stdio.h>

// #ifdef WINDOWS   // TODO
#include <windows.h>
// #endif WINDOWS   // TODO

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
    double _x;
    double _y;
} Point_t;

typedef struct Line_t {
    Point_t _point1;
    Point_t _point2;
} Line_t;

void draw_line(Field_t* field, const Line_t* line, char filled_symbol) {

}

void output_frame(const Field_t* field) {

}

int main() {    // TODO: arguments including delay between frames

}