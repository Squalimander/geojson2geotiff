#ifndef GEOJSON_H

#define GEOJSON_H
#include <stddef.h>

typedef struct {double x,y; } geoj_point;
typedef struct {geoj_point *points; size_t n;} geoj_ring;
typedef struct {geoj_ring *rings; size_t n;} geoj_poly;

typedef struct {geoj_poly *polys; size_t n; double attr_num; int has_attr;} geoj_feature;

typedef struct {geoj_feature *features; size_t n; double minx, miny, maxx, maxy;} geoj_dataset;

#endif
