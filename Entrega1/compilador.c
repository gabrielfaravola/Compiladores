/*
gcc -Wall -Wno-unused-result -g -Og compilador.c -o compilador
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define Total_IDs 50 // Define o maximo de IDs disponiveis na tabela de simbolos


typedef enum{
    ERRO,
    IDENTIFICADOR,
    PALAVRA_RESERVADA,
    STRING,
    NUMERO,
    DELIMITADOR,
    BOOLEANO,
    OPERADOR_ARITMETICO,
    OPERADOR_RELACIONAL,
    OPERADOR_LOGICO,
    EOS
}TAtomo;

char* tipoAtomo(TAtomo tipo) {
    switch(tipo) {
        case IDENTIFICADOR: return "IDENTIFICADOR";
        case PALAVRA_RESERVADA: return "PALAVRA_RESERVADA";
        case STRING: return "STRING";
        case NUMERO: return "NUMERO";
        case DELIMITADOR: return "DELIMITADOR";
        case BOOLEANO: return "BOOLEANO";
        case OPERADOR_ARITMETICO: return "OPERADOR_ARITMETICO";
        case OPERADOR_RELACIONAL: return "OPERADOR_RELACIONAL";
        case OPERADOR_LOGICO: return "OPERADOR_LOGICO";
        default: return "DESCONHECIDO";
    }
}

// Estrutura do token
typedef struct{
    char lexema[100];
    int linha;
    TAtomo tipo;
}TInfoAtomo;


// Variaveis globais
FILE *buffer;
FILE *saida;
int linhaAtual = 1;
char tabela_simbolos[Total_IDs][100];
int IDs_definidos = 1;


// Função para abrir arquivos fonte
void abrirArquivos(char *arquivo){
    buffer = fopen(arquivo, "r");
    if(buffer == NULL){
        perror("Erro ao abrir o fonte");
    }

    saida = fopen("saida.txt", "w");
    if(saida == NULL){
        perror("Erro ao criar/abrir saida.txt");
    }
}

// Printa erro lexico e encerra programa
void erroLexico(TInfoAtomo atomo) {
    fprintf(stderr, "\n[ERRO LEXICO]\n");
    fprintf(stderr, "Linha: %d\n", atomo.linha);
    fprintf(stderr, "Token invalido: %s\n", atomo.lexema);
    fprintf(stderr, "Encerrando analise.\n");

    if (buffer != NULL) {
        fclose(buffer);
    }

    exit(EXIT_FAILURE);
}

// Percorre a tabela de simbolos para encontrar se um ID ja foi definido, caso não, insere
int buscarOuInserir(char *lexema) {
    if (IDs_definidos >= Total_IDs) {
        printf("Erro: tabela de simbolos cheia\n");
        exit(1);
    }

    for (int i = 0; i < IDs_definidos; i++) {
        if (strcmp(tabela_simbolos[i], lexema) == 0) {
            return i;
        }
    }

    strcpy(tabela_simbolos[IDs_definidos], lexema);
    return IDs_definidos++;
}

// Printa atomo incluindo seu TIPO escrito
void printAtomo(TInfoAtomo atomo){
    char* TIPO = tipoAtomo(atomo.tipo);

    if(atomo.tipo == IDENTIFICADOR) {
        int linhaTS = buscarOuInserir(atomo.lexema);
        printf("%d# %s | %d\n", atomo.linha, TIPO, linhaTS);
        fprintf(saida, "%d# %s | %d\n", atomo.linha, TIPO, linhaTS);
    } else {
        printf("%d# %s | %s\n", atomo.linha, TIPO, atomo.lexema);
        fprintf(saida, "%d# %s | %s\n", atomo.linha, TIPO, atomo.lexema);
    }
}

// Ignora espaços, cometários, e \n(s)
char proximoCharValido() {
    char c;

    while ((c = fgetc(buffer)) != EOF) {
        if (c == '\n') {
            linhaAtual++;
            continue;
        }

        if (isspace(c)) {
            continue;
        }

        if (c == '#') {
            while ((c = fgetc(buffer)) != '\n' && c != EOF);
            if (c == '\n') linhaAtual++;
            continue;
        }

        return c; // Caracter validado
    }

    return EOF;
}

// Verifica se é BOOLEANO, LÓGICO, PALAVRA RESERVADA ou ID
TAtomo classificarLexema(char *lexema) {
    const char *p_reservada [] = {
        "return", "from", "while", "as", "elif", "with", "else", "if", 
        "break", "len", "input", "print", "exec", "raise", "continue", 
        "range", "def", "for"
    };

    const char *logicos[] = {
        "and", "or", "not", "in", "is"
    };

    // Booleanos
    if (strcmp(lexema, "True") == 0 || strcmp(lexema, "False") == 0) {
        return BOOLEANO;
    }

    int numP_reservada  = sizeof(p_reservada) / sizeof(p_reservada [0]);
    int numLogicos = sizeof(logicos) / sizeof(logicos[0]);

    // Operadores lógicos
    for (int i = 0; i < numLogicos; i++) {
        if (strcmp(lexema, logicos[i]) == 0) {
            return OPERADOR_LOGICO;
        }
    }

    // Palavras reservadas 
    for (int i = 0; i < numP_reservada ; i++) {
        if (strcmp(lexema, p_reservada [i]) == 0) {
            return PALAVRA_RESERVADA;
        }
    }

    // Identificador padrão
    buscarOuInserir(lexema);
    return IDENTIFICADOR;
}

// Retorna um Token a partir do seguinte agrupamentos do buffer
TInfoAtomo obter_atomo(){
    TInfoAtomo atomo;
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
        while ((c = fgetc(buffer)) != EOF && !isspace(c)) {
            if (!isdigit(c)) {
                erro = true;
            }
            atomo.lexema[i++] = c;
        }
        atomo.lexema[i] = '\0';
        
        if (erro) {
            atomo.tipo = ERRO;
            erroLexico(atomo);
        }

        atomo.tipo = NUMERO;
        printAtomo(atomo);
        return atomo;
    }

    // Verifica se é palavra reservada ou ID
    if (isalpha(c) || c == '_') {
        atomo.lexema[i++] = c;

        while ((c = fgetc(buffer)) != EOF && (!isspace(c))){
            if(c != '_' && !isalpha(c) && !isdigit(c)){ 
                erro = true;
            }
            atomo.lexema[i++] = c;
        }
        atomo.lexema[i] = '\0';
        
        if(erro){
            atomo.tipo = ERRO;
            erroLexico(atomo);
        }

        atomo.tipo = classificarLexema(atomo.lexema);
        printAtomo(atomo);
        return atomo;
    }

    // Verifica se é string
    if (c == '"') {
        atomo.lexema[i++] = c;
        while ((c = fgetc(buffer)) != '"' && c != EOF && c != '\n') {
            atomo.lexema[i++] = c;
        }
        
        if (c == '"') {
            atomo.lexema[i++] = c; 
            atomo.lexema[i] = '\0';
            atomo.tipo = STRING;

        } else {
            atomo.lexema[i] = '\0';
            atomo.tipo = ERRO;
        }

        if (atomo.tipo == EOS || atomo.tipo == ERRO) {
            erroLexico(atomo);
        } 

        printAtomo(atomo);
        return atomo;
    }

    // Verifica se é operador relacional ou de simples "=" para atribuição
    if (c == '=' || c == '<' || c == '>' || c == '!') {
        atomo.lexema[i++] = c;
        while((c = fgetc(buffer)) != EOF && !isspace(c)){
            atomo.lexema[i++] = c;
        }
        atomo.lexema[i] = '\0';

        if(strcmp(atomo.lexema, "==") == 0 || strcmp(atomo.lexema, ">=") == 0 || 
           strcmp(atomo.lexema, "<=") == 0 || strcmp(atomo.lexema, "!=") == 0 ||
           strcmp(atomo.lexema, "<") == 0 || strcmp(atomo.lexema, ">") == 0){
            atomo.tipo = OPERADOR_RELACIONAL;
       
        } else if(strcmp(atomo.lexema, "=") == 0){
            atomo.tipo = DELIMITADOR;
        } else {
            atomo.tipo = ERRO;
        }
        
        if (atomo.tipo == EOS || atomo.tipo == ERRO) {
            erroLexico(atomo);
        }

        printAtomo(atomo);
        return atomo;
        
    }

    // Verifica se é operador aritmético
    if (c == '+' || c == '-' || c == '*' || c == '/' || c == '%') {
        atomo.lexema[i++] = c;
        
        if (c == '*') {
            char prox = fgetc(buffer);
            if (prox == '*') {
                atomo.lexema[i++] = prox; 
            } else {
                ungetc(prox, buffer); 
            }
        }
        
        atomo.lexema[i] = '\0';
        atomo.tipo = OPERADOR_ARITMETICO;

        printAtomo(atomo);
        return atomo;
    }

    // Verifica se é delimitador
    if (c == '(' || c == ')' || c == '[' || c == ']' || c == '{' || c == '}' || c == ',' || c == ':') {
        atomo.lexema[i++] = c;
        atomo.lexema[i] = '\0';
        atomo.tipo = DELIMITADOR;

        printAtomo(atomo);
        return atomo;
    }

    // Se nao for nada = ERRO
    atomo.lexema[0] = c;
    atomo.lexema[1] = '\0';
    
    erroLexico(atomo);
    return atomo;
}

// || ANALISADOR SINTÁTICO ||

//Váriavel global lookhead para passar por todos os atómos e verificar 
TInfoAtomo lookahead;

//Função consome que vai comparar cada átomo e avançar
void consome(TAtomo tipo_esperado) {
    if (lookahead.tipo == tipo_esperado) {
        lookahead = obter_atomo();
    } else {
        printf("\n[ERRO SINTÁTICO] Linha %d: Esperava token do tipo %s, mas encontrou %s ('%s')\n", 
               lookahead.linha, 
               tipoAtomo(tipo_esperado), 
               tipoAtomo(lookahead.tipo), 
               lookahead.lexema);
        
        if (buffer != NULL) fclose(buffer);
        exit(EXIT_FAILURE); // Encerra o processo conforme exigido
    }
}

int main(int argv, char *argc[]){
    if(argv != 2){  // Se o executavel não receber parametro, já finaliza
        printf("Exemplo de execução: %s <arquivo.py>\n", argc[0]);
        return 1;
    }

    abrirArquivos(argc[1]);

    obter_atomo();
    obter_atomo();
    obter_atomo();
    obter_atomo();
    obter_atomo();
    obter_atomo();
    obter_atomo();
    obter_atomo();
    obter_atomo();
    obter_atomo();
    obter_atomo();
    obter_atomo();
    obter_atomo();
    obter_atomo();
    obter_atomo();
    obter_atomo();


    return 0;
}

