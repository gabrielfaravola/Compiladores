/*
gcc -Wall -Wno-unused-result -g -Og compilador.c -o compilador
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define Total_IDs 1000 // Define o maximo de IDs disponiveis na tabela de simbolos

typedef enum{
    ERRO,
    IDENTIFICADOR,
    PALAVRA_RESERVADA,
    STRING,
    NUMERO,
    DELIMITADOR,
    OPERADOR_BOOLEAN,
    OPERADOR_ARITMETICO,
    OPERADOR_RELACIONAL,
    OPERADOR_LOGICO,
    DOIS_PONTOS,
    EOS
}TAtomo;

typedef struct{     // Estrutura do token
    char lexema[100];
    int linha;
    TAtomo tipo;
}Token;

FILE *fonte;
int linhaAtual = 1;

// Função para abrir arquivo
void leituraArquivo(char *arquivo){
    fonte = fopen(arquivo, "r");
    if(fonte == NULL){
        perror("Erro ao abrir o arquivo");
    }
}

// Retorna o proximo caracter válido, ignorando espaços, cometários, e \n(s)
char proximoCharValido() {
    char c;

    while ((c = fgetc(fonte)) != EOF) {
        if (c == '\n') {
            linhaAtual++;
            continue;
        }

        if (isspace(c)) {
            continue;
        }

        if (c == '#') {
            while ((c = fgetc(fonte)) != '\n' && c != EOF);
            if (c == '\n') linhaAtual++;
            continue;
        }

        return c; // Caracter validado
    }

    return EOF;
}

void erroLexico(char *lexema) {
    fprintf(stderr, "\n[ERRO LEXICO]\n");
    fprintf(stderr, "Linha: %d\n", linhaAtual);
    fprintf(stderr, "Token invalido: \"%s\"\n", lexema);
    fprintf(stderr, "Encerrando analise...\n");

    if (fonte != NULL) {
        fclose(fonte);
    }

    exit(EXIT_FAILURE);
}

// Verifica se é BOOLEAN, LÓGICO, PALAVRA RESERVADA ou ID
TAtomo classificarLexema(char *lexema) {
    const char *keywords [] = {
        "return", "from", "while", "as", "elif", "with", "else", "if", 
        "break", "len", "input", "print", "exec", "raise", "continue", 
        "range", "def", "for"
    };

    const char *logicos[] = {
        "and", "or", "not", "in", "is"
    };

    int numkeywords  = sizeof(keywords ) / sizeof(keywords [0]);
    int numLogicos = sizeof(logicos) / sizeof(logicos[0]);

    // Booleanos
    if (strcmp(lexema, "True") == 0 || strcmp(lexema, "False") == 0) {
        return OPERADOR_BOOLEAN;
    }

    // Operadores lógicos
    for (int i = 0; i < numLogicos; i++) {
        if (strcmp(lexema, logicos[i]) == 0) {
            return OPERADOR_LOGICO;
        }
    }

    // Palavras keywords 
    for (int i = 0; i < numkeywords ; i++) {
        if (strcmp(lexema, keywords [i]) == 0) {
            return PALAVRA_RESERVADA;
        }
    }

    // Identificador padrão
    return IDENTIFICADOR;
}

Token obter_atomo(){
    Token atomo;
    atomo.tipo = ERRO;
    atomo.lexema[0] = '\0';
    atomo.linha = linhaAtual;
    
    bool erro = false;
    int i = 0;
    char c = proximoCharValido();
    atomo.linha = linhaAtual;


    if (c == EOF) { // Define o ultimo atomo de final do arquivo
        atomo.tipo = EOS;
        strcpy(atomo.lexema, "EOF");
        return atomo;
    }

    // Verifica se é numero
    if (isdigit(c)) {
        atomo.lexema[i++] = c;

        while ((c = fgetc(fonte)) != EOF && !isspace(c) && c != '\n') {

            if (isalpha(c) || c == '_') {
                erro = true;
                atomo.lexema[i++] = c;
                continue;
            }

            // se for símbolo tipo ) , + - etc -> encerra token
            if (!isdigit(c)) {
                ungetc(c, fonte);
                break;
            }

            atomo.lexema[i++] = c;
        }

        atomo.lexema[i] = '\0';
        atomo.tipo = erro ? ERRO : NUMERO;

        return atomo;
    }

    // Verifica se é palavra reservada ou ID
    if (isalpha(c) || c == '_') {
        atomo.lexema[i++] = c;
        while ((c = fgetc(fonte)) != EOF && (isalnum(c) || c == '_')){
            atomo.lexema[i++] = c;
        }
        ungetc(c, fonte); 
        atomo.lexema[i] = '\0'; 
        
        atomo.tipo = classificarLexema(atomo.lexema);

//        printToken(atomo);

        return atomo;
    }

    // Verifica se é string
    if (c == '"' || c == '\'') {
        char delimitador_string = c; 
        atomo.lexema[i++] = c;
        
        while ((c = fgetc(fonte)) != delimitador_string && c != EOF && c != '\n') {
            atomo.lexema[i++] = c;
        }
        
        if (c == delimitador_string) {
            atomo.lexema[i++] = c; 
            atomo.lexema[i] = '\0';
            atomo.tipo = STRING;
        } else {
            atomo.lexema[i] = '\0';
            atomo.tipo = ERRO;
        }

        if (atomo.tipo != EOS && atomo.tipo != ERRO) {
    //        printToken(atomo);

        }
        return atomo;
    }

    // Verifica se é operador relacional ou de simples "=" para atribuição
    if (c == '=' || c == '<' || c == '>' || c == '!') {
        atomo.lexema[i++] = c;
        char prox = fgetc(fonte); 
        
        if (prox == '=') {
            atomo.lexema[i++] = prox;
            atomo.lexema[i] = '\0';
            atomo.tipo = OPERADOR_RELACIONAL;

    //        printToken(atomo);

            return atomo;
        }
        
        ungetc(prox, fonte);
        atomo.lexema[i] = '\0';
        
        if (c == '!') {
            atomo.tipo = ERRO; 
        } else if (c == '=') {
            atomo.tipo = DELIMITADOR; 
        } else {
            atomo.tipo = OPERADOR_RELACIONAL; 
        }
        
        if (atomo.tipo != EOS && atomo.tipo != ERRO) {
    //        printToken(atomo);

        }
        return atomo;
    }

    // Verifica se é operador aritmético
    if (c == '+' || c == '-' || c == '*' || c == '/' || c == '%') {
        atomo.lexema[i++] = c;
        
        if (c == '*') {
            char prox = fgetc(fonte);
            if (prox == '*') {
                atomo.lexema[i++] = prox; 
            } else {
                ungetc(prox, fonte); 
            }
        }
        
        atomo.lexema[i] = '\0';
        atomo.tipo = OPERADOR_ARITMETICO;

//        printToken(atomo);

        return atomo;
    }

    // Verifica se é delimitador
    if (c == '(' || c == ')' || c == '[' || c == ']' || c == '{' || c == '}' || c == ',') {
        atomo.lexema[i++] = c;
        atomo.lexema[i] = '\0';
        atomo.tipo = DELIMITADOR;

//        printToken(atomo);

        return atomo;
    }
    
    // Verifica ":"
    if (c == ':') {
        atomo.lexema[i++] = c;
        atomo.lexema[i] = '\0';
        atomo.tipo = DOIS_PONTOS;

//        printToken(atomo);

        return atomo;
    }

    // Se nao for nada = ERRO
    atomo.lexema[0] = c;
    atomo.lexema[1] = '\0';
    atomo.tipo = ERRO;
    
    erroLexico(atomo.lexema);
    return atomo;
}










int main(int argv, char *argc[]){
    if(argv != 2){  // Se o executavel não receber parametro, já finaliza
        printf("Exemplo de execução: %s <arquivo.py>\n", argc[0]);
        return 1;
    }

    leituraArquivo(argc[1]);

    
    Token token = obter_atomo();
    printf("%s, %d, %d\n", token.lexema, token.linha, token.tipo);

    return 0;
}