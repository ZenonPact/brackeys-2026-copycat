        #include "raylib.h"
        #include "raymath.h"
        #include <algorithm>
        #include <string>
        #include <vector>



        enum class State {Idle, Walking, Mimicking};
        enum class GameState {Playing, Won, Lost, Title};
        const Color PALETTE[] = {PURPLE,BLUE,GREEN,YELLOW,ORANGE,BROWN};
        const char* PALETTE_NAMES [] = {"purple", "blue", "green", "yellow", "orange", "brown"};
        const int SIZE_OF_PALETTE = 6;
        const int SCREEN_WIDTH = 960;
        const int SCREEN_HEIGHT = 540;
        const Vector2 ROOM_MIN = {40, 40};
        const Vector2 ROOM_MAX = {SCREEN_WIDTH - 40.0f, SCREEN_HEIGHT - 40.0f};

        struct Agent
        {
            Vector2 AgentPosition;
            Color AgentColor;
            Vector2 TargetPosition;
            State AgentState;
            float TimeStanding;
            float PauseDuration;
            bool IsMimic;
            int ColorIndex;
            Vector2 noise;
        };

        void ResetRound(std::vector<Agent> &agents, std::vector<Vector2> &playerRecordedDeltaPositions, GameState &gameState, Vector2 &playerPosition, int &currentRound, std::string &hint)
        {
            int agentCount = 0;
            int truthChance = 0;
            agents.clear();
            playerRecordedDeltaPositions.clear();
            playerPosition = Vector2{SCREEN_WIDTH/2.0f, SCREEN_HEIGHT/2.0f};
            agentCount = currentRound + 5;
            int mimicAgentIndex = GetRandomValue(0, agentCount-1);
            truthChance = std::clamp(100 - (currentRound - 3) * 10, 60, 100);
            for(int i = 0; i < agentCount; i++)
            {
                int generatedColorIndex = GetRandomValue(0, SIZE_OF_PALETTE-1);
                Agent newAgent =
                {
                    {(float)GetRandomValue((int)ROOM_MIN.x, (int)ROOM_MAX.x), (float)GetRandomValue((int)ROOM_MIN.y, (int)ROOM_MAX.y)},
                    PALETTE[generatedColorIndex],
                    {(float)GetRandomValue((int)ROOM_MIN.x, (int)ROOM_MAX.x), (float)GetRandomValue((int)ROOM_MIN.y, (int)ROOM_MAX.y)},
                    State::Walking,
                    0.0f,
                    0.0f,
                    false,
                    generatedColorIndex,
                    {0,0}
                };
                agents.push_back(newAgent);
            }

            agents[mimicAgentIndex].IsMimic = true;
            if(GetRandomValue(1, 100) <= truthChance)
            {
                hint = TextFormat("The color of the liar is %s", PALETTE_NAMES[agents[mimicAgentIndex].ColorIndex]);
            }
            else
            {
            int lieIndex = (agents[mimicAgentIndex].ColorIndex + GetRandomValue(1, SIZE_OF_PALETTE-1)) % SIZE_OF_PALETTE;
            hint = TextFormat("The color of the liar is %s", PALETTE_NAMES[lieIndex]);
            }
            gameState = GameState::Playing;
        }

        void DrawGuy(Vector2 position, Color color)
        {
            DrawCircleV(position, 20.0f, color);
            DrawCircleV({position.x + 7, position.y - 5}, 6.0f, WHITE);
            DrawCircleV({position.x - 7, position.y - 5}, 6.0f, WHITE);
            DrawCircleV({position.x + 7, position.y - 5}, 3.0f, BLACK);
            DrawCircleV({position.x - 7, position.y - 5}, 3.0f, BLACK);
        }
        int main()
        {
            std::vector<Vector2> playerRecordedDeltaPositions;
            Vector2 playerStartingPosition;
            Vector2 playerDeltaPosition;
            bool hasHistory;
            Vector2 delayedData;
            bool delayedMoving;
            bool clickedThisAgent;
            int currentRound = 1;
            std::string hint;
            
            InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "CopyCat");
            std::vector<Agent> agents{};
            GameState gameState;

            SetTargetFPS(60);
            Vector2 position;
            Vector2 direction = Vector2{0.0f,0.0f};
            float speed = 200.0f;

            gameState = GameState::Title;

            InitAudioDevice();
            Sound sfxHint =  LoadSound("assets/blipSelect.wav");
            Sound sfxWrong =  LoadSound("assets/hitHurt.wav");
            Sound sfxClear =  LoadSound("assets/powerUp.wav");
            Sound sfxRestart =  LoadSound("assets/restart.wav");

            while(!WindowShouldClose())
            {
                if(gameState == GameState::Playing)
                {
                    playerStartingPosition = position;
                    direction = Vector2{0.0,0.0};
                    
                    if(IsKeyDown(KEY_W))
                    {
                        direction.y += -1;
                    }
                    if(IsKeyDown(KEY_S))
                    {
                        direction.y += 1;
                    }
                    if(IsKeyDown(KEY_D))
                    {
                        direction.x += 1;
                    }
                    if(IsKeyDown(KEY_A))
                    {
                        direction.x += -1;
                    }
                    direction = Vector2Normalize(direction);
                    position = Vector2Clamp(Vector2Add(position, Vector2Scale(direction, speed*GetFrameTime())), ROOM_MIN, ROOM_MAX);

                    playerDeltaPosition = Vector2Subtract(position, playerStartingPosition);
                    playerRecordedDeltaPositions.push_back(playerDeltaPosition);
                    hasHistory = playerRecordedDeltaPositions.size() >= 150;
                    delayedData = hasHistory ? playerRecordedDeltaPositions[playerRecordedDeltaPositions.size() - 150] : Vector2 {0,0};
                    delayedMoving = Vector2Length(delayedData) > 0.5f;

                    for (Agent& agent: agents) 
                        {
                            switch (agent.AgentState) 
                            {
                                case State::Idle:
                                {
                                    agent.TimeStanding += GetFrameTime();
                                    if(agent.TimeStanding >= agent.PauseDuration)
                                    {
                                        agent.AgentState = State::Walking;
                                    }
                                    if(agent.IsMimic && delayedMoving)
                                    {
                                        agent.AgentState = State::Mimicking;
                                    }
                                    break;
                                }
                                case State::Walking:
                                {
                                    Vector2 walkingDirection = Vector2Normalize(Vector2Subtract(agent.TargetPosition, agent.AgentPosition));
                                    agent.AgentPosition = Vector2Clamp((agent.AgentPosition + (Vector2Scale(walkingDirection, GetFrameTime() * 130))),ROOM_MIN, ROOM_MAX);
                                    if(Vector2Distance(agent.AgentPosition, agent.TargetPosition)<= 5.0f)
                                    {
                                        agent.TimeStanding = 0.0f;
                                        agent.TargetPosition = {(float)GetRandomValue((int)ROOM_MIN.x, (int)ROOM_MAX.x), (float)GetRandomValue((int)ROOM_MIN.y, (int)ROOM_MAX.y)};
                                        agent.AgentState = State::Idle;
                                        agent.PauseDuration = (float)GetRandomValue(6, 12)/10;
                                    }
                                    if(agent.IsMimic && delayedMoving)
                                    {
                                        agent.AgentState = State::Mimicking;
                                    }
                                    break;
                                }
                                case State::Mimicking:
                                {
                                    agent.noise = Vector2Scale(agent.noise, 0.9f);
                                    agent.noise += {GetRandomValue(-15, 15)/100.0f,GetRandomValue(-15, 15)/100.0f };
                                    agent.noise = Vector2ClampValue(agent.noise, 0, 1.2f);
                                    agent.AgentPosition = Vector2Clamp(agent.AgentPosition + delayedData + agent.noise, ROOM_MIN, ROOM_MAX);
                                    if(!delayedMoving)
                                    {
                                        agent.TimeStanding = 0.0f;
                                        agent.AgentState = State::Idle;
                                    }
                                    break;
                                }
                            }
                            if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
                            {
                                clickedThisAgent = CheckCollisionPointCircle(GetMousePosition(), agent.AgentPosition, 25.0f);
                                if(clickedThisAgent)
                                {
                                    if(agent.IsMimic)
                                    {
                                        currentRound += 1;
                                        PlaySound(sfxClear);
                                        ResetRound(agents, playerRecordedDeltaPositions, gameState, position, currentRound, hint);
                                    }
                                    else 
                                    {
                                        PlaySound(sfxWrong);
                                        gameState = GameState::Lost;
                                    }
                                }
                            }
                        }
                    BeginDrawing();
                        ClearBackground(LIGHTGRAY);
                        for (Agent& agent: agents) 
                        {
                            DrawGuy(agent.AgentPosition, agent.AgentColor);
                        }
                        DrawGuy(position, RED);
                        DrawRectangle(position.x - 40/2.0f, position.y -20, 40 , 10, BLACK);
                        DrawRectangle(position.x - 30/2.0f, position.y -35, 30 , 15, BLACK);
                        DrawRectangle(position.x - 7, position.y + 4, 14 , 5, BLACK);
                        // DrawRectangle(position.x - 8, position.y + 10, 16 , 2, BLACK);
                        DrawRing({position.x + 7, position.y - 5}, 6, 9, 0, 360, 32, BLACK);
                        //DrawRing({position.x, position.y +6}, 8, 10, 0, 180, 16, BLACK);
                        DrawText(TextFormat("Round %d", currentRound), 12, 12, 48, BLACK);
                        DrawText("Keep pressing H for hint:", 12, SCREEN_HEIGHT - 48 , 24, BLACK);
                        if(IsKeyDown(KEY_H))
                        {
                            DrawRectangleLines(SCREEN_WIDTH - 34 - MeasureText(hint.c_str(),24), SCREEN_HEIGHT - 53, MeasureText(hint.c_str(), 24) + 30, 34, BLACK);
                            DrawText(hint.c_str(), SCREEN_WIDTH - 24 - MeasureText(hint.c_str(), 24), SCREEN_HEIGHT - 48  , 24, WHITE);
                        }
                        if(IsKeyPressed(KEY_H))
                        {
                            PlaySound(sfxHint);
                        }
                    EndDrawing();   
                }
                else if (gameState == GameState::Lost)
                {
                    if(IsKeyPressed(KEY_R))
                    {   
                        currentRound = 1;
                        PlaySound(sfxRestart);
                        ResetRound(agents, playerRecordedDeltaPositions, gameState, position, currentRound, hint);
                    }
                    BeginDrawing();
                        ClearBackground(LIGHTGRAY);
                        DrawText("You lose, press R to reset", SCREEN_WIDTH/2.0 - MeasureText("You lose, press R to reset", 48)/2.0f, SCREEN_HEIGHT/2.0f - 48/2.0f, 48, RED);
                    EndDrawing();
                }
                else if (gameState == GameState::Title)
                {
                    if (IsKeyPressed(KEY_SPACE)) 
                    {
                        PlaySound(sfxRestart);
                        ResetRound(agents, playerRecordedDeltaPositions, gameState, position, currentRound, hint);
                    }                    
                    BeginDrawing();
                        ClearBackground(LIGHTGRAY);
                        //TextFormat("COPYCAT \n", "Press WASD to move \n", "Find the CopyCat \n", "SPACE to start");
                        DrawText("COPYCAT", SCREEN_WIDTH/2.0 - MeasureText("COPYCAT", 64)/2.0f, SCREEN_HEIGHT/2.0 - 64/2.0f, 64.0f, BLACK);
                        DrawText("Press WASD to Move", SCREEN_WIDTH/2.0 - MeasureText("Press WASD to Move", 32)/2.0f, SCREEN_HEIGHT/2.0 + 80 - 32/2.0f, 32.0f, BLACK);
                        DrawText("Left Click to accuse the Copycat", SCREEN_WIDTH/2.0 - MeasureText("Left Click to accuse the Copycat", 32)/2.0f, SCREEN_HEIGHT/2.0 + 120 - 32/2.0f, 32.0f, BLACK);
                        DrawText("Press SPACE to Start", SCREEN_WIDTH/2.0 - MeasureText("Press SPACE to Start", 32)/2.0f, SCREEN_HEIGHT/2.0 + 160 - 32/2.0f, 32.0f, BLACK);
                        EndDrawing();

                }
            }

            CloseWindow();
                
            return 0;
        }
