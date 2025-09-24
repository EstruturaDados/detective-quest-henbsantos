#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ============================================================================
// Estrutura da Sala (árvore da mansão)
// ============================================================================
typedef struct Sala {
    char nome[50];
    char pista[100];           // Pista associada à sala (pode estar vazia)
    struct Sala *esquerda;
    struct Sala *direita;
} Sala;

// ============================================================================
// Estrutura da árvore de pistas (BST)
// ============================================================================
typedef struct PistaNode {
    char pista[100];
    struct PistaNode *esquerda;
    struct PistaNode *direita;
} PistaNode;

// ============================================================================
// Função: criarSala
// Cria dinamicamente uma sala com nome e pista
// ============================================================================
Sala* criarSala(const char *nome, const char *pista) {
    Sala *nova = (Sala*) malloc(sizeof(Sala));
    if (nova == NULL) {
        printf("Erro ao alocar memoria para a sala!\n");
        exit(1);
    }
    strcpy(nova->nome, nome);
    if (pista != NULL)
        strcpy(nova->pista, pista);
    else
        strcpy(nova->pista, ""); // Sem pista
    nova->esquerda = NULL;
    nova->direita = NULL;
    return nova;
}

// ============================================================================
// Função: inserirPista
// Insere uma pista na árvore BST em ordem alfabética
// ============================================================================
PistaNode* inserirPista(PistaNode *raiz, const char *pista) {
    if (strlen(pista) == 0) return raiz; // Ignora salas sem pista

    if (raiz == NULL) {
        PistaNode *novo = (PistaNode*) malloc(sizeof(PistaNode));
        strcpy(nov
