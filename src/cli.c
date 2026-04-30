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

static int parse_overlap(const char *s, overlap_mode_t *out) {
    if (strcmp(s, "last") == 0)         *out = OVERLAP_LAST;
    else if (strcmp(s, "first") == 0)   *out = OVERLAP_FIRST;
    else if (strcmp(s, "max") == 0)     *out = OVERLAP_MAX;
    else if (strcmp(s, "min") == 0)     *out = OVERLAP_MIN;
    else if (strcmp(s, "average") == 0) *out = OVERLAP_AVERAGE;
    else return -1;
    return 0;
}

static int parse_dtype(const char *s, dtype_t *out) {
    if (strcmp(s, "uint8") == 0)        *out = DTYPE_UINT8;
    else if (strcmp(s, "int16") == 0)   *out = DTYPE_INT16;
    else if (strcmp(s, "int32") == 0)   *out = DTYPE_INT32;
    else if (strcmp(s, "float32") == 0) *out = DTYPE_FLOAT32;
    else return -1;
    return 0;
}

static int parse_compress(const char *s, compress_t *out) {
    if (strcmp(s, "none") == 0)         *out = COMPRESS_NONE;
    else if (strcmp(s, "deflate") == 0) *out = COMPRESS_DEFLATE;
    else if (strcmp(s, "lzw") == 0)     *out = COMPRESS_LZW;
    else return -1;
    return 0;
}

static int parse_double(const char *s, double *out) {
    char *end;
    double v = strtod(s, &end);
    if (end == s || *end != '\0') return -1;
    *out = v;
    return 0;
}

static int parse_int(const char *s, int *out) {
    char *end;
    long v = strtol(s, &end, 10);
    if (end == s || *end != '\0') return -1;
    *out = (int)v;
    return 0;
}

static int parse_bounds(const char *s, double out[4]) {
    char extra;
    int n = sscanf(s, "%lf,%lf,%lf,%lf%c",
                   &out[0], &out[1], &out[2], &out[3], &extra);
    if (n != 4) return -1;
    if (out[0] >= out[2] || out[1] >= out[3]) return -1;
    return 0;
}

enum {
    OPT_ATTRIBUTE = 256,
    OPT_OVERLAP,
    OPT_RESOLUTION,
    OPT_MATCH_RASTER,
    OPT_BOUNDS,
    OPT_DTYPE,
    OPT_NODATA,
    OPT_ALL_TOUCHED,
    OPT_TILE_SIZE,
    OPT_THREADS,
    OPT_COMPRESS,
    OPT_VERSION,
};

int cli_parse(int argc, char **argv, cli_opts_t *opts) {
    static const struct option longopts[] = {
        {"attribute",    required_argument, 0, OPT_ATTRIBUTE},
        {"overlap",      required_argument, 0, OPT_OVERLAP},
        {"resolution",   required_argument, 0, OPT_RESOLUTION},
        {"match-raster", required_argument, 0, OPT_MATCH_RASTER},
        {"bounds",       required_argument, 0, OPT_BOUNDS},
        {"dtype",        required_argument, 0, OPT_DTYPE},
        {"nodata",       required_argument, 0, OPT_NODATA},
        {"all-touched",  no_argument,       0, OPT_ALL_TOUCHED},
        {"tile-size",    required_argument, 0, OPT_TILE_SIZE},
        {"threads",      required_argument, 0, OPT_THREADS},
        {"compress",     required_argument, 0, OPT_COMPRESS},
        {"help",         no_argument,       0, 'h'},
        {"version",      no_argument,       0, OPT_VERSION},
        {0, 0, 0, 0},
    };

    *opts = (cli_opts_t){
        .overlap     = OVERLAP_LAST,
        .resolution  = NAN,
        .dtype       = DTYPE_INT32,
        .tile_size   = 512,
        .threads     = 0,
        .compress    = COMPRESS_DEFLATE,
    };

    const char *progname = argc > 0 ? argv[0] : "geojson2geotiff";

    int c;
    while ((c = getopt_long(argc, argv, "h", longopts, NULL)) != -1) {
        switch (c) {
        case 'h':
            cli_print_usage(progname);
            return 1;
        case OPT_VERSION:
            printf("geojson2geotiff 0.1.0\n");
            return 1;
        case OPT_ATTRIBUTE:
            opts->attribute = optarg;
            break;
        case OPT_OVERLAP:
            if (parse_overlap(optarg, &opts->overlap) != 0) {
                fprintf(stderr, "error: invalid --overlap '%s'\n", optarg);
                return -1;
            }
            break;
        case OPT_RESOLUTION:
            if (parse_double(optarg, &opts->resolution) != 0 ||
                opts->resolution <= 0.0) {
                fprintf(stderr, "error: invalid --resolution '%s'\n", optarg);
                return -1;
            }
            break;
        case OPT_MATCH_RASTER:
            opts->match_raster = optarg;
            break;
        case OPT_BOUNDS:
            if (parse_bounds(optarg, opts->bounds) != 0) {
                fprintf(stderr, "error: invalid --bounds '%s' "
                                "(expected minx,miny,maxx,maxy)\n", optarg);
                return -1;
            }
            opts->has_bounds = true;
            break;
        case OPT_DTYPE:
            if (parse_dtype(optarg, &opts->dtype) != 0) {
                fprintf(stderr, "error: invalid --dtype '%s'\n", optarg);
                return -1;
            }
            break;
        case OPT_NODATA:
            if (parse_double(optarg, &opts->nodata) != 0) {
                fprintf(stderr, "error: invalid --nodata '%s'\n", optarg);
                return -1;
            }
            opts->has_nodata = true;
            break;
        case OPT_ALL_TOUCHED:
            opts->all_touched = true;
            break;
        case OPT_TILE_SIZE:
            if (parse_int(optarg, &opts->tile_size) != 0 ||
                opts->tile_size <= 0) {
                fprintf(stderr, "error: invalid --tile-size '%s'\n", optarg);
                return -1;
            }
            break;
        case OPT_THREADS:
            if (parse_int(optarg, &opts->threads) != 0 ||
                opts->threads < 0) {
                fprintf(stderr, "error: invalid --threads '%s'\n", optarg);
                return -1;
            }
            break;
        case OPT_COMPRESS:
            if (parse_compress(optarg, &opts->compress) != 0) {
                fprintf(stderr, "error: invalid --compress '%s'\n", optarg);
                return -1;
            }
            break;
        default:
            cli_print_usage(progname);
            return -1;
        }
    }

    if (optind >= argc) {
        fprintf(stderr, "error: missing INPUT.geojson\n");
        cli_print_usage(progname);
        return -1;
    }
    opts->input_path = argv[optind++];

    if (optind >= argc) {
        fprintf(stderr, "error: missing OUTPUT.tif\n");
        cli_print_usage(progname);
        return -1;
    }
    opts->output_path = argv[optind++];

    if (optind < argc) {
        fprintf(stderr, "error: unexpected argument '%s'\n", argv[optind]);
        return -1;
    }

    if (opts->attribute == NULL) {
        fprintf(stderr, "error: --attribute is required\n");
        return -1;
    }

    bool have_res = !isnan(opts->resolution);
    bool have_match = opts->match_raster != NULL;
    if (have_res == have_match) {
        fprintf(stderr,
                "error: specify exactly one of --resolution or --match-raster\n");
        return -1;
    }

    return 0;
}
