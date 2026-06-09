#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifdef _WIN32
  #include <conio.h>
  #define LIMPAR system("cls")
  #define GETCH() _getch()
#else
  #include <termios.h>
  #include <unistd.h>
  #define LIMPAR system("clear")
  char GETCH() {
      struct termios t_old, t_new;
      tcgetattr(STDIN_FILENO, &t_old);
      t_new = t_old;
      t_new.c_lflag &= ~(ICANON | ECHO);
      tcsetattr(STDIN_FILENO, TCSANOW, &t_new);
      char c = getchar();
      tcsetattr(STDIN_FILENO, TCSANOW, &t_old);
      return c;
  }
#endif

/* ===================== CONSTANTES ===================== */
#define MAX_MONSTROS 20
#define L1  10
#define C1  10
#define L2  15
#define C2  15
#define L3  25
#define C3  25

/* ===================== VARIAVEIS GLOBAIS ===================== */

/* -- Jogador -- */
int px, py;           
char pdir;            
int vidas;
int arma;             
int tem_chave;        
int arma_escolhida;   

/* -- Monstros -- */
int mL[MAX_MONSTROS]; 
int mC[MAX_MONSTROS]; 
int mT[MAX_MONSTROS]; 
int mV[MAX_MONSTROS]; 
int mHP[MAX_MONSTROS];
int nmonstros;       

/* -- Mapas -- */
char vila   [L1][C1];
char andar1 [L1][C1];
char andar2 [L2][C2];
char andar3 [L3][C3];

/* backups para reiniciar fase */
char bk_vila   [L1][C1];
char bk_andar1 [L1][C1];
char bk_andar2 [L2][C2];
char bk_andar3 [L3][C3];

/* dimensões do mapa atual */
int ML, MC;
int fase; 

/* ===================== ACESSO AO MAPA ATUAL ===================== */
char obter(int l, int c) {
    if (fase == 0) return vila[l][c];
    if (fase == 1) return andar1[l][c];
    if (fase == 2) return andar2[l][c];
    return andar3[l][c];
}
void definir(int l, int c, char v) {
    if (fase == 0) vila[l][c]   = v;
    else if (fase == 1) andar1[l][c] = v;
    else if (fase == 2) andar2[l][c] = v;
    else andar3[l][c] = v;
}

/* ===================== EXIBIÇÃO ===================== */
void exibir_mapa() {
    int l, c;
    printf("\n  VIDAS: ");
    for (l = 0; l < vidas; l++) printf("?? ");
    printf("  ARMA: %s  CHAVES: %d\n\n",
        arma == 0 ? "Espada" : arma == 1 ? "Arco" : "Cajado", tem_chave);

    for (l = 0; l < ML; l++) {
        for (c = 0; c < MC; c++) {
            printf("%c ", obter(l, c));
        }
        printf("\n");
    }
    printf("\n[wasd] mover  [o] atacar  [i] interagir  [q] sair\n");
}

/* ===================== BACKUP / RESTAURAR ===================== */
void fazer_backup() {
    int l, c;
    if (fase == 0) { for(l=0;l<L1;l++) for(c=0;c<C1;c++) bk_vila[l][c]   = vila[l][c]; }
    if (fase == 1) { for(l=0;l<L1;l++) for(c=0;c<C1;c++) bk_andar1[l][c] = andar1[l][c]; }
    if (fase == 2) { for(l=0;l<L2;l++) for(c=0;c<C2;c++) bk_andar2[l][c] = andar2[l][c]; }
    if (fase == 3) { for(l=0;l<L3;l++) for(c=0;c<C3;c++) bk_andar3[l][c] = andar3[l][c]; }
}
void restaurar_backup() {
    int l, c;
    if (fase == 0) { for(l=0;l<L1;l++) for(c=0;c<C1;c++) vila[l][c]   = bk_vila[l][c]; }
    if (fase == 1) { for(l=0;l<L1;l++) for(c=0;c<C1;c++) andar1[l][c] = bk_andar1[l][c]; }
    if (fase == 2) { for(l=0;l<L2;l++) for(c=0;c<C2;c++) andar2[l][c] = bk_andar2[l][c]; }
    if (fase == 3) { for(l=0;l<L3;l++) for(c=0;c<C3;c++) andar3[l][c] = bk_andar3[l][c]; }
}

/* ===================== MONSTROS ===================== */
void adicionar_monstro(int l, int c, int tipo, int hp) {
    if (nmonstros >= MAX_MONSTROS) return;
    mL[nmonstros] = l;
    mC[nmonstros] = c;
    mT[nmonstros] = tipo;
    mV[nmonstros] = 1;
    mHP[nmonstros] = hp;
    nmonstros++;
}

void desenhar_monstros() {
    int i;
    char simbolo;
    for (i = 0; i < nmonstros; i++) {
        if (!mV[i]) continue;
        simbolo = mT[i] == 1 ? 'X' : mT[i] == 2 ? 'Y' : 'Z';
        definir(mL[i], mC[i], simbolo);
    }
}

void mover_monstros() {
    int i, dl, dc, dirs[4][2] = {{-1,0},{1,0},{0,-1},{0,1}};
    int nl, nc, d;
    for (i = 0; i < nmonstros; i++) {
        if (!mV[i]) continue;

        /* apaga posição atual */
        definir(mL[i], mC[i], ' ');

        if (mT[i] == 1) {
            /* Tipo 1: movimento aleatorio */
            d = rand() % 4;
            nl = mL[i] + dirs[d][0];
            nc = mC[i] + dirs[d][1];
        } else if (mT[i] == 2) {
            /* Tipo 2: perseguição simples */
            dl = px - mL[i];
            dc = py - mC[i];
            if (abs(dl) >= abs(dc)) {
                nl = mL[i] + (dl > 0 ? 1 : -1);
                nc = mC[i];
            } else {
                nl = mL[i];
                nc = mC[i] + (dc > 0 ? 1 : -1);
            }
        } else {
            /* Boss (Tipo 3): alterna entre furia e pulso */
            dl = px - mL[i];
            dc = py - mC[i];
            if (abs(dl) >= abs(dc)) {
                nl = mL[i] + (dl > 0 ? 1 : -1);
                nc = mC[i];
            } else {
                nl = mL[i];
                nc = mC[i] + (dc > 0 ? 1 : -1);
            }
        }

        /* valida movimento: dentro do mapa e célula livre */
        if (nl >= 0 && nl < ML && nc >= 0 && nc < MC) {
            char dest = obter(nl, nc);
            if (dest == ' ') {
                mL[i] = nl;
                mC[i] = nc;
            }
        }
    }
    desenhar_monstros();
}

/* ===================== DANO AO MONSTRO ===================== */
void dano_celula(int l, int c) {
    int i;
    if (l < 0 || l >= ML || c < 0 || c >= MC) return;
    for (i = 0; i < nmonstros; i++) {
        if (!mV[i]) continue;
        if (mL[i] == l && mC[i] == c) {
            mHP[i]--;
            if (mHP[i] <= 0) {
                mV[i] = 0;
                definir(l, c, ' ');
            }
        }
    }
    /* destrói caixa */
    if (obter(l, c) == 'k') definir(l, c, ' ');
}

/* ===================== ATAQUE ===================== */
void atacar() {
    int i;

    if (arma == 0) {
        /* Espada: 3x2 à frente */
        int dl = (pdir == '^' || pdir == 'v') ? (pdir == 'v' ? 1 : -1) : 0;
        int dc = (pdir == '<' || pdir == '>') ? (pdir == '>' ? 1 : -1) : 0;
        for (i = 1; i <= 2; i++) {
            dano_celula(px + dl*i - dc, py + dc*i - dl);
            dano_celula(px + dl*i,      py + dc*i);
            dano_celula(px + dl*i + dc, py + dc*i + dl);
        }
    } else if (arma == 1) {
        /* Arco: 4 celulas em linha reta */
        int dl = (pdir == '^') ? -1 : (pdir == 'v') ? 1 : 0;
        int dc = (pdir == '<') ? -1 : (pdir == '>') ? 1 : 0;
        for (i = 1; i <= 4; i++) dano_celula(px + dl*i, py + dc*i);
    } else {
        /* Cajado: 8 celulas adjacentes */
        int dirs[8][2] = {{-1,-1},{-1,0},{-1,1},{0,-1},{0,1},{1,-1},{1,0},{1,1}};
        for (i = 0; i < 8; i++) dano_celula(px + dirs[i][0], py + dirs[i][1]);
    }
}

/* ===================== COLISÃO COM MONSTRO ===================== */
int colidiu_monstro() {
    int i;
    for (i = 0; i < nmonstros; i++) {
        if (mV[i] && mL[i] == px && mC[i] == py) return 1;
    }
    return 0;
}

/* ===================== PERDER VIDA ===================== */
/* retorna 1 se game over */
int perder_vida() {
    vidas--;
    if (vidas <= 0) return 1;
    /* reinicia fase */
    restaurar_backup();
    nmonstros = 0;
    /* reposiciona monstros conforme a fase */
    if (fase == 1) { adicionar_monstro(2, 7, 1, 1); }
    if (fase == 2) { adicionar_monstro(3, 3, 1, 1); adicionar_monstro(10, 10, 1, 1); }
    if (fase == 3) {
        adicionar_monstro(5, 5, 2, 1);
        adicionar_monstro(20, 20, 3, 3);
    }
    px = (fase == 0) ? 8 : 1;
    py = (fase == 0) ? 1 : 1;
    pdir = 'v';
    tem_chave = 0;
    return 0;
}

/* ===================== VERIFICAR VITÓRIA ===================== */
int boss_morto() {
    int i;
    for (i = 0; i < nmonstros; i++) {
        if (mT[i] == 3 && mV[i]) return 0;
    }
    return 1;
}

/* ===================== INTERAGIR ===================== */
void interagir() {
    int fl = px + (pdir == 'v' ? 1 : pdir == '^' ? -1 : 0);
    int fc = py + (pdir == '>' ? 1 : pdir == '<' ? -1 : 0);
    if (fl < 0 || fl >= ML || fc < 0 || fc >= MC) return;

    char obj = obter(fl, fc);

    if (obj == '@') { /* chave */
        tem_chave++;
        definir(fl, fc, ' ');
        printf("Você pegou uma chave!\n");
    } else if (obj == 'D') { /* porta fechada */
        if (tem_chave > 0) {
            tem_chave--;
            definir(fl, fc, '=');
            printf("Porta aberta!\n");
        } else {
            printf("Você precisa de uma chave!\n");
        }
    } else if (obj == 'O') { /* botão */
        definir(fl, fc, 'o');
        definir(7, 7, ' ');
        printf("Botão ativado!\n");
    } else if (obj == 'N') { /* NPC da vila */
        LIMPAR;
        printf("\nDorin: Qual arma voce quer levar?\n\n");
        printf("  1 - Espada (ataca 3x2 a frente)\n");
        printf("  2 - Arco   (ataca 4 celulas em linha)\n");
        printf("  3 - Cajado (ataca 8 celulas ao redor)\n\n");
        printf("Opcao (1/2/3): ");
        char op;
        do { op = GETCH(); } while (op < '1' || op > '3');
        arma = op - '1';
        arma_escolhida = 1;
        printf("\nArma escolhida: %s\n",
            arma == 0 ? "Espada" : arma == 1 ? "Arco" : "Cajado");
        printf("Boa sorte, Kivimag. Pressione qualquer tecla...\n");
        GETCH();
    }
}

/* ===================== INICIALIZAR MAPAS ===================== */
void init_vila() {
    int l, c;
    ML = L1; MC = C1;
    for (l = 0; l < L1; l++)
        for (c = 0; c < C1; c++)
            vila[l][c] = ' ';
    for (l = 0; l < L1; l++) vila[l][0] = vila[l][C1-1] = '*';
    for (c = 0; c < C1; c++) vila[0][c] = vila[L1-1][c] = '*';
    vila[3][4] = 'N';
    vila[1][8] = 'L';
    px = 8; py = 1; pdir = 'v';
    vila[px][py] = pdir;
    fazer_backup();
}

void init_andar1() {
    int l, c;
    ML = L1; MC = C1;
    for (l = 0; l < L1; l++)
        for (c = 0; c < C1; c++)
            andar1[l][c] = ' ';
    for (l = 0; l < L1; l++) andar1[l][0] = andar1[l][C1-1] = '*';
    for (c = 0; c < C1; c++) andar1[0][c] = andar1[L1-1][c] = '*';
    andar1[5][5] = 'k';
    andar1[2][3] = '@';
    andar1[7][7] = 'D';
    andar1[8][8] = 'L';
    adicionar_monstro(2, 7, 1, 1);
    px = 1; py = 1; pdir = 'v';
    andar1[px][py] = pdir;
    fazer_backup();
    desenhar_monstros();
}

void init_andar2() {
    int l, c;
    ML = L2; MC = C2;
    for (l = 0; l < L2; l++)
        for (c = 0; c < C2; c++)
            andar2[l][c] = ' ';
    for (l = 0; l < L2; l++) andar2[l][0] = andar2[l][C2-1] = '*';
    for (c = 0; c < C2; c++) andar2[0][c] = andar2[L2-1][c] = '*';
    andar2[5][3] = '#'; andar2[5][4] = '#'; andar2[5][5] = '#';
    andar2[3][7] = 'k';
    andar2[2][2]   = '@'; andar2[12][12] = '@';
    andar2[6][6]   = 'D'; andar2[13][13] = 'D';
    andar2[8][8] = 'O';
    andar2[7][7] = '*';
    andar2[13][12] = 'L';
    adicionar_monstro(3, 3, 1, 1);
    adicionar_monstro(10, 10, 1, 1);
    px = 1; py = 1; pdir = 'v';
    andar2[px][py] = pdir;
    fazer_backup();
    desenhar_monstros();
}

void init_andar3() {
    int l, c;
    ML = L3; MC = C3;
    for (l = 0; l < L3; l++)
        for (c = 0; c < C3; c++)
            andar3[l][c] = ' ';
    for (l = 0; l < L3; l++) andar3[l][0] = andar3[l][C3-1] = '*';
    for (c = 0; c < C3; c++) andar3[0][c] = andar3[L3-1][c] = '*';
    for (c = 5; c < 20; c++) andar3[12][c] = '#';
    andar3[12][10] = ' ';
    andar3[5][5] = 'k'; andar3[5][19] = 'k';
    andar3[3][3]   = '@'; andar3[3][21]  = '@'; andar3[21][12] = '@';
    andar3[10][10] = 'D'; andar3[10][14] = 'D'; andar3[15][12] = 'D';
    andar3[8][12] = 'O';
    andar3[9][12] = '*';
    adicionar_monstro(5,  5,  2, 1);
    adicionar_monstro(5,  19, 2, 1);
    adicionar_monstro(19, 5,  2, 1);
    adicionar_monstro(22, 12, 3, 3);
    andar3[23][12] = 'L';
    px = 1; py = 1; pdir = 'v';
    andar3[px][py] = pdir;
    fazer_backup();
    desenhar_monstros();
}

/* ===================== INICIAR FASE ===================== */
void iniciar_fase(int f) {
    fase = f;
    nmonstros = 0;
  
    if (f == 0) init_vila();
    else if (f == 1) init_andar1();
    else if (f == 2) init_andar2();
    else init_andar3();
}

/* ===================== TELAS ===================== */
void tela_game_over() {
    LIMPAR;
    printf("\n\n");
    printf("  ================================\n");
    printf("          G A M E  O V E R        \n");
    printf("  ================================\n");
    printf("  Kivimag caiu nas trevas de Valdrek.\n");
    printf("  O Cristal de Arius permanece    \n");
    printf("  apagado... por enquanto.         \n");
    printf("  ================================\n\n");
    printf("  Pressione qualquer tecla...\n");
    GETCH();
}

void tela_vitoria() {
    LIMPAR;
    printf("\n\n");
    printf("  ================================\n");
    printf("         V I T O R I A !          \n");
    printf("  ================================\n");
    printf("  Mordak cai de joelhos.           \n");
    printf("  O Cristal de Arius pulsa e volta \n");
    printf("  a brilhar. Apollo esta salva.    \n\n");
    printf("  Dorin olha para o ceu pela       \n");
    printf("  primeira vez em semanas.          \n\n");
    printf("  - Obrigado, Kivimag.                \n");
    printf("  ================================\n\n");
    printf("  Pressione qualquer tecla...\n");
    GETCH();
}

void tela_tutorial() {
    LIMPAR;
    printf("\n=== TUTORIAL — AS RUINAS DE ARTTEMIS ===\n\n");
    printf("HISTORIA:\n");
    printf("  O Cristal de Arius misteriosamente se apagou. Voce e Kivimag,\n");
    printf("  o unico habitante que nao fugiu de Apollo.\n");
    printf("  Entre na masmorra e restaure o cristal.\n\n");
    printf("SIMBOLOS:\n");
    printf("  ^ v < >  Jogador (direção)\n");
    printf("  *        Parede\n");
    printf("  #        Espinho (mata ao tocar)\n");
    printf("  k        Caixa (destruivel com ataque)\n");
    printf("  O        Botao (ativa algo ao interagir)\n");
    printf("  D        Porta fechada\n");
    printf("  =        Porta aberta\n");
    printf("  @        Chave\n");
    printf("  L        Escada (proxima fase)\n");
    printf("  X        Monstro tipo 1 (aleatorio)\n");
    printf("  Y        Monstro tipo 2 (perseguicao)\n");
    printf("  Z        Boss Final\n");
    printf("  N        NPC\n\n");
    printf("CONTROLES:\n");
    printf("  wasd     Mover\n");
    printf("  o        Atacar\n");
    printf("  i        Interagir\n");
    printf("  q        Sair\n\n");
    printf("Pressione qualquer tecla para voltar...\n");
    GETCH();
}

void tela_creditos() {
    LIMPAR;
    printf("\n=== CREDITOS ===\n\n");
    printf("  As Ruinas de Arttemis\n\n");
    printf("  Desenvolvido por:\n");
    printf("  - [Joao Pedro Valois Deltetto Chagas]\n");
    printf("  - [Lucas Braz Alho]\n");
    printf("  - [Joao Gabriel Valadares ]\n\n");
    printf("  Obrigado por jogar!\n\n");
    printf("Pressione qualquer tecla para sair...\n");
    GETCH();
}

/* ===================== MENU ===================== */
int menu_principal() {
    char op;
    while (1) {
        LIMPAR;
        printf("\n");
        printf("  ================================\n");
        printf("    AS RUINAS DE ARTTEMIS          \n");
        printf("  ================================\n");
        printf("  1. Jogar                        \n");
        printf("  2. Tutorial                     \n");
        printf("  3. Sair                         \n");
        printf("  ================================\n");
        printf("  Opcao: ");
        op = GETCH();
        if (op == '1') return 1;
        if (op == '2') { tela_tutorial(); }
        if (op == '3') { tela_creditos(); return 0; }
    }
}

/* ===================== MOVER JOGADOR ===================== */
/* retorna 1 se subiu escada */
int mover_jogador(char tecla) {
    int nl = px, nc = py;

    if (tecla == 'w') { nl = px - 1; nc = py;     pdir = '^'; }
    if (tecla == 's') { nl = px + 1; nc = py;     pdir = 'v'; }
    if (tecla == 'a') { nl = px;     nc = py - 1; pdir = '<'; }
    if (tecla == 'd') { nl = px;     nc = py + 1; pdir = '>'; }

    if (nl < 0 || nl >= ML || nc < 0 || nc >= MC) return 0;

    char dest = obter(nl, nc);

    if (dest == '*' || dest == 'k' || dest == 'D') return 0;

    
    char dest_salvo = dest;
    definir(px, py, ' ');
    px = nl; py = nc;
    
    if (dest_salvo != '#')
        definir(px, py, pdir);

    /* escada — na vila so avança se escolheu arma */
    if (dest_salvo == 'L') {
        if (fase == 0 && !arma_escolhida) {
            printf("Fale com Roivel antes de entrar na masmorra!\n");
            definir(px, py, pdir);
            return 0;
        }
        return 1;
    }

    return 0;
}

/* ===================== LOOP DO JOGO ===================== */
void jogar() {
    vidas = 3;
    arma  = 0;
    arma_escolhida = 0;
    tem_chave = 0;
    iniciar_fase(0);

    char tecla;
    int subiu, game_over;

    while (1) {
        LIMPAR;
        exibir_mapa();

        tecla = GETCH();
        if (tecla == 'q') break;

        subiu = 0;

        if (tecla == 'w' || tecla == 'a' || tecla == 's' || tecla == 'd') {
            subiu = mover_jogador(tecla);
        } else if (tecla == 'o') {
            atacar();
        } else if (tecla == 'i') {
            interagir();
        }

        if (subiu) {
            if (fase < 3) iniciar_fase(fase + 1);
            continue;
        }

        mover_monstros();

        if (fase == 3 && boss_morto()) {
            tela_vitoria();
            return;
        }

        if (obter(px, py) == '#') {
            game_over = perder_vida();
            if (game_over) { tela_game_over(); return; }
            continue;
        }

        if (colidiu_monstro()) {
            game_over = perder_vida();
            if (game_over) { tela_game_over(); return; }
        }
    }
}

/* ===================== MAIN ===================== */
int main() {
    srand(time(NULL));
    if (menu_principal()) jogar();
    return 0;
}