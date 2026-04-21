#ifndef CLI_H

#define CLI_H
#include <stdbool.h>

typedef enum overlap_mode {
    OVERLAP_LAST,
    OVERLAP_FIRST,
    OVERLAP_MAX,
    OVERLAP_MIN,
    OVERLAP_AVERAGE
} overlap_mode_t;

typedef enum dtype {
    DTYPE_UINT8,
    DTYPE_INT16,
    DTYPE_INT32,
    DTYPE_FLOAT32
} dtype_t;

typedef enum compress {
    COMPRESS_NONE,
    COMPRESS_DEFLATE,
    COMPRESS_LZW
} compress_t;

typedef struct cli_opts {
    const char *input_path;
    const char *output_path;
    const char *attribute;
    overlap_mode_t overlap;
    double resolution;
    const char *match_raster;
    bool has_bounds;
    double bounds[4];
    dtype_t dtype;
    bool has_nodata;
    double nodata;
    bool all_touched;
    int tile_size;
    int threads;
    compress_t compress;
} cli_opts_t;

//cli_parse returns 0 for run 1 for help or version -1 for error
int cli_parse(int argc, char **argv, cli_opts_t *opts);
void cli_print_usage(const char *progname);

#endif

