#include <math.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <parser.h>

int
parse_digits(Parser parser, int* count) {
    *count = 0;
    if (! is_digit(spsps_peek(parser))) {
        SPSPS_ERR(parser, "Expected to find a digit, but instead found %s.",
                  spsps_printchar(spsps_peek(parser)));
        return 0;
    }
    int value = 0;
    do {
        value *= 10;
        value += spsps_consume(parser) - '0';
        *count += 1;
    } while (isdigit(spsps_peek(parser)));
    return value;
}

double
parse_double(Parser parser) {
    bool neg = spsps_peek_and_consume(parser, "-");
    int digits = 0;
    double value = (double) parse_digits(parser, &digits);
    if (digits == 0) {
        return NAN;
    }
    if (spsps_peek_and_consume(parser, ".")) {
        // We have found a fractional part.  Parse it.
        double fracpart = (double) parse_digits(parser, &digits);
        if (digits == 0) {
            return NAN;
        }
        value += fracpart / pow(10, digits);
    }
    if (spsps_peek_and_consume(parser, "E")
            || spsps_peek_and_consume(parser, "e")) {
        // We have found an exponent part.  Parse it.
        bool negexp = false;
        if (spsps_peek_and_consume(parser, "-")) {
            negexp = true;
        } else {
            spsps_peek_and_consume(parser, "+");
        }
        double exppart = (double) parse_digits(parser, &digits);
        if (digits == 0) {
            return NAN;
        }
        if (negexp) exppart = -exppart;
        value *= pow(10, exppart);
    }
    return (neg ? -value : value);
}

int
main(int argc, char * argv[]) {
    Parser parser = spsps_new("(console)", stdin);
    spsps_consume_whitespace(parser);
    double value = parse_double(parser);
    printf("Parsed: %lf\n", value);
    spsps_free(parser);
    return 0;
}
