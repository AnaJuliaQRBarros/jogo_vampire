#define _CRT_SECURE_NO_WARNINGS

#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/keyboard.h>

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>


typedef struct {

    int x, y;

} Posicao;

typedef struct {

    Posicao pos;

    int velocidadeX;

} Inimigo;

typedef struct {

    int x, y;

} Item;

typedef struct {

    char nome[50];
    int pontos;

} Ranking;

// Ordenação do ranking

void ordenarRanking(Ranking ranking[], int tamanho) {

    for (int i = 0; i < tamanho - 1; i++) {
        for (int j = 0; j < tamanho - 1 - i; j++) {
            if (ranking[j].pontos <
                ranking[j + 1].pontos) {

                Ranking temp =
                    ranking[j];

                ranking[j] =
                    ranking[j + 1];

                ranking[j + 1] =
                    temp;
            }
        }
    }
}

int main() {

    // Nome do jogador

    char nomeJogador[50];

    printf("Digite o nome do jogador: ");
    scanf("%49s", nomeJogador);

    // Inicialização

    al_init();

    al_init_font_addon();
    al_init_ttf_addon();
    al_init_image_addon();
    al_init_primitives_addon();
    al_install_keyboard();
    srand(time(NULL));

    ALLEGRO_DISPLAY* display =
        al_create_display(1280, 720);

    al_set_window_title(display, "Vampire Loves");

    // Adicionar fontes

    ALLEGRO_FONT* font =
        al_load_font("./font.ttf", 18, 0);

    ALLEGRO_FONT* fontGrande =
        al_load_font("./font.ttf", 40, 0);

    // Timer

    ALLEGRO_TIMER* timer =
        al_create_timer(1.0 / 30.0);

    // Imagens

    ALLEGRO_BITMAP* sprite =
        al_load_bitmap("./personagem.png");
    ALLEGRO_BITMAP* bg =
        al_load_bitmap("./fundo.png");
    ALLEGRO_BITMAP* inimigoSprite =
        al_load_bitmap("./inimigo.png");
    ALLEGRO_BITMAP* itemSprite =
        al_load_bitmap("./item.png");

	// Verificação de erros

    if (!sprite)
        printf("Erro personagem.png\n");
    if (!bg)
        printf("Erro fundo.png\n");
    if (!inimigoSprite)
        printf("Erro inimigo.png\n");
    if (!itemSprite)
        printf("Erro item.png\n");

    // Eventos

    ALLEGRO_EVENT_QUEUE* event_queue =
        al_create_event_queue();

    al_register_event_source(
        event_queue,
        al_get_display_event_source(display)
    );

    al_register_event_source(
        event_queue,
        al_get_timer_event_source(timer)
    );

    al_register_event_source(
        event_queue,
        al_get_keyboard_event_source()
    );

    al_start_timer(timer);

    // Player

    int pos_x = 100;
    int pos_y = 150;
    float frame = 0.f;
    int current_frame_y = 132;

	// Pontuação, vida e tempo

    int areaHUD = 120;
    int pontuacao = 0;
    int vida = 3;
    float tempo_restante = 60;
    bool fimDeJogo = false;

	// Dano e invencibilidade

    bool tomou_dano = false;
    float tempo_dano = 0;

	// Animação inimigo

    float frameInimigo = 0.f;

    // Teclas

    bool teclas[4] = {
        false,
        false,
        false,
        false
    };

    // Inimigos

    int qtd_inimigos = 3;

    Inimigo inimigos[3];

    for (int i = 0; i < qtd_inimigos; i++) {

        inimigos[i].pos.x =
            rand() % 1200;

        inimigos[i].pos.y =
            rand() % 500 + areaHUD;

        inimigos[i].velocidadeX =
            (rand() % 5) + 3;
    }

    // Itens

    int qtd_itens = 5;
    Item itens[5];

    for (int i = 0; i < qtd_itens; i++) {

        itens[i].x =
            rand() % 1200 + 20;

        itens[i].y =
            rand() % 500 + areaHUD;
    }

    // Ranking

    Ranking ranking[100];
    int qtdRanking = 0;

	// Ler ranking do arquivo

    FILE* leitura =
        fopen("ranking.dat", "rb");

    if (leitura != NULL) {

        while (fread(
            &ranking[qtdRanking],
            sizeof(Ranking),
            1,
            leitura) == 1) {

            qtdRanking++;
        }

        fclose(leitura);
    }

    bool sair = false;

    // Loop Principal

    while (!sair) {

        ALLEGRO_EVENT event;

        al_wait_for_event(
            event_queue,
            &event
        );

		// Fechar janela

        if (event.type ==
            ALLEGRO_EVENT_DISPLAY_CLOSE) {
            sair = true;
        }

		// Teclas pressionadas

        if (event.type ==
            ALLEGRO_EVENT_KEY_DOWN) {

            switch (event.keyboard.keycode) {

            case ALLEGRO_KEY_UP:
                teclas[0] = true;
                break;
            case ALLEGRO_KEY_DOWN:
                teclas[1] = true;
                break;
            case ALLEGRO_KEY_LEFT:
                teclas[2] = true;
                break;
            case ALLEGRO_KEY_RIGHT:
                teclas[3] = true;
                break;
            case ALLEGRO_KEY_ESCAPE:
                sair = true;
                break;
            }
        }

        else if (event.type ==
            ALLEGRO_EVENT_KEY_UP) {

            switch (event.keyboard.keycode) {

            case ALLEGRO_KEY_UP:
                teclas[0] = false;
                break;
            case ALLEGRO_KEY_DOWN:
                teclas[1] = false;
                break;
            case ALLEGRO_KEY_LEFT:
                teclas[2] = false;
                break;
            case ALLEGRO_KEY_RIGHT:
                teclas[3] = false;
                break;
            }
        }

		// Timer (Atualização do jogo)

        if (event.type ==
            ALLEGRO_EVENT_TIMER) {

            // End game

            if (fimDeJogo) {

				// Procurar jogador no ranking (Busca Linear)

                int indiceJogador = -1;

                for (int i = 0; i < qtdRanking; i++) {

                    if (strcmp(
                        ranking[i].nome,
                        nomeJogador) == 0) {

                        indiceJogador = i;

                        break;
                    }
                }

				// Adiciona jogador ao ranking

                if (indiceJogador == -1) {

                    strcpy_s(
                        ranking[qtdRanking].nome,
                        sizeof(ranking[qtdRanking].nome),
                        nomeJogador
                    );

                    ranking[qtdRanking].pontos =
                        pontuacao;

                    qtdRanking++;
                }

				// Atualiza pontuação se jogador já existir

                else {

                    if (pontuacao >
                        ranking[indiceJogador].pontos) {

                        ranking[indiceJogador].pontos =
                            pontuacao;
                    }
                }

				// Ordena ranking

                ordenarRanking(
                    ranking,
                    qtdRanking
                );

				// Salvar ranking no arquivo

                FILE* salvar =
                    fopen("ranking.dat", "wb");

                if (salvar != NULL) {

                    fwrite(
                        ranking,
                        sizeof(Ranking),
                        qtdRanking,
                        salvar
                    );

                    fclose(salvar);
                }

				// Tela de Fim de Jogo

                al_clear_to_color(
                    al_map_rgb(0, 0, 0)
                );

                char pontosFinais[100];

                sprintf_s(
                    pontosFinais,
                    sizeof(pontosFinais),
                    "Pontuacao Final: %d",
                    pontuacao
                );

                al_draw_text(
                    fontGrande,
                    al_map_rgb(255, 0, 0),

                    640,
                    100,

                    ALLEGRO_ALIGN_CENTER,

                    "FIM DE JOGO"
                );

                al_draw_text(
                    fontGrande,
                    al_map_rgb(255, 255, 0),

                    640,
                    180,

                    ALLEGRO_ALIGN_CENTER,

                    pontosFinais
                );

                // Ranking

                al_draw_text(
                    fontGrande,
                    al_map_rgb(255, 255, 255),

                    640,
                    300,

                    ALLEGRO_ALIGN_CENTER,

                    "RANKING"
                );

                for (int i = 0;
                    i < qtdRanking && i < 5;
                    i++) {

                    char textoRank[100];

                    sprintf_s(
                        textoRank,
                        sizeof(textoRank),

                        "%d. %s - %d",

                        i + 1,

                        ranking[i].nome,

                        ranking[i].pontos
                    );

                    al_draw_text(
                        font,
                        al_map_rgb(255, 255, 0),

                        640,
                        360 + (i * 35),

                        ALLEGRO_ALIGN_CENTER,

                        textoRank
                    );
                }

                al_flip_display();

                continue;
            }

			// Tempo restante

            tempo_restante -=
                1.0 / 30.0;

            if (tempo_restante <= 0) {

                fimDeJogo = true;
            }

			// Movimento do player

            if (teclas[0]) {

                pos_y -= 8;

                current_frame_y = 0;
            }

            if (teclas[1]) {

                pos_y += 8;

                current_frame_y = 132 * 2;
            }

            if (teclas[2]) {

                pos_x -= 8;

                current_frame_y = 132 * 3;
            }

            if (teclas[3]) {

                pos_x += 8;

                current_frame_y = 132;
            }

			// Animação do player

            frame += 0.3f;

            if (frame >= 3) {

                frame = 0;
            }

			// Animação inimigos

            frameInimigo += 0.2f;

            if (frameInimigo >= 3) {

                frameInimigo = 0;
            }

			// Movimento dos inimigos

            for (int i = 0; i < qtd_inimigos; i++) {

                inimigos[i].pos.x +=
                    inimigos[i].velocidadeX;

                if (inimigos[i].pos.x > 1280) {

                    inimigos[i].pos.x = -80;

                    inimigos[i].pos.y =
                        rand() % 500 + areaHUD;
                }
            }

			// Dano e invencibilidade

            if (tomou_dano) {

                tempo_dano -=
                    1.0 / 30.0;

                if (tempo_dano <= 0) {

                    tomou_dano = false;
                }
            }

			// Colisão Inimigos

            for (int i = 0; i < qtd_inimigos; i++) {

                if (!tomou_dano &&

                    pos_x <
                    inimigos[i].pos.x + 80 &&

                    pos_x + 120 >
                    inimigos[i].pos.x &&

                    pos_y <
                    inimigos[i].pos.y + 80 &&

                    pos_y + 120 >
                    inimigos[i].pos.y) {

                    vida--;

                    tomou_dano = true;

                    tempo_dano = 1.0f;

                    if (vida <= 0) {

                        fimDeJogo = true;
                    }
                }
            }

			// Colisão Itens

            for (int i = 0; i < qtd_itens; i++) {

                if (pos_x <
                    itens[i].x + 64 &&

                    pos_x + 120 >
                    itens[i].x &&

                    pos_y <
                    itens[i].y + 64 &&

                    pos_y + 120 >
                    itens[i].y) {

                    pontuacao++;

                    itens[i].x =
                        rand() % 1200 + 20;

                    itens[i].y =
                        rand() % 500 + areaHUD;

                    break;
                }
            }

            // Desenho

            al_clear_to_color(
                al_map_rgb(255, 255, 255)
            );

            al_draw_bitmap(
                bg,
                0,
                0,
                0
            );

            // Hud

            char vidaTexto[50];

            sprintf_s(
                vidaTexto,
                sizeof(vidaTexto),
                "Vida: %d",
                vida
            );

            al_draw_text(
                font,
                al_map_rgb(0, 255, 0),
                10,
                10,
                0,
                vidaTexto
            );

            char pontosTexto[50];

            sprintf_s(
                pontosTexto,
                sizeof(pontosTexto),
                "Pontos: %d",
                pontuacao
            );

            al_draw_text(
                font,
                al_map_rgb(255, 255, 0),
                10,
                35,
                0,
                pontosTexto
            );

            char tempoTexto[50];

            sprintf_s(
                tempoTexto,
                sizeof(tempoTexto),
                "Tempo: %d",
                (int)tempo_restante
            );

            al_draw_text(
                font,
                al_map_rgb(255, 0, 0),
                1100,
                10,
                0,
                tempoTexto
            );

            // Itens

            for (int i = 0; i < qtd_itens; i++) {

                if (itemSprite) {

                    int largura =
                        al_get_bitmap_width(itemSprite);

                    int altura =
                        al_get_bitmap_height(itemSprite);

                    al_draw_scaled_bitmap(

                        itemSprite,

                        0,
                        0,

                        largura,
                        altura,

                        itens[i].x,
                        itens[i].y,

                        64,
                        64,

                        0
                    );
                }
            }

            // Inimigos

            for (int i = 0; i < qtd_inimigos; i++) {

                int frameAtualInimigo =
                    (int)frameInimigo;

                al_draw_scaled_bitmap(

                    inimigoSprite,

                    frameAtualInimigo * 48,
                    0,

                    48,
                    48,

                    inimigos[i].pos.x,
                    inimigos[i].pos.y,

                    80,
                    80,

                    0
                );
            }

            // Player

            al_draw_bitmap_region(

                sprite,

                157 * (int)frame,

                current_frame_y,

                157,
                132,

                pos_x,
                pos_y,

                0
            );

            al_flip_display();
        }
    }

	// Limpeza de recursos

    al_destroy_bitmap(bg);
    al_destroy_bitmap(sprite);
    al_destroy_bitmap(inimigoSprite);
    al_destroy_bitmap(itemSprite);
    al_destroy_font(font);
    al_destroy_font(fontGrande);
    al_destroy_timer(timer);
    al_destroy_event_queue(event_queue);
    al_destroy_display(display);

    return 0;
}
