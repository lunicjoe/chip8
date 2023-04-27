#include "assembly.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "chip8.h"

uint8_t get_0x00(uint16_t opcode) {
    return (opcode & 0x0f00) >> 8;
}
uint8_t get_00x0(uint16_t opcode) {
    return (opcode & 0x00f0) >> 4;
}
uint8_t get_00xx(uint16_t opcode) {
    return opcode & 0x00ff;
}
uint16_t get_0xxx(uint16_t opcode) {
    return opcode & 0x0fff;
}

uint8_t* get_rom(FILE *rom_file, long *rom_size) {
    fseek(rom_file, 0, SEEK_END);
    *rom_size = ftell(rom_file);
    rewind(rom_file);
    uint8_t *rom = malloc(*rom_size);
    fread(rom, sizeof(uint8_t), *rom_size, rom_file);
    fclose(rom_file);
    return rom;
}

#define get_0x00 get_0x00(opcode)
#define get_00x0 get_00x0(opcode)
#define get_00xx get_00xx(opcode)
#define get_0xxx get_0xxx(opcode)

char* get_asm_code(uint16_t opcode) {
    char *instruction = malloc(1);
    instruction[0] = '\0';
    switch (opcode & 0xf000) {
        case 0x0000:
            switch (get_00xx) {
                case 0x00e0:
                    set_instruction("cls");
                    break;
                case 0x00ee:
                    set_instruction("ret");
                    break;
            }
            break;
        case 0x1000:
            set_instruction("jmp 0x%X", get_0xxx);
            break;
        case 0x2000:
            set_instruction("call 0x%X", get_0xxx);
            break;
        case 0x3000:
            set_instruction("se V%X, 0x%X", get_0x00, get_00xx);
            break;
        case 0x4000:
            set_instruction("sne V%X, 0x%X", get_0x00, get_00xx);
            break;
        case 0x5000:
            set_instruction("se V%X, V%X", get_0x00, get_00x0);
            break;
        case 0x6000:
            set_instruction("ld V%X, 0x%X", get_0x00, get_00xx);
            break;
        case 0x7000:
            set_instruction("add V%X, 0x%X", get_0x00, get_00xx);
            break;
        case 0x8000:
            switch (opcode & 0x000f) {
                case 0x0:
                    set_instruction("ld V%X, V%X", get_0x00, get_00x0);
                    break;
                case 0x1:
                    set_instruction("or V%X, V%X", get_0x00, get_00x0);
                    break;
                case 0x2:
                    set_instruction("and V%X, V%X", get_0x00, get_00x0);
                    break;
                case 0x3:
                    set_instruction("xor V%X, V%X", get_0x00, get_00x0);
                    break;
                case 0x4:
                    set_instruction("add V%X, V%X", get_0x00, get_00x0);
                    break;
                case 0x5:
                    set_instruction("sub V%X, V%X", get_0x00, get_00x0);
                    break;
                case 0x6:
                    set_instruction("shr V%X", get_0x00);
                    break;
                case 0x7:
                    set_instruction("subn V%X, V%X", get_0x00, get_00x0);
                    break;
                case 0xe:
                    set_instruction("shl V%X", get_0x00);
                    break;
            }
            break;
        case 0x9000:
            set_instruction("sne V%X, V%X", get_0x00, get_00x0);
            break;
        case 0xa000:
            set_instruction("ld I, 0x%X", get_0xxx);
            break;
        case 0xb000:
            set_instruction("jmp V0, 0x%X", get_0xxx)
            break;
        case 0xc000:
            set_instruction("rnd V%X, 0x%02X", get_0x00, get_00xx);
            break;
        case 0xd000:
            set_instruction("drw V%X, V%X, 0x%02X", get_0x00, get_00x0, opcode & 0xf);
            break;
        case 0xe000:
            switch (get_00xx) {
                case 0x9e:
                    set_instruction("skp V%X", get_0x00);
                    break;
                case 0xa1:
                    set_instruction("sknp V%X", get_0x00);
                    break;
            }
        case 0xf000:
            switch (get_00xx) {
                case 0x7:
                    set_instruction("ld V%X, DT", get_0x00);
                    break;
                case 0x0a:
                    set_instruction("ld V%X, KEY", get_0x00);
                    break;
                case 0x15:
                    set_instruction("ld DT, V%X", get_0x00);
                    break;
                case 0x1e:
                    set_instruction("add I, V%X", get_0x00);
                    break;
                case 0x29:
                    set_instruction("font V%X", get_0x00);
                    break;
                case 0x33:
                    set_instruction("bcd V%X", get_0x00);
                    break;
                case 0x55:
                    set_instruction("ld [I], V%X", get_0x00);
                    break;
                case 0x65:
                    set_instruction("ld V%X, [I]", get_0x00);
                    break;
            }
            break;
        default:
            set_instruction("0x%04X", opcode);
    }
    return instruction;
}

uint16_t set_000x(uint16_t number) {
    return number & 0xf;
}
uint16_t set_00xx(uint16_t number) {
    return number & 0xff;
}
uint16_t set_0xxx(uint16_t number) {
    return number & 0xfff;
}
uint16_t set_0x00(uint16_t number) {
    return (number << 8) & 0xf00;
}
uint16_t set_00x0(uint16_t number) {
    return (number << 4) & 0xf0;
}

Instruction instructions[] = {
        {"cls", CLS},
        {"ret", RET},
        {"jmp", JMP},
        {"call", CALL},
        {"se", SE},
        {"sne", SNE},
        {"ld", LD},
        {"add", ADD},
        {"rnd", RND},
        {"drw", DRW},
        {"bcd", BCD},
        {"skp", SKP},
        {"sknp", SKNP},
        {"or", OR},
        {"and", AND},
        {"xor", XOR},
        {"sub", SUB},
        {"shr", SHR},
        {"subn", SUBN},
        {"shl", SHL},
        {"font", FONT},
};
char delimiters[] = " ,";
int label_count = 0;
Label *labels;

instruction_type get_instruction(char *token) {
    for (int i = 0; i < sizeof(instructions) / sizeof(Instruction); i++) {
        if (strcmp(token, instructions[i].string) == 0) return instructions[i].type;
    }
    return OTHER;
}

char *get_line(FILE *code) {
    char *line = malloc(0);
    char c;
    int line_size = 0;
    while ((c = (char)fgetc(code)) != EOF) {
        line = realloc(line, line_size + 1);
        if (c == '\n') {
            line[line_size] = '\0';
            break;
        }
        line[line_size] = c;
        line_size++;
    }
    if (c == EOF) return NULL;
    return line;
}

void append_line(Line **head, char *value) {
    Line *new_line=malloc(sizeof(Line));
    Line *last=*head;
    new_line->value=value;
    new_line->next=NULL;
    if (*head == NULL) {
        *head=new_line;
        return;
    }
    while (last->next != NULL) {
        last=last->next;
    }
    last->next=new_line;
}

Line* get_lines(FILE *code) {
    Line* line = malloc(sizeof(Line));
    int line_count = 0;
    char *line_str;
    while ((line_str = get_line(code)) != NULL) {
        if (line_str[0] == '\0' || line_str[0] == '#') continue;
        for (int i = 0; line_str[i]; i++) {
            line_str[i] = (char)tolower(line_str[i]);
        }
        char *new_line = malloc(strlen(line_str) + 1);
        strcpy(new_line, line_str);
        append_line(&line, new_line);
        line_count++;
    }
    return line;
}

void preprocessor(Line *line) {
    Line *last=line;
    while (last->next) {
        last=last->next;
        get_label(&last->value);
    }
}

void get_label(char **line) {
    static int address=0x200;
    static int i_label=0;
    if ((*line)[0] == ':') {
        (*line)++;
        labels=realloc(labels, sizeof(Label) * (1 + i_label));
        labels[i_label].name=malloc(strlen(*line) + 1);
        strcpy(labels[i_label].name, *line);
        labels[i_label].address=address;
        i_label++;
        *line=NULL;
    } else {
        address+=2;
    }
    label_count = i_label;
}

char **get_tokens(char *line, int *token_count) {
    *token_count = 0;
    char **tokens = malloc(1 * sizeof(char*));
    tokens[*token_count] = strtok(line, delimiters);
    do {
        (*token_count)++;
        tokens = realloc(tokens, (1 + *token_count) * sizeof(char *));
    } while ((tokens[*token_count] = strtok(NULL, delimiters)) != NULL);
    return tokens;
}

char *replace_label(char *token) {
    if (token[0] == '$') {
        token++;
        for (int i_label = 0; i_label < label_count; i_label++) {
            if (strcmp(token, labels[i_label].name) == 0) {
                int str_length = snprintf(NULL, 0, "0x%X", labels[i_label].address);
                char *replaced_token = malloc(str_length + 1);
                sprintf(replaced_token, "0x%X", labels[i_label].address);
                return replaced_token;
            }
        }
    }
    return token;
}

uint16_t get_value(char *token) {
    if (token[0] == '0' && token[1] == 'b') {
        token += 2;
        return strtol(token, NULL, 2);
    }
    return strtol(token, NULL, 0);
}

bool is_register(char *token) {
    return token[0] == 'v';
}

uint16_t get_register(char *token) {
    return strtol(&token[1], NULL, 16);
}

uint16_t get_binary(char **tokens, int token_count) {
    instruction_type instruction = get_instruction(tokens[0]);
    for (int i = 0; i < token_count; i++) {
        tokens[i] = replace_label(tokens[i]);
    }
    switch (instruction) {
        case CLS:
            return 0x00e0;
        case RET:
            return 0x00ee;
        case JMP:
            if (!is_register(tokens[1])) {
                return 0x1000 + set_0xxx(get_value(tokens[1]));
            } else {
                return 0xb000 + set_0xxx(get_value(tokens[2]));
            }
        case CALL:
            return 0x2000 + set_0xxx(get_value(tokens[1]));
        case SE:
            if (is_register(tokens[2])) {
                return 0x5000 + set_0x00(get_register(tokens[1])) + set_00x0(get_register(tokens[2]));
            } else {
                return 0x3000 + set_0x00(get_register(tokens[1])) + set_00xx(get_value(tokens[2]));
            }
        case SNE:
            if (is_register(tokens[2])) {
                return 0x9000 + set_0x00(get_register(tokens[1])) + set_00x0(get_register(tokens[2]));
            } else {
                return 0x4000 + set_0x00(get_register(tokens[1])) + set_00xx(get_value(tokens[2]));
            }
        case LD:
            if (is_register(tokens[1])) {
                if (is_register(tokens[2])) {
                    return 0x8000 + set_0x00(get_register(tokens[1])) + set_00x0(get_register(tokens[2]));
                } else if ((strcmp(tokens[2], "[i]") == 0)) {
                    return 0xf065 + set_0x00(get_register(tokens[1]));
                } else if ((strcmp(tokens[2], "dt") == 0)) {
                    return 0xf007 + set_0x00(get_register(tokens[1]));
                } else if ((strcmp(tokens[2], "key") == 0)) {
                    return 0xf00a + set_0x00(get_register(tokens[1]));
                } else {
                    return 0x6000 + set_0x00(get_register(tokens[1])) + set_00xx(get_value(tokens[2]));
                }
            } else {
                if (strcmp(tokens[1], "i") == 0) {
                    return 0xa000 + set_0xxx(get_value(tokens[2]));
                } else if (strcmp(tokens[1], "dt") == 0) {
                    return 0xf015 + set_0x00(get_register(tokens[2]));
                } else if (strcmp(tokens[1], "[i]") == 0) {
                    return 0xf055 + set_0x00(get_register(tokens[2]));
                }
            }
        case ADD:
            if (!is_register(tokens[2])) {
                return 0x7000 + set_0x00(get_register(tokens[1])) + set_00xx(get_value(tokens[2]));
            } else if (is_register(tokens[1]) && is_register(tokens[2])) {
                return 0x8004 + set_0x00(get_register(tokens[1])) + set_00x0(get_register(tokens[2]));
            } else if (tokens[1][0] == 'i') {
                return 0xf01e + set_0x00(get_register(tokens[2]));
            }
        case RND:
            return 0xc000 + set_0x00(get_register(tokens[1])) + set_00xx(get_value(tokens[2]));
        case DRW:
            return 0xd000 + set_0x00(get_register(tokens[1])) + set_00x0(get_register(tokens[2])) + set_000x(get_value(tokens[3]));
        case BCD:
            return 0xf033 + set_0x00(get_register(tokens[1]));
        case SKP:
            return 0xe09e + set_0x00(get_register(tokens[1]));
        case SKNP:
            return 0xe0a1 + set_0x00(get_register(tokens[1]));
        case OR:
            return 0x8001 + set_0x00(get_register(tokens[1])) + set_00x0(get_register(tokens[2]));
        case AND:
            return 0x8002 + set_0x00(get_register(tokens[1])) + set_00x0(get_register(tokens[2]));
        case XOR:
            return 0x8003 + set_0x00(get_register(tokens[1])) + set_00x0(get_register(tokens[2]));
        case SUB:
            return 0x8005 + set_0x00(get_register(tokens[1])) + set_00x0(get_register(tokens[2]));
        case SHR:
            return 0x8006 + set_0x00(get_register(tokens[1]));
        case SUBN:
            return 0x8007 + set_0x00(get_register(tokens[1])) + set_00x0(get_register(tokens[2]));
        case SHL:
            return 0x800e + set_0x00(get_register(tokens[1]));
        case FONT:
            return 0xf029 + set_0x00(get_register(tokens[1]));
        default:
            return get_value(tokens[0]);
    }
}
