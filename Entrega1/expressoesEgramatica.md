# Especificação Léxica e Sintática - MiniPython

Este documento descreve as Expressões Regulares (ERs) para a identificação dos átomos (tokens) e a Gramática Livre de Contexto (GLC) para a análise sintática da linguagem MiniPython.

## 1. Expressões Regulares (Vocabulário Terminal)

O MiniPython é uma linguagem *case sensitive*. Abaixo estão as definições regulares para os elementos válidos da linguagem:

### Definições Auxiliares
* **Letra (`L`):** `[a-zA-Z]`
* **Dígito (`D`):** `[0-9]`

### Tokens / Átomos
* **Números Inteiros:** `D+` (Um ou mais dígitos).
* **Booleanos:** `True | False`.
* **Identificadores:** `(L | _)(L | D | _)*` (Iniciam obrigatoriamente com uma letra ou sublinhado, seguidos de zero ou mais letras, dígitos ou sublinhados).
* **Comentários:** `#.*` (Iniciam com o caractere `#` e ignoram tudo até o fim da linha).
* **Strings Literais:** `"[^"]*" | '[^']*'` (Utilizadas apenas para mensagens em comandos de interação como `print` e `input`).
* **Operadores Aritméticos:** `\+ | \- | \* | / | \*\* | %`
* **Operadores Relacionais e de Identidade:** `== | != | < | > | <= | >= | is`
* **Operadores Lógicos e de Associação:** `and | or | not | in`
* **Delimitadores e Pontuação:** `\( | \) | \[ | \] | \{ | \} | : | = | ,`.
* **Palavras Reservadas:** `return | from | while | as | elif | with | else | if | break | len | input | print | exec | raise | continue | range | def | for`.
* **Espaços em Branco:** `[ \t\n\r]+` (Devem ser ignorados na geração de tokens, servindo apenas para separar os lexemas e controlar a numeração das linhas).

---

## 2. Gramática Livre de Contexto (GLC)

A sintaxe do MiniPython possui restrições importantes: não utiliza o conceito de blocos de código baseados em indentação e estruturas de controle (como `for` e `while`) aceitam apenas um comando no seu escopo. 

Abaixo está a proposta da GLC, onde as palavras em maiúsculo (ou entre aspas) representam os *tokens* terminais:

### Regras de Produção

**Programa e Comandos**
* `Programa -> Comando Programa | ε`
* `Comando -> Atribuicao | Condicional | Repeticao | ComandoPrint | ComandoInput`

**Estruturas de Controle**
* `Condicional -> "if" Expressao ":" Comando ( "elif" Expressao ":" Comando )* ( "else" ":" Comando )?`
* `Repeticao -> "while" Expressao ":" Comando | "for" IDENTIFICADOR "in" "range" "(" Expressao ")" ":" Comando`

**Atribuição e Interação**
* `Atribuicao -> IDENTIFICADOR ( "[" Expressao "]" )? "=" Expressao | IDENTIFICADOR "(" (ListaExpressoes)? ")"`
* `ComandoPrint -> "print" ListaExpressoes`
* `ComandoInput -> IDENTIFICADOR "=" "input" "(" STRING_LITERAL ")"`

**Expressões**
* `ListaExpressoes -> Expressao ( "," Expressao )*`
* `Expressao -> ExpRelacional ( ("and" | "or") ExpRelacional )*`
* `ExpRelacional -> ExpSimples ( OperadorRelacional ExpSimples )?`
* `ExpSimples -> ( "+" | "-" )? Termo ( OperadorAritmetico Termo )*`
* `Termo -> ( "not" )? Fator`
* `Fator -> IDENTIFICADOR ( "[" Expressao "]" )? | NUMERO_INTEIRO | BOOLEANO | Lista | STRING_LITERAL | "(" Expressao ")" | "len" "(" Expressao ")"`

**Listas (Estrutura de Dados suportada)**
* `Lista -> "[" ElementosLista "]"`
* `ElementosLista -> Expressao ( "," Expressao )* | ε`

**Operadores**
* `OperadorRelacional -> "==" | "!=" | "<" | ">" | "<=" | ">=" | "is" | "in"`
* `OperadorAritmetico -> "+" | "*" | "**" | "%"`
* `OperadorLogico -> "and" | "or" | "not"`
