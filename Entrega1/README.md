# Desenvolvedores

Gabriel Pereira Faravola - 10427189

Matheus Veiga Bacetic Joaquim - 10425638

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
* `ProgramaMiniPython -> Instrucao ProgramaMiniPython | ε`
* `Instrucao -> AtribuicaoVar | ComandoIf | ComandoRepeticao | ComandoPrint | ComandoInput | ChamadaFuncao | ControleFluxo`

**Estruturas de Controle**
* `ComandoIf -> "if" Expressao ":" Instrucao ( "elif" Expressao ":" Instrucao )* ( "else" ":" Instrucao )?`
* `ComandoRepeticao -> "while" Expressao ":" Instrucao | "for" IDENTIFICADOR "in" "range" "(" Expressao ")" ":" Instrucao`
* `ControleFluxo -> "break" | "continue" | "return" ( Expressao )?`

**Atribuição e Interação**
* `AtribuicaoVar -> IDENTIFICADOR ( "[" Expressao "]" )? "=" Expressao`
* `ChamadaFuncao -> IDENTIFICADOR "(" (ListaArgumentos)? ")"`
* `ComandoPrint -> "print" ListaPrint`
* `ComandoInput -> IDENTIFICADOR "=" "input" "(" STRING_LITERAL ")"`

**Expressões e Lógica**
* `ListaPrint -> ItemPrint ( "," ItemPrint )*`
* `ItemPrint -> STRING_LITERAL | Expressao`
* `ListaArgumentos -> Expressao ( "," Expressao )*`
* `Expressao -> TesteRelacional ( ("and" | "or") TesteRelacional )*`
* `TesteRelacional -> ExpMatematica ( OperadorRelacional ExpMatematica )?`
* `ExpMatematica -> TermoMultiplicativo ( ("+" | "-") TermoMultiplicativo )*`
* `TermoMultiplicativo -> ( "not" )? ElementoBase ( ("*" | "**" | "%") ElementoBase )*`
* `ElementoBase -> IDENTIFICADOR ( "[" Expressao "]" )? | NUMERO_INTEIRO | BOOLEANO | CriacaoLista | "(" Expressao ")" | "len" "(" Expressao ")"`

**Listas e Tuplas**
* `CriacaoLista -> "[" ElementosColecao "]"`
* `ElementosColecao -> ListaArgumentos | ε`

**Operadores**
* `OperadorRelacional -> "==" | "!=" | "<" | ">" | "<=" | ">=" | "is" | "in"`
* `OperadorAritmetico -> "+" | "-" | "*" | "**" | "%"`
* `OperadorLogico -> "and" | "or" | "not"`
