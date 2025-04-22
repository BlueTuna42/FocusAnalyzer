#include <dirent.h>
#include <string.h>

#include "struct.h"

/**
 * scan_bmp_files:
 *  Opens the directory 'path', finds all files with the ".bmp" extension
 *  (case-insensitive), and returns an array of malloc'd strings.
 *  *out_count is set to the number of entries found.
 *  Returns NULL on error.
 */
char **scan_bmp_files(const char *path, size_t *out_count) {
    DIR *dir = opendir(path);
    if (!dir) {
        perror("opendir");  // failed to open directory
        return NULL;
    }

    size_t capacity = 16;
    size_t count = 0;
    char **names = malloc(capacity * sizeof(char*));
    if (!names) {
        perror("malloc");
        closedir(dir);
        return NULL;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        // Skip "." and ".."
        if (entry->d_name[0] == '.' &&
           (entry->d_name[1] == '\0' ||
            (entry->d_name[1] == '.' && entry->d_name[2] == '\0')))
            continue;

        // Find last dot in filename
        char *ext = strrchr(entry->d_name, '.');
        // Check for ".bmp" (case-insensitive)
        if (ext && strcasecmp(ext, ".bmp") == 0) {
            // Grow array if needed
            if (count == capacity) {
                capacity *= 2;
                char **tmp = realloc(names, capacity * sizeof(char*));
                if (!tmp) {
                    perror("realloc");
                    break;
                }
                names = tmp;
            }
            // Store a copy of the filename
            names[count] = strdup(entry->d_name);
            if (!names[count]) {
                perror("strdup");
                break;
            }
            count++;
        }
    }

    closedir(dir);
    *out_count = count;
    return names;
}

