#include <stdio.h>
#include <assert.h>

int main(int argc, char** argv) {
    assert(argc == 3);
    FILE* f = fopen(argv[1], "rb");
    printf("    std::string %s = std::string(\"", argv[2]);
    unsigned long n = 0;
	bool esc = false;
    unsigned char c;
    while(!feof(f)) {
        if (fread(&c, 1, 1, f) == 0) break;
        if (c == '"' || c == '\\') {
            printf("\\%c", c);
            esc = false;
        } else if (c == '\t') {
            printf("\\t");
            esc = false;
        } else if (c == '\r') {
            printf("\\r");
            esc = false;
        } else if (c == '\n') {
            printf("\\n");
            esc = false;
        } else if (c >= 0x20 && c <= 0x7e) {
			if (esc && (('0' <= c && c <= '9') || ('a' <= c && c <= 'f') || ('A' <= c && c <= 'F'))) {
                printf("\"\"");
            }
            printf("%c", c);
            esc = false;
        } else {
            printf("\\x%.2x", c);
            esc = true;
        }
		n++;
    }
    fclose(f);
    printf("\", %d);\n", n);
}

