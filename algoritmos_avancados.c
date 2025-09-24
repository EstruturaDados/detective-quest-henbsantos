/*
 Detective Quest - Sistema de coleta de pistas, associação a suspeitos e veredito.
 Autor: Enigma Studios (implementação exemplo)
 Compilar: gcc -o detective detective.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* ------------------------
   Definições e constantes
   ------------------------ */
#define MAX_NOME 64
#define MAX_PISTA 128
#define HASH_SIZE 101   // tamanho simples para tabela hash

/* ------------------------
   Structs principais
   ------------------------ */

// Nó da árvore da mansão (cada sala)
typedef struct Sala {
    char nome[MAX_NOME];
    char pista[MAX_PISTA]; // "" se não houver pista
    struct Sala *esquerda;
    struct Sala *direita;
} Sala;

// Nó da BST de pistas coletadas
typedef struct PistaNode {
    char pista[MAX_PISTA];
    struct PistaNode *esquerda;
    struct PistaNode *direita;
} PistaNode;

// Nó para tabela hash (encadeamento)
typedef struct HashNode {
    char pista[MAX_PISTA];          // chave
    char suspeito[MAX_NOME];        // valor
    struct HashNode *next;
} HashNode;

/* ------------------------
   Protótipos
   ------------------------ */
Sala* criarSala(const char *nome, const char *pista);
PistaNode* inserirPista(PistaNode *raiz, const char *pista); // insere na BST
void exibirPistasInOrder(PistaNode *raiz);
void liberarPistas(PistaNode *raiz);
void liberarMansao(Sala *raiz);

unsigned long hashString(const char *s);
void inserirNaHash(HashNode *tabela[], const char *pista, const char *suspeito);
const char* encontrarSuspeito(HashNode *tabela[], const char *pista);
void liberarHash(HashNode *tabela[]);

PistaNode* explorarSalas(Sala *atual, PistaNode *colecao, HashNode *tabela[]); // exploração

int verificarSuspeitoFinal(PistaNode *colecao, HashNode *tabela[], const char *acusado,
                           char pistas_encontradas[][MAX_PISTA], int *num_found);

/* ------------------------
   Implementação
   ------------------------ */

/*
 criarSala() – cria dinamicamente um cômodo.
   Recebe nome da sala e descrição da pista (ou NULL/"").
   Retorna ponteiro para Sala alocada.
*/
Sala* criarSala(const char *nome, const char *pista) {
    Sala *s = (Sala*) malloc(sizeof(Sala));
    if (!s) {
        fprintf(stderr, "Erro de alocacao!\n");
        exit(1);
    }
    strncpy(s->nome, nome, MAX_NOME-1);
    s->nome[MAX_NOME-1] = '\0';
    if (pista != NULL) {
        strncpy(s->pista, pista, MAX_PISTA-1);
        s->pista[MAX_PISTA-1] = '\0';
    } else {
        s->pista[0] = '\0';
    }
    s->esquerda = s->direita = NULL;
    return s;
}

/*
 inserirPista() / adicionarPista() – insere a pista coletada na árvore de pistas (BST).
  - Ignora pistas vazias ("").
  - Evita duplicatas (compara strings).
  - Retorna a raiz (atualizada).
*/
PistaNode* inserirPista(PistaNode *raiz, const char *pista) {
    if (pista == NULL || strlen(pista) == 0) return raiz; // nada a inserir

    if (raiz == NULL) {
        PistaNode *n = (PistaNode*) malloc(sizeof(PistaNode));
        if (!n) { fprintf(stderr,"Erro aloc PistaNode\n"); exit(1); }
        strncpy(n->pista, pista, MAX_PISTA-1);
        n->pista[MAX_PISTA-1] = '\0';
        n->esquerda = n->direita = NULL;
        return n;
    }

    int cmp = strcmp(pista, raiz->pista);
    if (cmp < 0) raiz->esquerda = inserirPista(raiz->esquerda, pista);
    else if (cmp > 0) raiz->direita = inserirPista(raiz->direita, pista);
    // se igual, não insere (evita duplicata)
    return raiz;
}

/*
 exibirPistas() – imprime a árvore de pistas em ordem alfabética.
*/
void exibirPistasInOrder(PistaNode *raiz) {
    if (!raiz) return;
    exibirPistasInOrder(raiz->esquerda);
    printf(" - %s\n", raiz->pista);
    exibirPistasInOrder(raiz->direita);
}

/*
 liberarPistas() – libera memória da BST de pistas.
*/
void liberarPistas(PistaNode *raiz) {
    if (!raiz) return;
    liberarPistas(raiz->esquerda);
    liberarPistas(raiz->direita);
    free(raiz);
}

/* ------------------------
   Hash table: funções
   ------------------------ */

/* djb2 - hash string */
unsigned long hashString(const char *s) {
    unsigned long hash = 5381;
    int c;
    while ((c = (unsigned char)*s++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    return hash;
}

/*
 inserirNaHash() – insere associação pista -> suspeito na tabela hash.
  - Se a mesma pista já existir, sobrescreve o suspeito (simples).
*/
void inserirNaHash(HashNode *tabela[], const char *pista, const char *suspeito) {
    if (!pista || strlen(pista) == 0) return;
    unsigned long h = hashString(pista) % HASH_SIZE;
    HashNode *cur = tabela[h];
    while (cur != NULL) {
        if (strcmp(cur->pista, pista) == 0) {
            // sobrescreve o suspeito
            strncpy(cur->suspeito, suspeito, MAX_NOME-1);
            cur->suspeito[MAX_NOME-1] = '\0';
            return;
        }
        cur = cur->next;
    }
    // não encontrou, insere no início da lista
    HashNode *n = (HashNode*) malloc(sizeof(HashNode));
    if (!n) { fprintf(stderr,"Erro aloc HashNode\n"); exit(1); }
    strncpy(n->pista, pista, MAX_PISTA-1);
    n->pista[MAX_PISTA-1] = '\0';
    strncpy(n->suspeito, suspeito, MAX_NOME-1);
    n->suspeito[MAX_NOME-1] = '\0';
    n->next = tabela[h];
    tabela[h] = n;
}

/*
 encontrarSuspeito() – consulta o suspeito correspondente a uma pista na tabela hash.
  - Retorna ponteiro para string interna (não liberar).
  - Retorna NULL se não encontrado.
*/
const char* encontrarSuspeito(HashNode *tabela[], const char *pista) {
    if (!pista || strlen(pista) == 0) return NULL;
    unsigned long h = hashString(pista) % HASH_SIZE;
    HashNode *cur = tabela[h];
    while (cur) {
        if (strcmp(cur->pista, pista) == 0) return cur->suspeito;
        cur = cur->next;
    }
    return NULL;
}

/*
 liberarHash() – libera a tabela hash.
*/
void liberarHash(HashNode *tabela[]) {
    for (int i = 0; i < HASH_SIZE; ++i) {
        HashNode *cur = tabela[i];
        while (cur) {
            HashNode *tmp = cur;
            cur = cur->next;
            free(tmp);
        }
        tabela[i] = NULL;
    }
}

/* ------------------------
   Exploração: coleta de pistas
   ------------------------ */

/*
 explorarSalas() – navega pela árvore e ativa o sistema de pistas.
  - Ao entrar em uma sala, exibe sua pista (se houver) e a adiciona na BST de pistas.
  - Entrada: sala atual (ponto de início), ponteiro para BST de colecao (pode ser NULL),
    tabela hash para mapear pistas->suspeitos (não usada aqui para inserir, mas pode ser usada se quiser exibir suspeitos imediatos).
  - Retorna ponteiro para BST atualizado.
*/
PistaNode* explorarSalas(Sala *atual, PistaNode *colecao, HashNode *tabela[]) {
    char escolha;
    while (atual != NULL) {
        printf("\nVocê está na sala: %s\n", atual->nome);
        if (strlen(atual->pista) > 0) {
            printf("  >> Pista encontrada: \"%s\"\n", atual->pista);
            colecao = inserirPista(colecao, atual->pista);
            // (Opcional) mostrar suspeito imediatamente:
            const char *s = encontrarSuspeito(tabela, atual->pista);
            if (s) printf("     (esta pista aponta para: %s)\n", s);
        } else {
            printf("  >> Nenhuma pista nesta sala.\n");
        }

        // Mostrar opções
        printf("\nEscolha um caminho:\n");
        if (atual->esquerda) printf(" (e) Ir para %s\n", atual->esquerda->nome);
        if (atual->direita) printf(" (d) Ir para %s\n", atual->direita->nome);
        printf(" (s) Sair da exploração\n");
        printf(">>> ");

        // lê escolha (ignora espaços)
        if (scanf(" %c", &escolha) != 1) { escolha = 's'; }

        if (escolha == 'e' && atual->esquerda) {
            atual = atual->esquerda;
        } else if (escolha == 'd' && atual->direita) {
            atual = atual->direita;
        } else if (escolha == 's') {
            printf("\nVocê escolheu encerrar a exploração.\n");
            break;
        } else {
            printf("Opção inválida ou caminho inexistente. Tente novamente.\n");
        }
    }
    return colecao;
}

/* ------------------------
   Verificação final
   ------------------------ */

/*
 verificarSuspeitoFinal() – conduz à fase de julgamento final.
  - Percorre a BST de pistas coletadas e conta quantas pistas apontam para o suspeito acusado.
  - Retorna o número de pistas que apontam para o acusado.
  - Também preenche pistas_encontradas[] com as pistas que apontam para o acusado (até tamanho do array).
*/
int verificarSuspeitoFinal(PistaNode *colecao, HashNode *tabela[], const char *acusado,
                           char pistas_encontradas[][MAX_PISTA], int *num_found) {
    if (!colecao) return 0;
    int count = 0;
    // Percorre in-order e coleta
    // Usaremos uma pilha recursiva simples:
    // esquerda -> nodo -> direita
    count += verificarSuspeitoFinal(colecao->esquerda, tabela, acusado, pistas_encontradas, num_found);

    const char *s = encontrarSuspeito(tabela, colecao->pista);
    if (s && strcasecmp(s, acusado) == 0) {
        if (*num_found >= 0) {
            strncpy(pistas_encontradas[*num_found], colecao->pista, MAX_PISTA-1);
            pistas_encontradas[*num_found][MAX_PISTA-1] = '\0';
        }
        (*num_found)++;
        count++;
    }

    count += verificarSuspeitoFinal(colecao->direita, tabela, acusado, pistas_encontradas, num_found);
    return count;
}

/* ------------------------
   Auxiliares
   ------------------------ */

/* liberarMansao() - libera toda a estrutura de salas (pré-ordem) */
void liberarMansao(Sala *raiz) {
    if (!raiz) return;
    liberarMansao(raiz->esquerda);
    liberarMansao(raiz->direita);
    free(raiz);
}

/* lowercase copy helper */
void lowercase_copy(char *dst, const char *src, int maxl) {
    int i = 0;
    for (; i < maxl-1 && src[i] != '\0'; ++i) dst[i] = tolower((unsigned char)src[i]);
    dst[i] = '\0';
}

/* ------------------------
   main: monta mansão, tabela de suspeitos e roda jogo
   ------------------------ */
int main(void) {
    // 1) Montagem fixa da mansão (árvore binária)
    Sala *hall = criarSala("Hall de Entrada", "Luvas jogadas no chao");
    hall->esquerda = criarSala("Sala de Estar", "Copo quebrado");
    hall->direita = criarSala("Cozinha", "Garrafa com resquicios de veneno");

    hall->esquerda->esquerda = criarSala("Biblioteca", "Livro rasgado");
    hall->esquerda->direita = criarSala("Jardim de Inverno", "Pegadas no vaso");

    hall->direita->esquerda = criarSala("Despensa", "Lata de veneno aberta");
    hall->direita->direita = criarSala("Sala de Jantar", "Guardanapo com monograma");

    // 2) Cria tabela hash e popula com associações pista -> suspeito
    HashNode *tabela[HASH_SIZE];
    for (int i = 0; i < HASH_SIZE; ++i) tabela[i] = NULL;

    // Mapeamentos (exemplos)
    inserirNaHash(tabela, "Luvas jogadas no chao", "Sr. Silva");
    inserirNaHash(tabela, "Copo quebrado", "Sra. Pereira");
    inserirNaHash(tabela, "Garrafa com resquicios de veneno", "Dr. Costa");
    inserirNaHash(tabela, "Livro rasgado", "Sr. Silva");
    inserirNaHash(tabela, "Pegadas no vaso", "Sra. Pereira");
    inserirNaHash(tabela, "Lata de veneno aberta", "Dr. Costa");
    inserirNaHash(tabela, "Guardanapo com monograma", "Sr. Silva");

    // 3) Exploração e coleta de pistas
    PistaNode *colecao = NULL;
    printf("=== Detective Quest: fase de coleta de pistas ===\n");
    colecao = explorarSalas(hall, colecao, tabela);

    // 4) Exibição das pistas coletadas (ordem alfabética)
    printf("\n=== Pistas coletadas (ordem alfabetica) ===\n");
    if (!colecao) {
        printf("Nenhuma pista coletada.\n");
    } else {
        exibirPistasInOrder(colecao);
    }

    // 5) Fase de acusação
    char acusado[MAX_NOME];
    // Limpa buffer pendente de entrada (quando uso scanf antes)
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {} // limpa resto da linha
    printf("\nQuem você acusa como culpado? (digite nome completo)\n>>> ");
    if (!fgets(acusado, MAX_NOME, stdin)) acusado[0] = '\0';
    // remove newline
    acusado[strcspn(acusado, "\n")] = '\0';

    if (strlen(acusado) == 0) {
        printf("Nenhum acusado informado. Encerrando sem veredito.\n");
    } else {
        // comparar sem diferenciar maiúsculas/minúsculas
        char accused_lower[MAX_NOME];
        lowercase_copy(accused_lower, acusado, MAX_NOME);

        // Verifica quantas pistas apontam para 'acusado'
        char pistas_encontradas[50][MAX_PISTA]; // até 50 pistas coletadas
        int num_found = 0;
        int total = verificarSuspeitoFinal(colecao, tabela, accused_lower, pistas_encontradas, &num_found);

        // A função verifica usando strcasecmp no valor do suspeito. Precisamos
        // garantir que encontrarSuspeito retorne nomes comparáveis. Para a
        // simplicidade, vamos comparar strcasecmp com acusado original.
        // Ajuste: na nossa verificarSuspeitoFinal usamos strcasecmp(s, acus...), 
        // porém passamos acusado em lowercase. Para garantir correto, 
        // chamaremos novamente mas de forma direta: vamos contar manualmente.

        // Faz uma nova contagem robusta (corrige eventual desacordo)
        num_found = 0;
        // Função auxiliar: percorre in-order e conta
        // Vamos reutilizar a função, mas passando acusado com casefold:
        char pistas_tmp[50][MAX_PISTA];
        int tmpn = 0;
        int count = 0;

        // Percorre in-order manualmente com função recursiva inline-mente:
        // Usaremos uma função lambda-simulada via função auxiliar local não disponível em C.
        // Portanto, reutilizamos verificarSuspeitoFinal porém passando acusado original.
        // Para isso, ajustamos: encontrarSuspeito devolve nomes em forma original,
        // e usamos strcasecmp para comparar com acusado informado.
        tmpn = 0;
        count = 0;
        // Reutiliza função: ela incrementa num_found por referência
        int nf = 0;
        count = verificarSuspeitoFinal(colecao, tabela, acusado, pistas_tmp, &nf);

        printf("\n=== Verificacao da acusacao para: %s ===\n", acusado);
        if (count >= 2) {
            printf("Acusacao sustentada! Foram encontradas %d pistas que apontam para %s:\n", count, acusado);
            for (int i = 0; i < nf; ++i) {
                printf("  - %s\n", pistas_tmp[i]);
            }
            printf("\nResultado: %s é o culpado! (Acusação aceita)\n", acusado);
        } else if (count == 1) {
            printf("Apenas 1 pista aponta para %s. Nao eh suficiente para condenacao.\n", acusado);
            printf("Pista encontrada:\n  - %s\n", pistas_tmp[0]);
            printf("\nResultado: Acusacao rejeitada por falta de evidencias.\n");
        } else {
            printf("Nenhuma pista coletada aponta para %s.\n", acusado);
            printf("Resultado: Acusacao rejeitada.\n");
        }
    }

    // 6) Limpeza: liberar memória
    liberarPistas(colecao);
    liberarMansao(hall);
    liberarHash(tabela);

    printf("\nEncerrando Detective Quest. Obrigado por jogar!\n");
    return 0;
}
