#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <omp.h>

#define ISO_DATE_FORMAT "%Y-%m-%dT%H:%M:%S.000Z"
#define MAX_DATE_LENGTH 64

// Date conversion utilities
typedef struct {
    time_t timestamp;
    char* formatted;
} DateConverter;

// Initialize converter
DateConverter* create_date_converter() {
    DateConverter* converter = malloc(sizeof(DateConverter));
    converter->formatted = malloc(MAX_DATE_LENGTH);
    return converter;
}

// Convert string to date
time_t parse_date(const char* date_str) {
    if (!date_str) {
        return 0;
    }

    struct tm tm = {0};
    
    #pragma omp critical
    {
        if (strptime(date_str, ISO_DATE_FORMAT, &tm) == NULL) {
            // Try alternative formats if ISO format fails
            if (strptime(date_str, "%Y-%m-%d", &tm) == NULL) {
                return 0;
            }
        }
    }

    return timegm(&tm);
}

// Convert date to string
char* format_date(time_t date) {
    if (date == 0) {
        return NULL;
    }

    char* formatted = malloc(MAX_DATE_LENGTH);
    struct tm* tm_info = gmtime(&date);

    #pragma omp critical
    {
        strftime(formatted, MAX_DATE_LENGTH, ISO_DATE_FORMAT, tm_info);
    }

    return formatted;
}

// DateParameterConverter implementation
DateConverter* convert_from_string(const char* value) {
    DateConverter* converter = create_date_converter();
    
    #pragma omp parallel sections
    {
        #pragma omp section
        {
            converter->timestamp = parse_date(value);
        }
        
        #pragma omp section
        {
            if (value) {
                strncpy(converter->formatted, value, MAX_DATE_LENGTH - 1);
                converter->formatted[MAX_DATE_LENGTH - 1] = '\0';
            } else {
                converter->formatted[0] = '\0';
            }
        }
    }
    
    return converter;
}

char* convert_to_string(DateConverter* converter) {
    if (!converter || converter->timestamp == 0) {
        return NULL;
    }
    return format_date(converter->timestamp);
}

// Clean up resources
void free_date_converter(DateConverter* converter) {
    if (converter) {
        free(converter->formatted);
        free(converter);
    }
}