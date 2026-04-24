#include "cli.h"
#include <getopt.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *USAGE =
    "Usage: %s INPUT.geojson OUTPUT.tif [options]\n"
    "\n"
    "Rasterize GeoJSON polygons into a GeoTIFF.\n"
    "\n"
    "Required:\n"
    "  --attribute NAME              Feature property to burn as pixel value.\n"
    "\n"
    "Grid (exactly one required):\n"
    "  --resolution DEG              Pixel size in degrees (EPSG:4326).\n"
    "  --match-raster TEMPLATE.tif   Inherit grid from a reference GeoTIFF.\n"
    "\n"
    "Burn options:\n"
    "  --overlap MODE                last|first|max|min|average  (default: last)\n"
    "  --all-touched                 Burn every pixel touched by a polygon edge.\n"
    "\n"
    "Output options:\n"
    "  --bounds minx,miny,maxx,maxy  Clip/extend to these bounds.\n"
    "  --dtype TYPE                  uint8|int16|int32|float32   (default: int32)\n"
    "  --nodata VAL                  NODATA value (default: dtype sentinel)\n"
    "  --compress MODE               deflate|lzw|none            (default: deflate)\n"
    "\n"
    "Performance options:\n"
    "  --tile-size N                 Tile edge length in pixels (default: 512)\n"
    "  --threads N                   Worker threads (default: auto)\n"
    "\n"
    "  -h, --help                    Show this help and exit.\n"
    "      --version                 Print version and exit.\n";

void cli_print_usage(const char *progname) {
    fprintf(stderr, USAGE, progname ? progname : "geojson2geotiff");
}
