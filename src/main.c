#include <stdio.h>
#include <assert.h>
#include <math.h>

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
    return abs(line->_point2._y - line->_point1._y) > abs(line->_point2._x - line->_point1._x);
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

    const float begin_x = line->_point1._x < line->_point2._x ? line->_point1._x : line->_point2._x;
    const float end_x = line->_point1._x > line->_point2._x ? line->_point1._x : line->_point2._x;

    *begin_column = (begin_x - LEFT) / ((RIGHT - LEFT) / (float)(columns - 1));

    // -1.0              0.0                  1.0
    //   0    1 2 3 4     5     6 7 8 9        10               // columns 11



    //  10.0       10.312   10.5   10.789              11.0
    //   0    1 2   3   4    5     6   7    8   9        10               // columns 11
    // 0.789 -> 7 = 0.789 / 0.1 = (10.789 - 10) / ((1.0 - 0.0) / 10)

    if (*begin_column < 0)
        *begin_column = 0;
    else if (*begin_column > columns - 1)
        *begin_column = columns;

    *end_column = (end_x - LEFT) / ((RIGHT - LEFT) / (float)(columns - 1)) + 1;

    if (*end_column > columns)
        *end_column = columns;
    else if (*end_column < 0)
        *end_column = 0;

    if (*end_column < *begin_column) {
        const int temporary = *end_column;
        *end_column = *begin_column;
        *begin_column = temporary;
    }
}

void get_vertical_range(int rows, const Line_t* line, int* begin_row, int* end_row) { 
    assert(0 < rows - 1 && "Rows is zero!");
    
    const float begin_y = line->_point1._y < line->_point2._y ? line->_point1._y : line->_point2._y;
    const float end_y = line->_point1._y > line->_point2._y ? line->_point1._y : line->_point2._y;
    
    *begin_row = (begin_y - DOWN) / ((UP - DOWN) / (float)(rows - 1));

    if (*begin_row < 0)
        *begin_row = 0;
    else if (*begin_row > rows - 1)
        *begin_row = -1;

    *end_row = (end_y - DOWN) / ((UP - DOWN) / (float)(rows - 1)) + 1;

    if (*end_row > rows)
        *end_row = rows;
    else if (*end_row < 0)
        *end_row = -1;

    if (*end_row < *begin_row) {
        const int temporary = *end_row;
        *end_row = *begin_row;
        *begin_row = temporary;
    }
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
    assert(0 <= column && "Column is negative!");
    return LEFT + column * ((columns - 1 > 0) ? ((RIGHT - LEFT) / (columns - 1)) : (RIGHT - LEFT));
}

float get_y(int rows, int row) {
    assert(0 < rows && "Rows is zero!");
    // assert(0 <= row && "Row is negative!");  
    return DOWN + row * ((rows - 1 > 0) ? ((UP - DOWN) / (rows - 1)) : (UP - DOWN));
}



void draw_horizontal_line(Field_t* field, const Line_t* line, char filled_symbol) {
    int column1 = 0;
    int column2 = 0;
    get_horizontal_range(field->_columns, line, &column1, &column2);

    if (column1 == column2)
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

    if (row1 == row2)
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



// 3D graphics==========================================================================

typedef struct Matrix_t {
    float** _matrix;
    int _rows;
    int _columns;
} Matrix_t;


typedef struct Triangle_t {
    Point_t _points[3];
} Triangle_t;

void draw_triangle(Field_t* field, const Triangle_t* triangle, char filled_symbol) {
    Line_t a;
    Line_t b;
    Line_t c;

    a._point1 = triangle->_points[0];
    a._point2 = triangle->_points[1];
    normalize(&a);

    draw_line(field, &a, filled_symbol);

    b._point1 = triangle->_points[1];
    b._point2 = triangle->_points[2];
    normalize(&b);

    draw_line(field, &b, filled_symbol);

    c._point1 = triangle->_points[2];
    c._point2 = triangle->_points[0];
    normalize(&c);

    draw_line(field, &c, filled_symbol);
}

typedef struct Point3D_t {
    float _x;
    float _y;
    float _z;
} Point3D_t;

typedef struct Triangle3D_t {
    Point3D_t _points[3];
} Triangle3D_t;

typedef struct Object3D_t {
    Point3D_t* _vertices;
    int _vertices_length;

    Triangle3D_t* _trianlges;
    int _trianlge_length;
} Object3D_t;

// o Cube
// v 1.000000 -1.000000 -1.000000
// v 1.000000 -1.000000 1.000000
// v -1.000000 -1.000000 1.000000
// v -1.000000 -1.000000 -1.000000
// v 1.000000 1.000000 -0.999999
// v 0.999999 1.000000 1.000001
// v -1.000000 1.000000 1.000000
// v -1.000000 1.000000 -1.000000
// vt 1.000000 0.333333
// vt 1.000000 0.666667
// vt 0.666667 0.666667
// vt 0.666667 0.333333
// vt 0.666667 0.000000
// vt 0.000000 0.333333
// vt 0.000000 0.000000
// vt 0.333333 0.000000
// vt 0.333333 1.000000
// vt 0.000000 1.000000
// vt 0.000000 0.666667
// vt 0.333333 0.333333
// vt 0.333333 0.666667
// vt 1.000000 0.000000
// vn 0.000000 -1.000000 0.000000
// vn 0.000000 1.000000 0.000000
// vn 1.000000 0.000000 0.000000
// vn -0.000000 0.000000 1.000000
// vn -1.000000 -0.000000 -0.000000
// vn 0.000000 0.000000 -1.000000
// usemtl Material
// s off
// f 2/1/1 3/2/1 4/3/1
// f 8/1/2 7/4/2 6/5/2
// f 5/6/3 6/7/3 2/8/3
// f 6/8/4 7/5/4 3/4/4
// f 3/9/5 7/10/5 8/11/5
// f 1/12/6 4/13/6 8/11/6
// f 1/4/1 2/1/1 4/3/1
// f 5/14/2 8/1/2 6/5/2
// f 1/12/3 5/6/3 2/8/3
// f 2/12/4 6/8/4 3/4/4
// f 4/13/5 3/9/5 8/11/5
// f 5/6/6 1/12/6 8/11/6

typedef struct Matrix4x4_t {
    float _data[4][4];
} Matrix4x4_t;

typedef struct Cube_t {
    Triangle3D_t _triangles[12];
} Cube_t;

void create_cube(Cube_t* cube) {
    // south ========================================================

    // triangle --------------------------------------------
    cube->_triangles[0]._points[0]._x = 0.0f;
    cube->_triangles[0]._points[0]._y = 0.0f;
    cube->_triangles[0]._points[0]._z = 0.0f;

    cube->_triangles[0]._points[1]._x = 0.0f;
    cube->_triangles[0]._points[1]._y = 1.0f;
    cube->_triangles[0]._points[1]._z = 0.0f;

    cube->_triangles[0]._points[2]._x = 1.0f;
    cube->_triangles[0]._points[2]._y = 1.0f;
    cube->_triangles[0]._points[2]._z = 0.0f;

    // return;

    // triangle --------------------------------------------
    cube->_triangles[1]._points[0]._x = 0.0f;
    cube->_triangles[1]._points[0]._y = 0.0f;
    cube->_triangles[1]._points[0]._z = 0.0f;

    cube->_triangles[1]._points[1]._x = 1.0f;
    cube->_triangles[1]._points[1]._y = 1.0f;
    cube->_triangles[1]._points[1]._z = 0.0f;

    cube->_triangles[1]._points[2]._x = 1.0f;
    cube->_triangles[1]._points[2]._y = 0.0f;
    cube->_triangles[1]._points[2]._z = 0.0f;


    // east ========================================================

    // triangle --------------------------------------------
    cube->_triangles[2]._points[0]._x = 1.0f;
    cube->_triangles[2]._points[0]._y = 0.0f;
    cube->_triangles[2]._points[0]._z = 0.0f;

    cube->_triangles[2]._points[1]._x = 1.0f;
    cube->_triangles[2]._points[1]._y = 1.0f;
    cube->_triangles[2]._points[1]._z = 0.0f;

    cube->_triangles[2]._points[2]._x = 1.0f;
    cube->_triangles[2]._points[2]._y = 1.0f;
    cube->_triangles[2]._points[2]._z = 1.0f;

    // triangle --------------------------------------------
    cube->_triangles[3]._points[0]._x = 1.0f;
    cube->_triangles[3]._points[0]._y = 0.0f;
    cube->_triangles[3]._points[0]._z = 0.0f;

    cube->_triangles[3]._points[1]._x = 1.0f;
    cube->_triangles[3]._points[1]._y = 1.0f;
    cube->_triangles[3]._points[1]._z = 1.0f;

    cube->_triangles[3]._points[2]._x = 1.0f;
    cube->_triangles[3]._points[2]._y = 0.0f;
    cube->_triangles[3]._points[2]._z = 1.0f;





    // north ========================================================

    // triangle --------------------------------------------
    cube->_triangles[4]._points[0]._x = 1.0f;
    cube->_triangles[4]._points[0]._y = 0.0f;
    cube->_triangles[4]._points[0]._z = 1.0f;

    cube->_triangles[4]._points[1]._x = 1.0f;
    cube->_triangles[4]._points[1]._y = 1.0f;
    cube->_triangles[4]._points[1]._z = 1.0f;

    cube->_triangles[4]._points[2]._x = 0.0f;
    cube->_triangles[4]._points[2]._y = 1.0f;
    cube->_triangles[4]._points[2]._z = 1.0f;

    // triangle --------------------------------------------
    cube->_triangles[5]._points[0]._x = 1.0f;
    cube->_triangles[5]._points[0]._y = 0.0f;
    cube->_triangles[5]._points[0]._z = 1.0f;

    cube->_triangles[5]._points[1]._x = 0.0f;
    cube->_triangles[5]._points[1]._y = 1.0f;
    cube->_triangles[5]._points[1]._z = 1.0f;

    cube->_triangles[5]._points[2]._x = 0.0f;
    cube->_triangles[5]._points[2]._y = 0.0f;
    cube->_triangles[5]._points[2]._z = 1.0f;



    // west ========================================================

    // triangle --------------------------------------------
    cube->_triangles[6]._points[0]._x = 0.0f;
    cube->_triangles[6]._points[0]._y = 0.0f;
    cube->_triangles[6]._points[0]._z = 1.0f;

    cube->_triangles[6]._points[1]._x = 0.0f;
    cube->_triangles[6]._points[1]._y = 1.0f;
    cube->_triangles[6]._points[1]._z = 1.0f;

    cube->_triangles[6]._points[2]._x = 0.0f;
    cube->_triangles[6]._points[2]._y = 1.0f;
    cube->_triangles[6]._points[2]._z = 0.0f;

    // triangle --------------------------------------------
    cube->_triangles[7]._points[0]._x = 0.0f;
    cube->_triangles[7]._points[0]._y = 0.0f;
    cube->_triangles[7]._points[0]._z = 1.0f;

    cube->_triangles[7]._points[1]._x = 0.0f;
    cube->_triangles[7]._points[1]._y = 1.0f;
    cube->_triangles[7]._points[1]._z = 0.0f;

    cube->_triangles[7]._points[2]._x = 0.0f;
    cube->_triangles[7]._points[2]._y = 0.0f;
    cube->_triangles[7]._points[2]._z = 0.0f;






    // top ========================================================

    // triangle --------------------------------------------
    cube->_triangles[8]._points[0]._x = 0.0f;
    cube->_triangles[8]._points[0]._y = 1.0f;
    cube->_triangles[8]._points[0]._z = 0.0f;

    cube->_triangles[8]._points[1]._x = 0.0f;
    cube->_triangles[8]._points[1]._y = 1.0f;
    cube->_triangles[8]._points[1]._z = 1.0f;

    cube->_triangles[8]._points[2]._x = 1.0f;
    cube->_triangles[8]._points[2]._y = 1.0f;
    cube->_triangles[8]._points[2]._z = 1.0f;

    // triangle --------------------------------------------
    cube->_triangles[9]._points[0]._x = 0.0f;
    cube->_triangles[9]._points[0]._y = 1.0f;
    cube->_triangles[9]._points[0]._z = 0.0f;

    cube->_triangles[9]._points[1]._x = 1.0f;
    cube->_triangles[9]._points[1]._y = 1.0f;
    cube->_triangles[9]._points[1]._z = 1.0f;

    cube->_triangles[9]._points[2]._x = 1.0f;
    cube->_triangles[9]._points[2]._y = 1.0f;
    cube->_triangles[9]._points[2]._z = 0.0f;





    // bottom ========================================================

    // triangle --------------------------------------------
    cube->_triangles[10]._points[0]._x = 1.0f;
    cube->_triangles[10]._points[0]._y = 0.0f;
    cube->_triangles[10]._points[0]._z = 1.0f;

    cube->_triangles[10]._points[1]._x = 0.0f;
    cube->_triangles[10]._points[1]._y = 0.0f;
    cube->_triangles[10]._points[1]._z = 1.0f;

    cube->_triangles[10]._points[2]._x = 0.0f;
    cube->_triangles[10]._points[2]._y = 0.0f;
    cube->_triangles[10]._points[2]._z = 0.0f;

    // triangle --------------------------------------------
    cube->_triangles[11]._points[0]._x = 1.0f;
    cube->_triangles[11]._points[0]._y = 0.0f;
    cube->_triangles[11]._points[0]._z = 1.0f;

    cube->_triangles[11]._points[1]._x = 0.0f;
    cube->_triangles[11]._points[1]._y = 0.0f;
    cube->_triangles[11]._points[1]._z = 0.0f;

    cube->_triangles[11]._points[2]._x = 1.0f;
    cube->_triangles[11]._points[2]._y = 0.0f;
    cube->_triangles[11]._points[2]._z = 0.0f;

}

void multiplyMatrixVector(const Point3D_t* in, Point3D_t* out, const Matrix4x4_t* m) {
    out->_x = in->_x * m->_data[0][0] + in->_y * m->_data[1][0] + in->_z * m->_data[2][0] + m->_data[3][0];
    out->_y = in->_x * m->_data[0][1] + in->_y * m->_data[1][1] + in->_z * m->_data[2][1] + m->_data[3][1];
    out->_z = in->_x * m->_data[0][2] + in->_y * m->_data[1][2] + in->_z * m->_data[2][2] + m->_data[3][2];

    const float w = in->_x * m->_data[0][3] + in->_y * m->_data[1][3] + in->_z * m->_data[2][3] + m->_data[3][3];

    if (w != 0.0f) // TODO: compare floating point numbers correctly
    {
        out->_x /= w;
        out->_y /= w;
        out->_z /= w;
    }
}

void initialize_triangle3d(Triangle3D_t* triangle) {
    for (int i = 0; i < 3; ++i) {
        triangle->_points[i]._x = 0.0f;
        triangle->_points[i]._y = 0.0f;
        triangle->_points[i]._z = 0.0f;
    } 
}

void draw_triangle3D(Field_t* field, const Triangle3D_t* triangle, const Matrix4x4_t* m, char filled_symbol) {
    Triangle3D_t projected;
    initialize_triangle3d(&projected);

    multiplyMatrixVector(&(triangle->_points[0]), &(projected._points[0]), m);
    multiplyMatrixVector(&(triangle->_points[1]), &(projected._points[1]), m);
    multiplyMatrixVector(&(triangle->_points[2]), &(projected._points[2]), m);

    Triangle_t triangle2D;

    triangle2D._points[0]._x = projected._points[0]._x;
    triangle2D._points[0]._y = projected._points[0]._y;
    triangle2D._points[1]._x = projected._points[1]._x;
    triangle2D._points[1]._y = projected._points[1]._y;
    triangle2D._points[2]._x = projected._points[2]._x;
    triangle2D._points[2]._y = projected._points[2]._y;


    // triangle2D._points[0]._x = triangle->_points[0]._x;
    // triangle2D._points[0]._y = triangle->_points[0]._y;
    // triangle2D._points[1]._x = triangle->_points[1]._x;
    // triangle2D._points[1]._y = triangle->_points[1]._y;
    // triangle2D._points[2]._x = triangle->_points[2]._x;
    // triangle2D._points[2]._y = triangle->_points[2]._y;



    draw_triangle(field, &triangle2D, filled_symbol);
}

void draw_cube(Field_t* field, const Cube_t* cube, const Matrix4x4_t* m, char filled_symbol) {
    for (int i = 0; i < 12; ++i) 
        draw_triangle3D(field, &(cube->_triangles[i]), m, filled_symbol);
}

void initialize_cube(Cube_t* cube) {
    for (int i = 0; i < 12; ++i)
        initialize_triangle3d(&(cube->_triangles[i]));
}

int main() {    // TODO: arguments including delay between frames
    // hidecursor();
    const int h = 24;
    const int w = 100;
    const float aspectRatio = (float)h / (float)w;

    const float zNear = 0.1f;
    const float zFar = 1000.0f;
    const float fov = 60.0f;

    const float fovRadian = 1.0f / tanf(fov * 0.5f / 180.0f * 3.14159f);
    Matrix4x4_t projectionMatrix;
    for (int row = 0; row < 4; ++row)
        for (int column = 0; column < 4; ++column)
            projectionMatrix._data[row][column] = 0.0f;
    projectionMatrix._data[0][0] = aspectRatio * fovRadian;
    projectionMatrix._data[1][1] = fovRadian;
    projectionMatrix._data[2][2] = zFar / (zFar - zNear);
    projectionMatrix._data[3][2] = (-zFar * zNear) / (zFar - zNear);
    projectionMatrix._data[2][3] = 1.0f;



    Cube_t cube1;
    initialize_cube(&cube1);
    create_cube(&cube1);





    Field_t field;
    allocate_field(&field, h, w);

    initialize_field(&field, '.');

    draw_cube(&field, &cube1, &projectionMatrix, '*');
    // Line_t h1;
    // h1._point1._x = -0.5f;
    // h1._point1._y =  0.0f;
    // h1._point2._x =  0.5f;
    // h1._point2._y =  0.0f;

    // Line_t h2;
    // h2._point1._x = -10.5f;
    // h2._point1._y = -10.0f;
    // h2._point2._x =  0.5f;
    // h2._point2._y =  0.5f;

    // Line_t v1;
    // v1._point1._x = -0.05f;
    // v1._point1._y = -0.75f;
    // v1._point2._x =  0.05f;
    // v1._point2._y =  0.75f;

    // Line_t v2;
    // v2._point1._x = -5.05f;
    // v2._point1._y = -20.75f;
    // v2._point2._x =  0.05f;
    // v2._point2._y =  0.75f;

    // draw_line(&field, &v2, '*');

    Triangle_t t1;
    t1._points[0]._x = 0.5f;
    t1._points[0]._y = 0.5f;

    t1._points[1]._x = -0.9f;
    t1._points[1]._y = -0.9f;

    t1._points[2]._x = -0.9f;
    t1._points[2]._y =  0.9f;

    // draw_triangle(&field, &t1, '*');

    for (int i = 0; i < 1; ++i) {
        output_frame(&field);
        // goto_xy(0, 0);
    }

    deallocate_field(&field);
}