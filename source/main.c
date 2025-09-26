\
    // Flappy DS - Proof of concept game logic (libnds)
    // This implementation focuses on game logic and displays state using the DS consoles.
    // Replace placeholder drawing (console text) with sprites/tiles for final visuals.
    #include <nds.h>
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <time.h>

    // Game configuration
    #define SCREEN_W 256
    #define SCREEN_H 192
    #define TOTAL_H (SCREEN_H*2)   // 384
    #define GAP_H 20               // invisible gap thickness (conceptual)
    #define GRAVITY 1
    #define FLAP_V -6
    #define FPS 60
    #define TRANSITION_FRAMES 9    // ~0.15s at 60fps -> feel free to tune (user asked short interval)
    #define TUBE_SPEED 2
    #define TUBE_GAP 55
    #define MAX_TUBES 8

    typedef enum { MODE_MENU, MODE_DUAL, MODE_SINGLE } GameMode;
    typedef struct {
        int x;
        int gap_y; // 0..TOTAL_H
        int passed;
    } Tube;

    // Game state
    volatile int keysHeld;
    volatile int keysDownV;

    GameMode mode = MODE_MENU;
    int menu_selection = 0; // 0=dual,1=single
    int running = 1;

    // Player
    int bird_x = 60;
    int bird_y = 80;
    int bird_vy = 0;

    // tubes
    Tube tubes[MAX_TUBES];
    int tube_count = 0;

    int score = 0;
    int best_score = 0;
    int game_over = 0;

    // transition between screens
    int transitioning = 0;
    int transition_timer = 0;
    int saved_vy = 0;
    int transition_direction = 0; // 1 = top->bottom, -1 bottom->top

    // utility
    int rand_between(int a, int b){ return a + (rand() % (b - a + 1)); }

    void seed_rng(){ srand((unsigned int)time(NULL)); }

    void init_video_console(){
        videoSetMode(MODE_5_2D);
        vramSetBankA(VRAM_A_MAIN_BG);
        videoSetModeSub(MODE_5_2D);
        vramSetBankC(VRAM_C_SUB_BG);
        consoleDemoInit(); // top
        consoleInit(FALSE, NULL); // bottom
    }

    void reset_game(GameMode m){
        mode = m;
        score = 0;
        game_over = 0;
        transitioning = 0;
        transition_timer = 0;
        bird_x = 60;
        bird_y = (m==MODE_DUAL)?40:SCREEN_H/2;
        bird_vy = 0;
        // init tubes: spaced to the right
        tube_count = 4;
        for(int i=0;i<tube_count;i++){
            tubes[i].x = SCREEN_W + i*120;
            tubes[i].gap_y = rand_between(60, (TOTAL_H - 100));
            tubes[i].passed = 0;
        }
    }

    void update_tubes(){
        for(int i=0;i<tube_count;i++){
            tubes[i].x -= TUBE_SPEED;
        }
        if(tube_count && tubes[0].x + 40 < 0){
            // shift left
            for(int j=0;j<tube_count-1;j++) tubes[j]=tubes[j+1];
            tubes[tube_count-1].x = tubes[tube_count-2].x + 120;
            tubes[tube_count-1].gap_y = rand_between(60, (TOTAL_H - 100));
            tubes[tube_count-1].passed = 0;
        }
    }

    void do_physics_and_input(){
        scanKeys();
        keysHeld = keysHeld() | keysHeldRepeat(); // not ideal but serves demo
        int kdown = keysDown();
        if(kdown & KEY_START) running = 0;
        if(mode==MODE_MENU){
            // navigate menu with up/down
            if(kdown & KEY_UP) menu_selection = 0;
            if(kdown & KEY_DOWN) menu_selection = 1;
            if(kdown & KEY_A){
                reset_game((menu_selection==0)?MODE_DUAL:MODE_SINGLE);
            }
            return;
        }

        if(!game_over && !transitioning){
            if(kdown & KEY_A) bird_vy = FLAP_V;
            // physics
            bird_vy += 1; // gravity simplified per frame
            if(bird_vy > 8) bird_vy = 8;
            bird_y += bird_vy;

            // check entering invisible bar (conceptual): top->bottom or bottom->top
            if(mode==MODE_DUAL){
                if(bird_y < 0){ // leaving top upwards (rare)
                    transitioning = 1;
                    transition_timer = TRANSITION_FRAMES;
                    transition_direction = -1;
                    saved_vy = bird_vy;
                } else if(bird_y > SCREEN_H && bird_y < SCREEN_H + GAP_H){
                    // entering gap area between screens
                    transitioning = 1;
                    transition_timer = TRANSITION_FRAMES;
                    transition_direction = 1;
                    saved_vy = bird_vy;
                }
            }
        }
    }

    void update_game_logic(){
        if(mode==MODE_DUAL){
            if(transitioning){
                // while hidden, tubes still move (we simulate update_tubes elsewhere)
                transition_timer--;
                // keep applying gravity to saved_vy so that when reappears it falls naturally
                saved_vy += 1;
                if(transition_timer<=0){
                    transitioning = 0;
                    // reappear on other screen just past the invisible gap
                    if(transition_direction==1){
                        bird_y = SCREEN_H + GAP_H + 5;
                    }else{
                        bird_y = SCREEN_H - 5;
                    }
                    bird_vy = saved_vy;
                }
            } else if(!game_over){
                // normal world updates
                update_tubes();

                // check collisions with tubes
                for(int i=0;i<tube_count;i++){
                    int tx = tubes[i].x;
                    int gap = tubes[i].gap_y;
                    // We only check collision when bird is within same visible vertical area
                    if( bird_x + 8 > tx && bird_x - 8 < tx + 30 ){
                        // collision if not within gap
                        if(!(bird_y > gap - TUBE_GAP && bird_y < gap + TUBE_GAP)){
                            game_over = 1;
                        }
                    }
                    if(!tubes[i].passed && tx + 30 < bird_x){
                        tubes[i].passed = 1; score++;
                        if(score>best_score) best_score=score;
                    }
                }
                // ground collision (ground is at bottom of lower screen)
                if(bird_y >= (TOTAL_H - 15)){
                    game_over = 1;
                }
            }
        } else if(mode==MODE_SINGLE){
            // only top screen is active: tubes limited to screen H
            update_tubes();
            // collision similar but constrained to screen
            for(int i=0;i<tube_count;i++){
                int tx = tubes[i].x;
                int gap = tubes[i].gap_y % SCREEN_H; // map into top screen
                if( bird_x + 8 > tx && bird_x - 8 < tx + 30 ){
                    if(!(bird_y > gap - TUBE_GAP && bird_y < gap + TUBE_GAP)){
                        game_over = 1;
                    }
                }
                if(!tubes[i].passed && tx + 30 < bird_x){
                    tubes[i].passed = 1; score++;
                    if(score>best_score) best_score=score;
                }
            }
            if(bird_y >= (SCREEN_H - 15)) game_over = 1;
        }
    }

    void render_console(){
        // top screen console: show gameplay / title / GAME OVER
        consoleClear();
        if(mode==MODE_MENU){
            iprintf("\x1b[1;0HFLAPPY DS\n\n");
            iprintf("  %s Gioca con due schermi\n", menu_selection==0?"->":"  ");
            iprintf("  %s Gioca con schermo superiore\n\n", menu_selection==1?"->":"  ");
            iprintf("Usa freccia su/giu per scegliere, A per confermare\n");
        } else {
            if(mode==MODE_DUAL){
                iprintf("\x1b[1;0HDual-screen mode (field 384px)\n");
            } else {
                iprintf("\x1b[1;0HSingle-screen mode (top only)\n");
            }
            iprintf("Score: %d   Best: %d\n", score, best_score);
            iprintf("Bird Y: %d  VY:%d\n", bird_y, bird_vy);
            iprintf("Transitioning: %d  GameOver: %d\n", transitioning, game_over);
            if(game_over){
                iprintf("\nGAME OVER\n");
            }
        }

        // bottom screen console: commands or results
        consoleInit(screenBottom, NULL);
        iprintf("\x1b[0;0H");
        if(mode==MODE_MENU){
            iprintf("Menu help: seleziona con su/giu, A per confermare\n");
        } else if(mode==MODE_SINGLE){
            if(!game_over){
                iprintf("Comandi:\n- Tocca lo schermo\n- Premi A o B\n");
            } else {
                iprintf("Score: %d\nBest: %d\nMedal: %s\n", score, best_score,
                        (score<5) ? "Bronze" : (score<10) ? "Silver" : (score<20) ? "Gold" : "Platinum");
            }
        } else if(mode==MODE_DUAL){
            if(!game_over){
                iprintf("Dual mode active: top+bottom. Ground on bottom screen.\n");
            } else {
                iprintf("Score: %d\nBest: %d\nMedal: %s\n", score, best_score,
                        (score<5) ? "Bronze" : (score<10) ? "Silver" : (score<20) ? "Gold" : "Platinum");
            }
        }
    }

    int main(void){
        irqInit();
        irqEnable(IRQ_VBLANK);
        init_video_console();
        seed_rng();
        reset_game(MODE_MENU);

        while(running){
            swiWaitForVBlank();
            do_physics_and_input();
            update_game_logic();
            render_console();
        }

        return 0;
    }
