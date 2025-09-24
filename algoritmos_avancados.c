#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ============================================================================
// Estrutura da sala (nó da árvore binária)
// ============================================================================
typedef struct Sala {
    char nome[50];
    struct Sala *esquerda;
    struct Sala *direita;
} Sala;

// ============================================================================
// Função: criarSala
// Cria dinamicamente uma sala com nome informado
// ============================================================================
Sala* criarSala(const char *nome) {
    Sala *nova = (Sala*) malloc(sizeof(Sala));
    if (nova == NULL) {
        printf("Erro ao alocar memoria!\n");
        exit(1);
    }
    strcpy(nova->nome, nome);
    nova->esquerda = NULL;
    nova->direita = NULL;
    return nova;
}

// ============================================================================
// Função: explorarSalas
// Permite ao jogador explorar as salas da mansão interativamente
// ============================================================================
void explorarSalas(Sala *atual) {
    char escolha;

    while (atual != NULL) {
        printf("\nVocê está na: %s\n", atual->nome);

        // Caso seja nó-folha (sem caminhos)
        if (atual->esquerda == NULL && atual->direita == NULL) {
            printf("Não há mais caminhos. Fim da exploração!\n");
            break;
        }

        printf("Escolha um caminho:\n");
        if (atual->esquerda != NULL)
            printf(" (e) Ir para %s\n", atual->esquerda->nome);
        if (atual->direita != NULL)
            printf(" (d) Ir para %s\n", atual->direita->nome);
        printf(" (s) Sair do jogo\n");

        printf(">>> ");
        scanf(" %c", &escolha);

        if (escolha == 'e' && atual->esquerda != NULL) {
            atual = atual->esquerda;
        } else if (escolha == 'd' && atual->direita != NULL) {
            atual = atual->direita;
        } else if (escolha == 's') {
            printf("Você decidiu encerrar a exploração.\n");
            break;
        } else {
            printf("Opção inválida! Tente novamente.\n");
        }
    }
}

// ============================================================================
// Função: main
// Monta o mapa da mansão (árvore binária) e inicia a exploração
// ============================================================================
int main() {
    // Construção manual da árvore (mansão)
    Sala *hall = criarSala("Hall de Entrada");
    hall->esquerda = criarSala("Sala de Estar");
    hall->direita = criarSala("Cozinha");

    hall->esquerda->esquerda = criarSala("Biblioteca");
    hall->esquerda->direita = criarSala("Jardim de Inverno");

    hall->direita->esquerda = criarSala("Despensa");
    hall->direita->direita = criarSala("Sala de Jantar");

    // Início da exploração
    printf("=== Detective Quest: Exploração da Mansão ===\n");
    explorarSalas(hall);

    // Liberação de memória (simples, sem percorrer toda árvore aqui)
    free(hall->esquerda->esquerda);
    free(hall->esquerda->direita);
    free(hall->direita->esquerda);
    free(hall->direita->direita);
    free(hall->esquerda);
    free(hall->direita);
    free(hall);

    return 0;
}
