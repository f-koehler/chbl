#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

enum OP { OP_ADD = 0, OP_SUB, OP_SET };

enum UNIT { UNIT_PERCENT = 0, UNIT_ABS };

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: chbl NAME OPERATION\n");
        exit(EXIT_FAILURE);
    }

    enum OP operation;
    char *remaining_op = NULL;
    if (argv[2][0] == '+') {
        operation = OP_ADD;
        remaining_op = (char *)malloc((strlen(argv[2]) - 1) * sizeof(char));
        strcpy(remaining_op, argv[2] + 1);
    } else if (argv[2][0] == '-') {
        operation = OP_SUB;
        remaining_op = (char *)malloc((strlen(argv[2]) - 1) * sizeof(char));
        strcpy(remaining_op, argv[2] + 1);
    } else if (isdigit(argv[2][0])) {
        operation = OP_SET;
        remaining_op = (char *)malloc(strlen(argv[2]) * sizeof(char));
        strcpy(remaining_op, argv[2]);
    } else {
        fprintf(stderr,
                "The operation string must start with +, -, or a number!\n");
        exit(EXIT_FAILURE);
    }

    size_t len_remaining_op = strlen(remaining_op);
    enum UNIT unit;
    double value_percent;
    int value_abs;
    if (remaining_op[len_remaining_op - 1] == '%') {
        unit = UNIT_PERCENT;
        if (sscanf(remaining_op, "%lf%%", &value_percent) != 1) {
            fprintf(stderr, "Failed to extract value from operation string!\n");
            exit(EXIT_FAILURE);
        }
    } else if (isdigit(remaining_op[len_remaining_op - 1])) {
        unit = UNIT_ABS;
        if (sscanf(remaining_op, "%d", &value_abs) != 1) {
            fprintf(stderr, "Failed to extract value from operation string!\n");
            exit(EXIT_FAILURE);
        }
    } else {
        fprintf(stderr, "The operation string must end with %% or a number!\n");
        exit(EXIT_FAILURE);
    }

    const char *sys_path = "/sys/class/backlight/";
    size_t len_sys_path = strlen(sys_path);
    size_t len_backlight = strlen(argv[1]);
    char *backlight_path =
        (char *)malloc((len_sys_path + len_backlight) * sizeof(char));
    strcpy(backlight_path, sys_path);
    strcat(backlight_path, argv[1]);
    size_t len_backlight_path = strlen(backlight_path);

    const char *brightness_file = "/brightness";
    size_t len_brightness_file = strlen(brightness_file);
    char *brightness_path = (char *)malloc(
        (len_backlight_path + len_brightness_file) * sizeof(char));
    strcpy(brightness_path, backlight_path);
    strcat(brightness_path, brightness_file);

    const char *max_brightness_file = "/max_brightness";
    size_t len_max_brightness_file = strlen(max_brightness_file);
    char *max_brightness_path = (char *)malloc(
        (len_backlight_path + len_max_brightness_file) * sizeof(char));
    strcpy(max_brightness_path, backlight_path);
    strcat(max_brightness_path, max_brightness_file);

    if (access(max_brightness_path, R_OK)) {
        fprintf(stderr, "No read access to file \"%s\"!\n",
                max_brightness_path);
        exit(EXIT_FAILURE);
    }

    if (access(brightness_path, W_OK)) {
        fprintf(stderr, "No write access to file \"%s\"!\n", brightness_path);
        exit(EXIT_FAILURE);
    }

    FILE *fp_max_brightness = fopen(max_brightness_path, "r");
    int max_brightness = 0;
    if (fp_max_brightness != NULL) {
        if (fscanf(fp_max_brightness, "%d", &max_brightness) != 1) {
            fprintf(stderr, "Failed to read max brightness!\n");
            fclose(fp_max_brightness);
            exit(EXIT_FAILURE);
        }
    } else {
        fprintf(stderr, "Failed to open file \"%s\" for reading!\n",
                max_brightness_path);
        exit(EXIT_FAILURE);
    }
    fclose(fp_max_brightness);

    FILE *fp_brightness = fopen(brightness_path, "r");
    int current_brightness = 0;
    if (fp_brightness != NULL) {
        if (fscanf(fp_brightness, "%d", &current_brightness) != 1) {
            fprintf(stderr, "Failed to read current brightness!\n");
            fclose(fp_brightness);
            exit(EXIT_FAILURE);
        }
    } else {
        fprintf(stderr, "Failed to open file \"%s\" for reading!\n",
                brightness_path);
        exit(EXIT_FAILURE);
    }
    fclose(fp_brightness);

    int new_brightness = current_brightness;
    if (unit == UNIT_PERCENT) {
        double current_percentage =
            (double)current_brightness / (double)max_brightness * 100.0;
        double new_percentage = current_percentage;
        if (operation == OP_ADD) {
            new_percentage += value_percent;
        } else if (operation == OP_SUB) {
            new_percentage -= value_percent;
        } else {
            new_percentage = value_percent;
        }
        new_brightness =
            (int)round((new_percentage / 100.0) * (double)max_brightness);
    } else {
        if (operation == OP_ADD) {
            new_brightness += value_abs;
        } else if (operation == OP_SUB) {
            new_brightness -= value_abs;
        } else {
            new_brightness = value_abs;
        }
    }
    new_brightness = MAX(MIN(new_brightness, max_brightness), 0);
    printf("%d\n", new_brightness);

    fp_brightness = fopen(brightness_path, "w");
    if (fp_brightness != NULL) {
        fprintf(fp_brightness, "%d", new_brightness);
    } else {
        fprintf(stderr, "Failed to open file \"%s\" for reading!\n",
                brightness_path);
        exit(EXIT_FAILURE);
    }
    fclose(fp_brightness);

    return EXIT_SUCCESS;
}
