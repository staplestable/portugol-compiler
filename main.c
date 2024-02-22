#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
//Atomo = codigo interno dos agrupamentos logicos gerados por sequencias de caracteres (lexemas) diferentes.
//Constantes numericas dos atomos

//Constantes numericas dos atomos como tipo enumerado.
typedef enum{
    ERRO,
    IDENTIFICADOR,
    NUMERO,
    OP_SOMA,
    OP_SUB,
    OP_MULT,
    OP_DIV,
    PONTO_VIRGULA,
    PONTO,
    DOIS_PONTOS,
    VIRGULA,
    ALGORITMO,
    VARIAVEL,
    INTEIRO,
    LOGICO,
    INICIO,
    FIM,
    ATRIBUICAO,
    MENOR,
    MENOR_IGUAL,
    IGUAL,
    DIFERENTE,
    MAIOR,
    MAIOR_IGUAL,
    VERDADEIRO,
    FALSO,
    ABRE_PARENTESES,
    FECHA_PARENTESES,
    SE,
    ENQUANTO,
    LEIA,
    ESCREVA,
    ENTAO,
    SENAO,
    FACA,
    COMENTARIO,
    E,
    OU,
    EOS
}TAtomo;

//Estrutura de registros para armazenar informacoes do atomo.
typedef struct {
	TAtomo atomo;
	int linha;
	char atributo_ID[16];
}TInfoAtomo;

//Lista para guardar as variáveis declaradas.
TInfoAtomo* tabela_de_simbolos = NULL;
//Tamanho da tabela de simbolos.
int tamanho = 0;

//Função de inserção para lista.
TInfoAtomo* insertList(TInfoAtomo variavel){
  tamanho++;
  tabela_de_simbolos = (TInfoAtomo*) realloc(tabela_de_simbolos, tamanho * sizeof(TInfoAtomo));

  if (tabela_de_simbolos == NULL) {
      fprintf(stderr, "Memory allocation failed\n");
      exit(-1);
  }
  tabela_de_simbolos[tamanho - 1] = variavel;

  return tabela_de_simbolos;
}

//Função para checar se a lista esta vazia para lista.
int isEmpty(){ return tamanho == 0; }

//Função para checar tamanho da lista.
int getSize() { return tamanho; }

//Variavel global para entrada do codigo fonte.
char* buffer;
//Variavel global para contagem de linhas.
int linhaCont = 1;
//Variavel global para contagem de rotulos.
int rotulo = 0;

//Funcao de leitura.
void leitura(char *nomeArquivo) {
  FILE *file;
  long size;
  size_t result;

  file = fopen(nomeArquivo, "rb");
  if (file == NULL) {
    fputs("File error", stderr);
    exit(1);
  }
  fseek(file, 0, SEEK_END);
  size = ftell(file);
  rewind(file);
  buffer = (char *)calloc((size+1), sizeof(char));
  if (buffer == NULL) {
    fputs("Memory error", stderr);
    exit(2);
  }
  result = fread(buffer, 1, size, file);
  if (result != size) {
    fputs("Reading error", stderr);
    exit(3);
  }

  fclose(file);
}

//Funcao para obter os atomos da linguagem.
TInfoAtomo obter_atomo();
TInfoAtomo reconhece_numero();
TInfoAtomo reconhece_id();
TInfoAtomo reconhece_comentario();

//Variaveis globais e funcoes DO ANALISADOR SINTATICO
char strAtomo[][30] = { "erro lexico", "identificador","numero","soma","subtracao","multiplicacao",
"divisao","ponto e virgula", "ponto","dois pontos","virgula","algoritmo", "variavel","inteiro",
"logico","inicio","fim","atribuicao","menor","menor igual","igual","diferente","maior","maior igual",
"verdadeiro", "falso", "abre parenteses", "fecha parenteses" , "se", "enquanto", "leia", "escreva", "entao", "senao","faca","comentario", "e", "ou", "fim da analise sintatica"};

TInfoAtomo InfoAtomo;
TAtomo lookahead;// lookahead = obter_atomo()

void programa();
void bloco();
void declaracao_de_variaveis();
void lista_variavel();
void tipo();
void comando_composto();
void comando();
void comando_atribuicao();
void comando_se();
void comando_enquanto();
void comando_entrada();
void comando_saida();
void expressao();
void relacional();
void expressao_simples();
void termo();
void fator();
int busca_tabela_simbolos();

void consome( TAtomo atomo );

int main() {
  tabela_de_simbolos = (TInfoAtomo*) malloc(tamanho * sizeof(TInfoAtomo));

  leitura("entrada.txt");
	printf("Analisando:\n\n%s\n\n", buffer);

	printf("\n== ANALISE SEMANTICA == \n\n");
	InfoAtomo = obter_atomo();
	lookahead = InfoAtomo.atomo;
	programa(); // chama o simbolo inicial da gramatica
	consome(EOS);

	return 0;
}

//###############################
// FUNCOES ANALISE SEMANTICA
//###############################

int proximo_rotulo() { return ++rotulo; }

int busca_tabela_simbolos(char* atributo_ID){
  if(isEmpty()) return -1;
  for(int i = 0; i < getSize(); i++){
    if(strcmp(tabela_de_simbolos[i].atributo_ID, atributo_ID) == 0) return i;
  }
  return -1;
}

void erroSemantico(){
  printf("\n#  %d: Erro semantico encontrado \n", InfoAtomo.linha);
  exit(1);
}

//###############################
// ANALISADOR LEXICO
//###############################

TInfoAtomo obter_atomo() {
    TInfoAtomo infoAtomo;

    infoAtomo.linha = 0;

	//Descarta os delimitadores
	while( *buffer== ' ' || *buffer== '\n' || *buffer== '\t' || *buffer== '\r'){
            if(*buffer == '\n') linhaCont++;
            buffer++;
    }
	if (isdigit(*buffer)) {
		infoAtomo = reconhece_numero();
	}
	else if (isalpha(*buffer)) {
		infoAtomo = reconhece_id();
	}
    else if(*buffer == '+'){
    infoAtomo.atomo = OP_SOMA;
    buffer++;
    }
    else if(*buffer == '-'){
    infoAtomo.atomo = OP_SUB;
    buffer++;
    }
    else if(*buffer == '*'){
    infoAtomo.atomo = OP_MULT;
    buffer++;
    }
    else if(*buffer == ';'){
    infoAtomo.atomo = PONTO_VIRGULA;
    buffer++;
    }
    else if(*buffer == '.'){
    infoAtomo.atomo = PONTO;
    buffer++;
    }
    else if(*buffer == ':' && *(buffer+1) != '='){
    infoAtomo.atomo = DOIS_PONTOS;
    buffer++;
    }
    else if(*buffer == ':' && *(buffer+1) == '='){
    infoAtomo.atomo = ATRIBUICAO;
    buffer += 2;
    }
    else if(*buffer == '<' && *(buffer+1) != '='){
    infoAtomo.atomo = MENOR;
    buffer++;
    }
    else if(*buffer == '<' && *(buffer+1) == '='){
    infoAtomo.atomo = MENOR_IGUAL;
    buffer += 2;
    }
    else if(*buffer == '='){
    infoAtomo.atomo = IGUAL;
    buffer++;
    }
    else if(*buffer == '#'){
    infoAtomo.atomo = DIFERENTE;
    buffer++;
    }
    else if(*buffer == '>'  && *(buffer+1) != '='){
    infoAtomo.atomo = MAIOR;
    buffer++;
    }
    else if(*buffer == '>' && *(buffer+1) == '='){
    infoAtomo.atomo = MAIOR_IGUAL;
    buffer += 2;
    }
    else if(*buffer == ','){
    infoAtomo.atomo = VIRGULA;
    buffer++;
    }
    else if(*buffer == '('){
    infoAtomo.atomo = ABRE_PARENTESES;
    buffer++;
    }
    else if(*buffer == ')'){
    infoAtomo.atomo = FECHA_PARENTESES;
    buffer++;
    }
    else if(*buffer == '/'){
    infoAtomo = reconhece_comentario();
    }
    else if (*buffer == '\x0') {
        infoAtomo.atomo = EOS;
    }
    else
        infoAtomo.atomo = ERRO;

    if(infoAtomo.linha == 0) infoAtomo.linha = linhaCont;
    return infoAtomo;
}

//Funcao para reconhecer os atomos numeros.
TInfoAtomo reconhece_numero() {
    char* pIniNum = buffer;
    TInfoAtomo infoAtomo;
    infoAtomo.atomo = ERRO;
    infoAtomo.linha = 0;

    if (isdigit(*buffer)) {
      buffer++;
      goto q1;
    }
    return infoAtomo;
q1:
    if (isdigit(*buffer)) {
        buffer++;
        goto q1;
    }
    if (isalpha(*buffer)){
      if(*buffer == 'e' || *buffer == 'E'){
        buffer++;
        goto q2;
      }
    }
    goto q3;
q2:
    if(*buffer == '+' || *buffer == '-'){
      buffer++;
      goto q3;
    }
    if(isdigit(*buffer)){
      buffer++;
      goto q3;
    }
    return infoAtomo; // retorna o InfoAtomo com erro
q3:
    if(isdigit(*buffer)){
      buffer++;
      goto q3;
    }

    strncpy(infoAtomo.atributo_ID, pIniNum, buffer - pIniNum);
    infoAtomo.atributo_ID[buffer - pIniNum] = '\x0';
    infoAtomo.atomo = NUMERO;
    return infoAtomo;
}

//Funcao para reconhecer os atomos identificadores.
TInfoAtomo reconhece_id() {
    char* pIniID = buffer;
    TInfoAtomo infoAtomo;
    infoAtomo.atomo = ERRO;
    infoAtomo.linha = 0;

    if (isalpha(*buffer)) {
        buffer++;
        goto q1;
    }
    return infoAtomo;
q1:
    if (isalpha(*buffer) || isdigit(*buffer) || *buffer == '_') {
        buffer++;
        goto q1;
    }

    strncpy(infoAtomo.atributo_ID, pIniID, buffer - pIniID);
    infoAtomo.atributo_ID[buffer - pIniID] = '\x0';

    if(strcmp(infoAtomo.atributo_ID,"algoritmo")==0)
      infoAtomo.atomo  = ALGORITMO;
    else if (strcmp(infoAtomo.atributo_ID,"variavel")==0)
      infoAtomo.atomo  = VARIAVEL;
    else if (strcmp(infoAtomo.atributo_ID,"inteiro")==0)
      infoAtomo.atomo  = INTEIRO;
    else if (strcmp(infoAtomo.atributo_ID,"logico")==0)
      infoAtomo.atomo  = LOGICO;
    else if (strcmp(infoAtomo.atributo_ID,"inicio")==0)
      infoAtomo.atomo  = INICIO;
    else if (strcmp(infoAtomo.atributo_ID,"fim")==0)
      infoAtomo.atomo  = FIM;
    else if (strcmp(infoAtomo.atributo_ID,"se")==0)
      infoAtomo.atomo  = SE;
    else if (strcmp(infoAtomo.atributo_ID,"enquanto")==0)
      infoAtomo.atomo  = ENQUANTO;
    else if (strcmp(infoAtomo.atributo_ID,"leia")==0)
      infoAtomo.atomo  = LEIA;
    else if (strcmp(infoAtomo.atributo_ID,"escreva")==0)
      infoAtomo.atomo  = ESCREVA;
    else if (strcmp(infoAtomo.atributo_ID,"entao")==0)
      infoAtomo.atomo  = ENTAO;
    else if (strcmp(infoAtomo.atributo_ID,"senao")==0)
      infoAtomo.atomo  = SENAO;
    else if (strcmp(infoAtomo.atributo_ID,"faca")==0)
      infoAtomo.atomo  = FACA;
    else if (strcmp(infoAtomo.atributo_ID,"div")==0)
      infoAtomo.atomo  = OP_DIV;
    else if (strcmp(infoAtomo.atributo_ID,"verdadeiro")==0)
      infoAtomo.atomo  = VERDADEIRO;
    else if (strcmp(infoAtomo.atributo_ID,"falso")==0)
      infoAtomo.atomo  = FALSO;
    else if (strcmp(infoAtomo.atributo_ID,"e")==0)
      infoAtomo.atomo  = E;
    else if (strcmp(infoAtomo.atributo_ID,"ou")==0)
      infoAtomo.atomo  = OU;
    else
      infoAtomo.atomo  = IDENTIFICADOR;
    return infoAtomo;
}

TInfoAtomo reconhece_comentario(){
    TInfoAtomo infoAtomo;
    infoAtomo.atomo = ERRO;
    infoAtomo.linha =  linhaCont;

    if(*buffer == '/' && *(buffer + 1) == '/'){
    buffer += 2;
    while (*buffer != '\n' && *buffer != '\x0')
      buffer++;
    infoAtomo.atomo = COMENTARIO;
    }
    else if (*buffer == '/' && *(buffer + 1) == '*'){
    buffer += 2;
    while (*buffer != '\x0') {
      if (*buffer == '\n') linhaCont++;
      if (*buffer == '*' && *(buffer + 1) == '/') {
        buffer += 2;
        infoAtomo.atomo = COMENTARIO;
        break;
      }
      buffer++;
    }
    } else {
    return infoAtomo;
    }
    return infoAtomo;
}


//###############################
// ANALISADOR SINTATICO
//###############################

void consome( TAtomo atomo ){
    if( lookahead == atomo ){
    InfoAtomo = obter_atomo();
    lookahead = InfoAtomo.atomo;
    while(lookahead == COMENTARIO) consome(COMENTARIO);

    }
    else{
      printf("\n#  %d: Erro sintatico: esperado [%s] encontrado [%s]\n", InfoAtomo.linha,strAtomo[atomo],strAtomo[lookahead]);
      exit(1);
    }
}

//<programa>::= algoritmo identificador “;” <bloco> “.
void programa(){
    printf("\tINPP \n");
    while(lookahead == COMENTARIO) consome(COMENTARIO);
    consome(ALGORITMO);
    consome(IDENTIFICADOR);
    consome(PONTO_VIRGULA);
    bloco();
    consome(PONTO);
    printf("\tPARA \n");
}

//<bloco>::= [ <declaracao_de_variaveis> ] <comando_composto>
void bloco(){
    if (lookahead == VARIAVEL)
    declaracao_de_variaveis();
    printf("\tAMEM %d \n", getSize());
    comando_composto();
}

//<declaracao_de_variaveis> ::= variavel {<lista_variavel> “:” <tipo> “;”}
void declaracao_de_variaveis(){
    consome(VARIAVEL);
    while(lookahead == IDENTIFICADOR){
    lista_variavel(0);
    consome(DOIS_PONTOS);
    tipo();
    consome(PONTO_VIRGULA);
    }
}

//<lista_variavel> ::= identificador { “,” identificador }
void lista_variavel(int chave){
    //chave 0 = declaracao
    //chave 1 = leitura
    if(chave == 0){
      if(busca_tabela_simbolos(InfoAtomo.atributo_ID) == -1) insertList(InfoAtomo);
      else erroSemantico();
    } else {
      printf("\tLEIT \n");
      int endereco = busca_tabela_simbolos(InfoAtomo.atributo_ID);
      //printf("[InfoAtomo sendo procurado: %s]\n", InfoAtomo.atributo_ID);
      // for(int i = 0; i < getSize(); i++){
      //   printf("[%s]", tabela_de_simbolos[i].atributo_ID);
      // }
      if(endereco == -1) erroSemantico();
      printf("\tARMZ %d \n",endereco);
    }
    consome(IDENTIFICADOR);
    while(lookahead == VIRGULA){
      consome(VIRGULA);
      if(chave == 0){
        if(busca_tabela_simbolos(InfoAtomo.atributo_ID) == -1) insertList(InfoAtomo);
        else erroSemantico();
      } else {
        printf("\tLEIT \n");
        int endereco = busca_tabela_simbolos(InfoAtomo.atributo_ID);
        if(endereco == -1) erroSemantico();
        printf("\tARMZ %d \n",endereco);
      }
      consome(IDENTIFICADOR);
    }
}

//<tipo> ::= inteiro
void tipo(){
    consome(INTEIRO);
}

//<comando_composto> ::= inicio <comando> { “;” <comando>} fim
void comando_composto(){
    consome(INICIO);
    comando();
    while(lookahead == PONTO_VIRGULA){
    consome(PONTO_VIRGULA);
    comando();
    }
    consome(FIM);
}

/*
<comando> ::= <comando_atribuicao> | <comando_se> | <comando_enquanto> | <comando_entrada> | <comando_saida> |
<comando_composto>
*/
void comando(){
    switch(lookahead){
    case IDENTIFICADOR:
      comando_atribuicao();
      break;
    case SE:
      comando_se();
      break;
    case ENQUANTO:
      comando_enquanto();
      break;
    case LEIA:
      comando_entrada();
      break;
    case ESCREVA:
      comando_saida();
      break;
    default:
      comando_composto();
    }
}

//<comando_atribuicao> ::= identificador “:=” <expressao>
void comando_atribuicao(){
    int end = busca_tabela_simbolos(InfoAtomo.atributo_ID);
    if(end == -1) erroSemantico();
    consome(IDENTIFICADOR);
    consome(ATRIBUICAO);
    expressao();
    printf("\tARMZ %d\n", end);
}

//<comando_se> ::= se “(” <expressao> “)” entao <comando> [senao <comando>]
void comando_se(){
    int L1 = proximo_rotulo();
    int L2 = proximo_rotulo();
    consome(SE);
    consome(ABRE_PARENTESES);
    expressao();
    consome(FECHA_PARENTESES);
    consome(ENTAO);
    printf("\tDSVF L%d\n",L1);
    comando();
    printf("\tDSVS L%d\n",L2);
    printf("L%d:\tNADA\n",L1);
    if(lookahead == SENAO){
    consome(SENAO);
    comando();
    }
    printf("L%d:\tNADA\n",L2);
}
//<comando_enquanto> ::= enquanto “(” <expressao> “)” faça <comando>
void comando_enquanto(){
    int L1 = proximo_rotulo();
    int L2 = proximo_rotulo();
    consome(ENQUANTO);
    printf("L%d:\tNADA\n",L1);
    consome(ABRE_PARENTESES);
    expressao();
    consome(FECHA_PARENTESES);
    printf("\tDSVF L%d\n",L2);
    consome(FACA);
    comando();
    printf("\tDSVS L%d\n",L1);
    printf("L%d:\tNADA\n",L2);
}

// <comando_entrada> ::= leia “(“ <lista_variavel> “)”
void comando_entrada(){
    consome(LEIA);
    consome(ABRE_PARENTESES);
    lista_variavel(1);
    consome(FECHA_PARENTESES);
}

//<comando_saida> ::= escreva “(“ <expressao> { “,” <expressao> } “)”
void comando_saida(){
    consome(ESCREVA);
    consome(ABRE_PARENTESES);
    expressao();
    printf("\tIMPR \n");
    while(lookahead == VIRGULA){
      consome(VIRGULA);
      expressao();
      printf("\tIMPR \n");
    }
    consome(FECHA_PARENTESES);
}

//<expressao> ::= <expressao_simples> [<relacional> <expressao_simples> ]
void expressao(){
    expressao_simples();
    switch(lookahead){
        case MENOR:
          relacional();
          expressao_simples();
          printf("\tCMME \n");
          break;
        case MENOR_IGUAL:
          relacional();
          expressao_simples();
          printf("\tCMEG \n");
          break;
        case IGUAL:
          relacional();
          expressao_simples();
          printf("\tCMIG \n");
          break;
        case DIFERENTE:
          relacional();
          expressao_simples();
          printf("\tCMDG \n");
          break;
        case MAIOR:
          relacional();
          expressao_simples();
          printf("\tCMMA \n");
          break;
        case MAIOR_IGUAL:
          relacional();
          expressao_simples();
          printf("\tCMAG \n");
          break;
        default:
          break;
    }
}

//<expressao_simples> ::= [“+” | “−”] <termo> { (“+” | “−” | ou ) <termo> }
void expressao_simples(){
    if(lookahead == OP_SOMA)
      consome(OP_SOMA);
    else if(lookahead == OP_SUB)
      consome(OP_SUB);
    termo();
    TAtomo prev;
    while(lookahead == OP_SOMA || lookahead == OP_SUB ||lookahead == OU){
      prev = lookahead;
      if(lookahead == OP_SOMA)
        consome(OP_SOMA);
      else if(lookahead == OP_SUB)
        consome(OP_SUB);
      else
        consome(OU); //vamos ter que criar
      termo();
      switch (prev){
        case OP_SOMA:
          printf("\tSOMA \n");
          break;
        case OP_SUB:
          printf("\tSUBT \n");
          break;
        default:
          printf("\tDISJ \n");
          break;
      }
    }
}

//“<” | “<=” | “=” | “#” | “>” | “>=”
void relacional(){
    switch(lookahead){
    case MENOR:
      consome(MENOR);
      break;
    case MENOR_IGUAL:
      consome(MENOR_IGUAL);
      break;
    case IGUAL:
      consome(IGUAL);
      break;
    case DIFERENTE:
      consome(DIFERENTE);
      break;
    case MAIOR:
      consome(MAIOR);
      break;
    default:
      consome(MAIOR_IGUAL);
    }
}

//<termo> ::= <fator> { ( “*” | div | e )<fator> }
void termo(){
    fator();
    TAtomo prev;
    while(lookahead == OP_MULT || lookahead == OP_DIV || lookahead == E){
      prev = lookahead;
      if(lookahead == OP_MULT)
        consome(OP_MULT);
      else if(lookahead == OP_DIV)
        consome(OP_DIV);
      else
        consome(E);
      fator();
      switch (prev){
        case OP_MULT:
          printf("\tMULT \n");
          break;
        case OP_DIV:
          printf("\tDIVI \n");
          break;
        default:
          printf("\tCONJ \n");
          break;
      }
    }
}

/*
<fator> ::= identificador |
numero |
verdadeiro |
falso |
“(” <expressao> “)
*/
void fator(){
    int endereco = -1;
    switch(lookahead){
    case IDENTIFICADOR:
      endereco = busca_tabela_simbolos(InfoAtomo.atributo_ID);
      if(endereco == -1) erroSemantico();
      printf("\tCRVL %d\n",endereco);
      consome(IDENTIFICADOR);
      break;
    case NUMERO:
      printf("\tCRCT %s\n", InfoAtomo.atributo_ID);
      consome(NUMERO);
      break;
    default:
      consome(ABRE_PARENTESES);
      expressao();
      consome(FECHA_PARENTESES);
    }
}

