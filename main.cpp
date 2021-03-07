#define playeryCenter (player[side].y+player[side].h/2)
#define playerTop player[side].y
#define playerBottom  (player[side].y+player[side].h)
#define player1x (player[0].x+player[0].w)
#define player2x player[1].x

#define ballyCenter (ball.y+ball.h/2)
#define ballxLeft ball.x
#define ballxRight (ball.x+ball.w)
#define ballTop ball.y
#define ballBottom (ball.y+ball.h)

#define borderTop (borderOffset*2+borderThickness)
#define borderBottom (820-(borderOffset*2+borderThickness))

#include"includes.h"

using namespace std;
const double pi = 3.1415926535897;
const unsigned short width = 1200, height = 820, gameSpeed = 10, borderThickness = 20, borderOffset = 10, ballSize=20,

            playerSpeed = 10, ballDefaultSpeed = 14, ballBonusSpeedOnHit = 4, maxscore = 10, FPS = 40;
//I use 820 as height because after the borders, we are left with 760 pixels height, which is 95*8,
// so in the center line  we can have 48 black and 47 white 8x8 squares(or rectangles with height 8),
//  alternating between black and white, starting with black and ending with black
SDL_Renderer* gRenderer = NULL;

SDL_Rect border[2]={
                     {borderOffset, borderOffset,                        width-2*borderOffset, borderThickness},
                     {borderOffset, height-borderOffset-borderThickness, width-2*borderOffset, borderThickness}
                   },//We declare the two borders,
         middledottedline={width/2-4 , borderOffset+borderThickness+8, 6, 8};       // and also the square we use to prunsigned short the middle line

bool keyPressed[4]={0,0,0,0};
enum{w=0,s,up,down};

bool side,scored=0,pause=0, hasHitPlayer=0, spectate;
enum{LEFT=0, RIGHT};

unsigned short option, difficulty, score[2]={0,0};

Mix_Chunk *hitPlayer;
Mix_Chunk *hitBorder;

SDL_Rect player[2],ball;

double ballSpeed, ballAngle;

//==================================================================================================
//==================================================================================================
//==================================================================================================
//==================================================================================================
//==================================================================================================
//==================================================================================================
int main(int argc, char* args[])
{
    SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO|SDL_INIT_EVENTS);
    Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 2, 2048 );//Initialise
    SDL_Window *window=NULL;
    window = SDL_CreateWindow( "Pong", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN );
    gRenderer = SDL_CreateRenderer( window, -1, SDL_RENDERER_ACCELERATED );

    hitPlayer=Mix_LoadWAV("playerhit.wav");
    hitBorder=Mix_LoadWAV("borderhit.wav");


    for(int i=0;i<2;i++)
    {
        player[i].w=20;
        player[i].h=120;
        player[i].y=height/2-player[i].h/2;
    }
    player[0].x=60-player[0].w/2;         player[1].x=width-60-player[0].w/2;

    ball.w=ballSize;  ball.h=ballSize;      ball.x=width/2-ball.w/2;  ball.y=height/2-ball.h/2;

    ballSpeed = ballDefaultSpeed;
    ballAngle = getRandomAngle();

    draw();
    //end of initialisation

    SDL_Event e;

    char key='d';//d for defult
    cout<<"Select mode:\n 1-Singleplayer\n 2-Multiplayer\n 3-Spectate the hard bot on the left vs the easy bot on the right\n\n";
    while(key<'1' || key>'3')
    {
        key=getKeyPress(e);
    }
    option=key-48;
    key='d';

    spectate=(option==3);
    if(spectate) difficulty=3;
    if(option==1)
    {
        cout<<"Which bot would you like to play against? \n 1-Easy \n 2-Hard\n\n";
        while(key<'1' || key>'2')
        {
            key=getKeyPress(e);
        }
        difficulty=key-48;
    }
    if(option==1 || option==2)
    {
        cout<<" Player 1 controls- W and S \n";
        if(option==2)
        {
            cout<<" Player 2 controls- Arrow keys(up and down)\n";
        }
    }



    bool quit=false;
    SDL_TimerID tim= SDL_AddTimer(1000/FPS, timer, NULL);
    ///set up a timer that renders our game FPS times per second!
    ///Also creates another thread, so performance is a bit better compared to rendering in our game loop down there

    while(!quit)
    {
        if( e.type == SDL_QUIT )
        {
            quit = true;
        }


        eventHandler(e);
        if(option==1 || option==3)
        {
            makeComputerMove(ballAngle);
        }
        playerMovement();
        ballMovement(ballSpeed, ballAngle);
        //draw();
        if(scored==1)
        {
            cout<<" Score: "<<score[0]<<"|"<<score[1]<<endl;
            hasHitPlayer=false;
            for(int i=0;i<2;i++)
            {
                if(score[i] == maxscore)
                {
                    cout<<"\n Player "<<i+1<<" wins!\n\n Keep playing?\n y/n \n\n";
                    score[0]=0;
                    score[1]=0;
                    while(key!='y' && key!='n')
                    {
                        key=getKeyPress(e);
                    }
                    if(key=='n') quit=true;
                    key='d';
                }
            }
            for(int i=0;i<4;i++)
            {
                keyPressed[i]=false;
            }
            draw();
            SDL_RenderDrawLine(gRenderer, ball.x+ball.w/2, ballyCenter, ball.x+ball.w/2+70*cos(ballAngle), ballyCenter-70*sin(ballAngle));
            ///Draws a line with length 70 showing the direction of the ball
            SDL_RenderPresent(gRenderer);
            SDL_Delay(700);
            scored=0;

        }

        SDL_Delay(166/gameSpeed);///it was previously 1000/fps=16.6 where default fps was 60, but i converted it
                                 /// to 166/gamespeed=16.6 for default gamespeed of 10 since gamespeed is more accurate and 10 is a nice number
    }

    SDL_RemoveTimer(timer);
    Mix_FreeChunk(hitBorder);
    Mix_FreeChunk(hitPlayer);
    SDL_DestroyRenderer(gRenderer);
    SDL_DestroyWindow(window);
    Mix_Quit();
    SDL_Quit();
    return 0;
}

void draw()
{
    SDL_SetRenderDrawColor( gRenderer, 0x00, 0x00, 0x00, 0x00 );
    SDL_RenderClear(gRenderer);
    ///Draw the black background

    SDL_SetRenderDrawColor( gRenderer, 0x99, 0x99, 0x99, 0x99 );
    SDL_RenderFillRects(gRenderer, &border[0], 2);
    while(middledottedline.y<height-borderThickness-borderOffset)
    {
        SDL_RenderFillRect(gRenderer, &middledottedline);
        middledottedline.y+=2*middledottedline.h;
    }
    middledottedline.y=borderOffset+borderThickness+8;
    ///draw the middle dotted line and the border

    SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );
    SDL_RenderFillRects(gRenderer, &player[0], 2);
    SDL_RenderFillRect(gRenderer, &ball);
    ///draw the two players and the ball

    SDL_RenderPresent(gRenderer);
    ///Comment this out if you enable the "tail" of the ball at the end
    ///this will cause the game to not render before you select options, so keep this in mind
    return;
}

void eventHandler(SDL_Event &e)
{
    while( SDL_PollEvent( &e ) != 0)
    {
        if( e.type == SDL_KEYDOWN  )
        {
            switch(e.key.keysym.sym)
            {
                case SDLK_w:    keyPressed[w]=true; break;
                case SDLK_s:    keyPressed[s]=true; break;
                case SDLK_UP:   keyPressed[up]=true; break;
                case SDLK_DOWN: keyPressed[down]=true; break;
                case SDLK_p:    while(getKeyPress(e)!='p');///pauses the game
            }

        }
        else if( e.type == SDL_KEYUP )
        {
              switch(e.key.keysym.sym)
              {
                case SDLK_w:    keyPressed[w]=false; break;
                case SDLK_s:    keyPressed[s]=false; break;
                case SDLK_UP:   keyPressed[up]=false; break;
                case SDLK_DOWN: keyPressed[down]=false;
          }
        }
    }
    return;
}

void playerMovement()
{
     if(keyPressed[w]==true    && belowTopBorder(player[0]))     player[0].y-=playerSpeed;
else if(keyPressed[s]==true    && aboveBottomBorder(player[0]))  player[0].y+=playerSpeed;

     if(keyPressed[up]==true   && belowTopBorder(player[1]))     player[1].y-=playerSpeed;
else if(keyPressed[down]==true && aboveBottomBorder(player[1]))  player[1].y+=playerSpeed;

    return;
}
//==================================================================================================
//==================================================================================================
//==================================================================================================
//==================================================================================================
//==================================================================================================
//==================================================================================================
void  ballMovement(double &ballSpeed, double &ballAngle)
{

    unsigned short finalballX = ball.x+cos(ballAngle)*ballSpeed;
    unsigned short finalballY = ball.y-sin(ballAngle)*ballSpeed;
    short ballxdirection = (cos(ballAngle)>0)? 1:-1;
    short ballydirection = (sin(ballAngle)>0)? -1:1;
    if(sin(ballAngle) == 0) ballydirection=0;
    if(cos(ballAngle) == 0) ballxdirection=0;

    while(ball.x!=finalballX || ball.y!=finalballY)
    {

        if( ball.y!=finalballY)
        {
            ball.y+=ballydirection;
        }
        if( ball.x!=finalballX)
        {
            ball.x+=ballxdirection;
        }///We could just teleport the ball for the final location, but this makes it much easier to handle ball hits and move the ball smoothly

        //bool hitborder=(((sin(ballAngle)>0 && ball.y > height/2) || ((sin(ballAngle) < 0) && ball.y < height/2)));
        //declare this so we don't get any bugs where the ball can ricochet more than once per hit

        if(ballTop < borderOffset+borderThickness || ballBottom > height-(borderOffset+borderThickness))
        {
            ballAngle=-ballAngle;
            ball.y+=5*((sin(ballAngle)>0)? -1:1);///move the ball away from the border on hit
            Mix_ExpireChannel(-1, 0);///stop playing if it's currently playing another sound effect
                                     ///not sure if it works and it's only useful when it bounces in a corner
            Mix_PlayChannel( -1, hitBorder, 0 );
            return;
        }

        bool inPlayerRange=((ballxLeft<=player1x && ballxLeft>=player1x-5) || (ballxRight >= player2x && ballxRight <= player2x+5));
        /// for hitting the left side of the right player/ right side of the left player
        bool justBehindPlayerRange=((ballxLeft<player1x-5 && ballxLeft>=player1x-30) || (ballxRight > player2x+5 && ballxRight <= player2x+30));
        /// for hitting the top or the bottom side of the player
        bool between=((ballTop > playerTop && ballTop < playerBottom) || (ballBottom > playerTop && ballBottom < playerBottom));
        ///check if the ball is between the top and the bottom of the player in the side where the ball currently is

        if(inPlayerRange && !hasHitPlayer && between)
        {

            hasHitPlayer=true;
            ballAngle=-ballAngle+pi;

            double offset=1.0*(playeryCenter-ballyCenter)/(player[side].h/2);

            double newangle=(cos(ballAngle)>0) ? (pi/4)*offset : pi-(pi/4)*offset;

            ballAngle=newangle;

            if(option==3 || ((option==1 && side==RIGHT) || ((keyPressed[0] || keyPressed[1]) && side==LEFT)) || (option==2 && (keyPressed[0+2*side] || keyPressed[1+2*side])))
            ///side is 0 when on the left side, and 1 on the right, so this way
            ///we only check the keys of the player whose side the ball is currently in
            {
                ballSpeed=ballDefaultSpeed+ballBonusSpeedOnHit;
            }
            else ballSpeed=ballDefaultSpeed;
            //ball.x=ball.x+20*((cos(ballAngle)>0)?1:-1);
            Mix_ExpireChannel(-1, 0);
            Mix_PlayChannel( -1, hitPlayer, 0 );
            return;
        }

        if(justBehindPlayerRange && !hasHitPlayer && between)
        {
            hasHitPlayer=true;
            ballAngle=-ballAngle;
            Mix_ExpireChannel(-1, 0);
            Mix_PlayChannel( -1, hitPlayer, 0 );
            return;
        }

        if(ballxLeft < 0 || ballxRight > width)
        {
            ball.x=width/2-ball.w/2;
            ball.y=310 - ball.h/2 + rand()%200+1;///spawns the ball somewhere in the middle 400 pixels
            ballAngle=getRandomAngle();
            ballSpeed=ballDefaultSpeed;
            scored=1;
            score[!side]++;
            return;
        }

        if(ball.x < width/2 && side!=LEFT)
        {
            side=LEFT;
            hasHitPlayer=false;

        }
        if(ball.x > width/2 && side!=RIGHT)
        {
            side=RIGHT;
            hasHitPlayer=false;
        }
    }

    return;
}



double getRandomAngle()
{
    srand(time(NULL));
    rand();
    ///We don't actually want a totally random angle, if the ball went at a 90(or 270) or close to that angle it would
    ///bounce a lot on the borders before getting to the players, so we make sure that doesn't happen

    double randomAngle=(rand() / (RAND_MAX + 1.))*2*pi;///generate a random angle between 0 and 2pi
    double anglelimit=60*(pi/180);
    ///eliminate angles that are less than 40 degrees from the y axis using the sin comparison below
    while(sin(randomAngle) > sin(anglelimit) || sin(randomAngle) < -sin(anglelimit))
    {
        randomAngle=(rand() / (RAND_MAX + 1.))*2*pi;
    }
    return randomAngle;
}

void makeComputerMove(double ballAngle)
{
    switch(difficulty)
    {
        case 3:
        case 1:
            {
                   if(player[1].y+player[1].h/2 - ballyCenter <= -10 )
                   {
                       if(aboveBottomBorder(player[1]))
                       {
                           player[1].y+=playerSpeed;
                       }
                   }
                    else if(player[1].y+player[1].h/2 - ballyCenter >= 10)
                    {
                        if(belowTopBorder(player[1]))
                        {
                            player[1].y-=playerSpeed;
                        }
                    }

                if(!spectate) break;
            }   ///Just follows the ball, simply trying to hit it with its center
        case 2:
            {   ///This one essentially does the same, however this one tries to hit the ball
                /// with its top when the ball is going down and its bottom when the ball is going up
                ///Because of this, it's much harder to miss the ball, while also returning shots
                ///that bounce a lot(which are harder to predict for the other player) since it hits the ball with its corner
                ///note: hits of bots are always accelerated
                int direction=(sin(ballAngle)>0)? -1 : 1;

                bool botside=1-spectate;

                if(player[botside].y+player[botside].h/2 <= ballyCenter + direction*(player[botside].h/2)-10)
                   {
                       if(aboveBottomBorder(player[1-spectate]))
                       {
                            player[1-spectate].y+=playerSpeed;
                       }
                   }
                else if(player[botside].y+player[botside].h/2 >= ballyCenter + direction*(player[botside].h/2)+10)
                   {
                       if(belowTopBorder(player[1-spectate]))
                       {
                            player[1-spectate].y-=playerSpeed;
                       }
                   }
                break;
            }

    }
}


char getKeyPress(SDL_Event &e)
{
    ///simple function for getting keypresses
    char key='d';

    while( SDL_PollEvent( &e ) != 0)
    {
        if( e.type == SDL_KEYDOWN  )
        {
            switch(e.key.keysym.sym)
            {
                case SDLK_1:    key='1'; break;
                case SDLK_2:    key='2'; break;
                case SDLK_3:    key='3'; break;
                case SDLK_y:    key='y'; break;
                case SDLK_p:    key='p'; break;
                case SDLK_n:    key='n';
            }

        }
    }
    return key;
}

bool belowTopBorder(SDL_Rect player)
{
    return (player.y > borderTop);
}
bool aboveBottomBorder(SDL_Rect player)
{
    return (player.y+player.h < borderBottom);
}

///

Uint32 timer(Uint32 interval, void *param)
{
    if(scored==0)
    {
        draw();
        /*
        SDL_RenderDrawLine(gRenderer, (tan(ballAngle) > 0)?ball.x:ball.x+ball.w, ballTop   , ball.x+ball.w/2-5*cos(ballAngle)*ballSpeed,ball.y+ball.h/2+5*sin(ballAngle)*ballSpeed);
        SDL_RenderDrawLine(gRenderer, (tan(ballAngle) < 0)?ball.x:ball.x+ball.w, ballBottom, ball.x+ball.w/2-5*cos(ballAngle)*ballSpeed,ball.y+ball.h/2+5*sin(ballAngle)*ballSpeed);
        SDL_RenderPresent(gRenderer);
        */
        ///Draws the "tail" of the ball, so it looks kinda like a meteor
        ///if you uncomment this part, comment out the renderpresent of the draw function!
    }
    return 1000/FPS;

}
