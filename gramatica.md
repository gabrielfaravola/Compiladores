# Gramática MiniPython 

Esta gramática foi estruturada para ser utilizada em um **Analisador Sintático Descendente Recursivo Preditivo**. 

## 1. Estrutura Global
- **ProgramaMiniPython** -> ( Instrucao )*
- **Instrucao** -> VariavelVetor | IF | WhileFor | Print | Input | Funcao | ControleFluxo

## 2. Comandos de Controle
- **IF** -> "if" Expressao ":" Instrucao ( "elif" Expressao ":" Instrucao )* [ "else" ":" Instrucao ]
- **WhileFor** -> "while" Expressao ":" Instrucao | "for" IDENTIFICADOR "in" "range" "(" Expressao ")" ":" Instrucao
- **ControleFluxo** -> "break" | "continue" | "return" [ Expressao ]

## 3. Atribuição e I/O
- **VariavelVetor** -> IDENTIFICADOR [ "[" Expressao "]" ] "=" Expressao
- **Funcao** -> IDENTIFICADOR "(" [ ListaArgumentos ] ")"
- **Print** -> "print" ListaPrint
- **Input** -> IDENTIFICADOR "=" "input" "(" STRING_LITERAL ")"

## 4. Expressões e Lógica
- **ListaPrint** -> ConteudoPrint ( "," ConteudoPrint )*
- **ConteudoPrint** -> STRING_LITERAL | Expressao
- **ListaArgumentos** -> Expressao ( "," Expressao )*
- **Expressao** -> Relacional ( ( "and" | "or" ) Relacional )*
- **Relacional** -> ExpMatematica [ OperadorRelacional ExpMatematica ]
- **ExpMatematica** -> TermoMultiplicativo ( "+" TermoMultiplicativo )*
- **TermoMultiplicativo** -> [ "not" ] ElementoBase ( ( "*" | "**" | "%" ) ElementoBase )*

## 5. Unidades de Dados e Tipos
- **ElementoBase** -> IDENTIFICADOR [ "[" Expressao "]" ]
                  | NUMERO_INTEIRO
                  | BOOLEANO
                  | CriacaoLista
                  | CriacaoTupla
                  | "(" Expressao ")"
                  | "len" "(" Expressao ")"

- **CriacaoLista** -> "[" [ ItensColecao ] "]"
- **CriacaoTupla** -> "(" [ ItensColecao ] ")"
- **ItensColecao** -> ListaArgumentos

## 6. Operadores Terminais
- **OperadorRelacional** -> "==" | "!=" | "<" | ">" | "<=" | ">=" | "is" | "in"