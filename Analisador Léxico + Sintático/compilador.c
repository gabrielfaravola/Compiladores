/* ===========================================================================
PROJETO COMPILADORES - FASE 1 (Análise Léxica e Sintática)

Gabriel Pereira Faravola - RA: 10427189
Matheus Veiga Bacetic - RA: 10425638

Compilação: gcc -Wall -Wno-unused-result -g -Og compilador.c -o compilador
=========================================================================== */

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


// || ANALISADOR LÉXICO ||

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

    // Verifica se é uma palavra valida
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
        
        // Verifica se é palavra reservada ou ID
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

    // Verifica se é operador relacional ou atribuição
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

    // Verifica se é delimitador'
    if (c == '(' || c == ')' || c == '[' || c == ']' || c == '{' || 
        c == '}' || c == ',' || c == ':' || c == ';' || c == '.') {
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

// Protótipos das funções do analisador sintático
void erroSintatico(char *mensagem);
void operacaoIdentificador(void);
void comandoIf(void);
void comandoPrint(void);
void comandoRepeticao(void);
void comandoInput(void);
void controleFluxo(void);
void testeRelacional(void);
void expressao(void);
void expMatematica(void);
void termoMultiplicativo(void);
void elementoBase(void);
void criacaoLista(void);
void listaArgumentos(void);
void listaPrint(void);

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

void instrucao() {
    switch (lookahead.tipo) {
        case IDENTIFICADOR:
            // O código decide se é Atribuição ou Chamada de Função pelo próximo token
            // Isso é o que chamamos de Lookahead na prática
            operacaoIdentificador(); 
            break;
        case PALAVRA_RESERVADA:
            if (strcmp(lookahead.lexema, "if") == 0) comandoIf();
            else if (strcmp(lookahead.lexema, "print") == 0) comandoPrint();
            else if (strcmp(lookahead.lexema, "while") == 0 || strcmp(lookahead.lexema, "for") == 0) comandoRepeticao();
            else if (strcmp(lookahead.lexema, "input") == 0) comandoInput(); // Se não for atribuído
            else controleFluxo();
            break;
        default:
            erroSintatico("Início de instrução inválido");
    }
}

void erroSintatico(char *mensagem) {
    fprintf(stderr, "\n[ERRO SINTÁTICO]\nLinha %d: %s\n", lookahead.linha, mensagem);
    if (buffer != NULL) fclose(buffer);
    exit(EXIT_FAILURE);
}

void erroSintaticoLinha(int linha, char *mensagem) {
    fprintf(stderr, "\n[ERRO SINTÁTICO]\nLinha %d: %s\n", linha, mensagem);
    if (buffer != NULL) fclose(buffer);
    exit(EXIT_FAILURE);
}

void listaArgumentos() {
    expressao();
    while (lookahead.tipo == DELIMITADOR && strcmp(lookahead.lexema, ",") == 0) {
        consome(DELIMITADOR);
        expressao();
    }
}

void listaPrint() {
    if (lookahead.tipo == STRING) {
        consome(STRING);
    } else {
        expressao();
    }

    while (lookahead.tipo == DELIMITADOR && strcmp(lookahead.lexema, ",") == 0) {
        consome(DELIMITADOR);
        if (lookahead.tipo == STRING) {
            consome(STRING);
        } else {
            expressao();
        }
    }
}




void operacaoIdentificador() {
    consome(IDENTIFICADOR);

    if (lookahead.tipo == DELIMITADOR && strcmp(lookahead.lexema, "(") == 0) {
        consome(DELIMITADOR);
        if (!(lookahead.tipo == DELIMITADOR && strcmp(lookahead.lexema, ")") == 0)) {
            listaArgumentos();
        }
        consome(DELIMITADOR);
        return;
    }

    if (lookahead.tipo == DELIMITADOR && strcmp(lookahead.lexema, "[") == 0) {
        consome(DELIMITADOR);
        expressao();
        consome(DELIMITADOR); // ']'
    }

    if (lookahead.tipo == DELIMITADOR && strcmp(lookahead.lexema, "=") == 0) {
        consome(DELIMITADOR);
        expressao();
        return;
    }

    erroSintatico("Esperado '=' ou chamada de função após identificador");
}

void comandoIf() {
    int linhaIf = lookahead.linha;
    consome(PALAVRA_RESERVADA); // if
    expressao();
    if (!(lookahead.tipo == DELIMITADOR && strcmp(lookahead.lexema, ":") == 0)) {
        erroSintaticoLinha(linhaIf, "Esperado ':' após expressão do if");
    }
    consome(DELIMITADOR);
    instrucao();

    while (lookahead.tipo == PALAVRA_RESERVADA && strcmp(lookahead.lexema, "elif") == 0) {
        int linhaElif = lookahead.linha;
        consome(PALAVRA_RESERVADA);
        expressao();
        if (!(lookahead.tipo == DELIMITADOR && strcmp(lookahead.lexema, ":") == 0)) {
            erroSintaticoLinha(linhaElif, "Esperado ':' após expressão do elif");
        }
        consome(DELIMITADOR);
        instrucao();
    }

    if (lookahead.tipo == PALAVRA_RESERVADA && strcmp(lookahead.lexema, "else") == 0) {
        int linhaElse = lookahead.linha;
        consome(PALAVRA_RESERVADA);
        if (!(lookahead.tipo == DELIMITADOR && strcmp(lookahead.lexema, ":") == 0)) {
            erroSintaticoLinha(linhaElse, "Esperado ':' após else");
        }
        consome(DELIMITADOR);
        instrucao();
    }
}

void comandoRepeticao() {
    if (strcmp(lookahead.lexema, "while") == 0) {
        consome(PALAVRA_RESERVADA);
        expressao();
        if (!(lookahead.tipo == DELIMITADOR && strcmp(lookahead.lexema, ":") == 0)) {
            erroSintatico("Esperado ':' após expressão do while");
        }
        consome(DELIMITADOR);
        instrucao();
    } else {
        consome(PALAVRA_RESERVADA); // for
        consome(IDENTIFICADOR);
        if (!(lookahead.tipo == OPERADOR_LOGICO && strcmp(lookahead.lexema, "in") == 0)) {
            erroSintatico("Esperado 'in' após identificador do for");
        }
        consome(OPERADOR_LOGICO);
        if (!(lookahead.tipo == PALAVRA_RESERVADA && strcmp(lookahead.lexema, "range") == 0)) {
            erroSintatico("Esperado 'range' no for");
        }
        consome(PALAVRA_RESERVADA);
        if (!(lookahead.tipo == DELIMITADOR && strcmp(lookahead.lexema, "(") == 0)) {
            erroSintatico("Esperado '(' após range");
        }
        consome(DELIMITADOR);
        expressao();
        if (!(lookahead.tipo == DELIMITADOR && strcmp(lookahead.lexema, ")") == 0)) {
            erroSintatico("Esperado ')' após expressão do for");
        }
        consome(DELIMITADOR);
        if (!(lookahead.tipo == DELIMITADOR && strcmp(lookahead.lexema, ":") == 0)) {
            erroSintatico("Esperado ':' após expressão do for");
        }
        consome(DELIMITADOR);
        instrucao();
    }
}

void comandoPrint() {
    consome(PALAVRA_RESERVADA);
    if (!(lookahead.tipo == DELIMITADOR && strcmp(lookahead.lexema, "(") == 0)) {
        erroSintatico("Esperado '(' após print");
    }
    consome(DELIMITADOR);
    listaPrint();
    if (!(lookahead.tipo == DELIMITADOR && strcmp(lookahead.lexema, ")") == 0)) {
        erroSintatico("Esperado ')' ao final do print");
    }
    consome(DELIMITADOR);
}

void comandoInput() {
    consome(PALAVRA_RESERVADA);
    if (!(lookahead.tipo == DELIMITADOR && strcmp(lookahead.lexema, "(") == 0)) {
        erroSintatico("Esperado '(' após input");
    }
    consome(DELIMITADOR);
    if (lookahead.tipo != STRING) {
        erroSintatico("Esperado string literal em input");
    }
    consome(STRING);
    if (!(lookahead.tipo == DELIMITADOR && strcmp(lookahead.lexema, ")") == 0)) {
        erroSintatico("Esperado ')' após string de input");
    }
    consome(DELIMITADOR);
}

void controleFluxo() {
    if (strcmp(lookahead.lexema, "break") == 0 || strcmp(lookahead.lexema, "continue") == 0) {
        consome(PALAVRA_RESERVADA);
        return;
    }

    if (strcmp(lookahead.lexema, "return") == 0) {
        consome(PALAVRA_RESERVADA);
        if (lookahead.tipo == IDENTIFICADOR || lookahead.tipo == NUMERO || lookahead.tipo == BOOLEANO ||
            (lookahead.tipo == DELIMITADOR && (strcmp(lookahead.lexema, "(") == 0 || strcmp(lookahead.lexema, "[") == 0)) ||
            (lookahead.tipo == PALAVRA_RESERVADA && strcmp(lookahead.lexema, "len") == 0)) {
            expressao();
        }
        return;
    }

    erroSintatico("Comando de fluxo inválido");
}

void testeRelacional() {
    expMatematica();
    if (lookahead.tipo == OPERADOR_RELACIONAL ||
        (lookahead.tipo == OPERADOR_LOGICO && (strcmp(lookahead.lexema, "is") == 0 || strcmp(lookahead.lexema, "in") == 0))) {
        if (lookahead.tipo == OPERADOR_RELACIONAL) {
            consome(OPERADOR_RELACIONAL);
        } else {
            consome(OPERADOR_LOGICO);
        }
        expMatematica();
    }
}

void criacaoLista() {
    consome(DELIMITADOR); // '['
    if (!(lookahead.tipo == DELIMITADOR && strcmp(lookahead.lexema, "]") == 0)) {
        listaArgumentos();
    }
    consome(DELIMITADOR); // ']'
}

void expressao() {
    testeRelacional();
    while (lookahead.tipo == OPERADOR_LOGICO && 
          (strcmp(lookahead.lexema, "and") == 0 || strcmp(lookahead.lexema, "or") == 0)) {
        consome(OPERADOR_LOGICO);
        testeRelacional();
    }
}

void expMatematica() {
    termoMultiplicativo();
    while (lookahead.tipo == OPERADOR_ARITMETICO && 
          (strcmp(lookahead.lexema, "+") == 0 || strcmp(lookahead.lexema, "-") == 0)) {
        consome(OPERADOR_ARITMETICO);
        termoMultiplicativo();
    }
}

void termoMultiplicativo() {
    if (lookahead.tipo == OPERADOR_LOGICO && strcmp(lookahead.lexema, "not") == 0) {
        consome(OPERADOR_LOGICO);
    }
    elementoBase();
    while (lookahead.tipo == OPERADOR_ARITMETICO && 
          (strcmp(lookahead.lexema, "*") == 0 || strcmp(lookahead.lexema, "**") == 0 || strcmp(lookahead.lexema, "%") == 0)) {
        consome(OPERADOR_ARITMETICO);
        elementoBase();
    }
}

void elementoBase() {
    switch (lookahead.tipo) {
        case IDENTIFICADOR: {
            // VOLTAMOS AO COMPORTAMENTO PURAMENTE SINTÁTICO
            // Ele apenas consome o token e avança, sem perguntar se já tem valor.
            consome(IDENTIFICADOR); 
            
            if (lookahead.tipo == DELIMITADOR && strcmp(lookahead.lexema, "[") == 0) {
                consome(DELIMITADOR);
                expressao();
                if (strcmp(lookahead.lexema, "]") == 0) {
                    consome(DELIMITADOR);
                } else {
                    erroSintatico("Esperado ']' após o índice");
                }
            }
            break;
        }
        case NUMERO:
            consome(NUMERO);
            break;
        case BOOLEANO:
            consome(BOOLEANO);
            break;
        case DELIMITADOR:
            if (strcmp(lookahead.lexema, "(") == 0) {
                consome(DELIMITADOR);
                expressao();
                if (strcmp(lookahead.lexema, ")") == 0) {
                    consome(DELIMITADOR);
                } else {
                    erroSintatico("Esperado ')' fechando a expressão");
                }
            } else if (strcmp(lookahead.lexema, "[") == 0) {
                criacaoLista();
            } else {
                // AQUI ESTAVA O FURO! Se for ':' ou qualquer outro delimitador solto, tem que quebrar!
                erroSintatico("Delimitador inesperado. Esperava-se o início de uma expressão válida.");
            }
            break;
        case PALAVRA_RESERVADA:
            if (strcmp(lookahead.lexema, "len") == 0) {
                consome(PALAVRA_RESERVADA);
                if (strcmp(lookahead.lexema, "(") == 0) consome(DELIMITADOR);
                else erroSintatico("Esperado '(' após len");
                
                expressao();
                
                if (strcmp(lookahead.lexema, ")") == 0) consome(DELIMITADOR);
                else erroSintatico("Esperado ')' fechando len");
                
            } else if (strcmp(lookahead.lexema, "input") == 0) {
                consome(PALAVRA_RESERVADA);
                if (strcmp(lookahead.lexema, "(") == 0) consome(DELIMITADOR);
                else erroSintatico("Esperado '(' após input");
                
                if (lookahead.tipo != STRING) erroSintatico("Esperado string literal em input");
                consome(STRING);
                
                if (strcmp(lookahead.lexema, ")") == 0) consome(DELIMITADOR);
                else erroSintatico("Esperado ')' fechando input");
            } else {
                // AQUI TAMBÉM! Palavras como 'elif' ou 'else' não podem entrar soltas numa conta.
                erroSintatico("Palavra reservada inesperada no lugar de uma expressão.");
            }
            break;
        default:
            erroSintatico("Elemento base inválido na expressão");
    }
}


//Analisador Semântico 

//tabela de símbolos
typedef struct _TNo {
char ID [16];
int endereco;
char tipo [7];
struct _TNo *prox;
} TNo;

int main(int argv, char *argc[]){
    // 1. Verificação de segurança da inicialização
    if(argv != 2){  
        printf("Exemplo de execução: %s <arquivo.py>\n", argc[0]);
        return 1;
    }

    // 2. Prepara o arquivo fonte para leitura
    abrirArquivos(argc[1]);

    // 3. Pega o primeiro token e armazena na variável global para o sintático começar a avaliar
    lookahead = obter_atomo();

    // 4. O LOOP PRINCIPAL (A regra ProgramaMiniPython)
    // Enquanto o token atual não for o fim do arquivo (EOS), continue lendo instruções
    while (lookahead.tipo != EOS) {
        instrucao(); // Chama a função raiz que criamos no Sintático
    }

    // 5. SUCESSO
    // Se o while terminar sem que nenhum exit(1) tenha sido disparado por erro, 
    // significa que o código MiniPython está com a sintaxe correta
    printf("\n[SUCESSO] Análise Léxica e Sintática concluídas. Nenhum erro encontrado!\n");

    // Limpa a memória fechando o arquivo
    if (buffer != NULL) {
        fclose(buffer);
    }

    return 0;
}
