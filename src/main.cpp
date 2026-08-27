        #include "raylib.h"
        #include "raymath.h"
#include <algorithm>
        #include <string>
        #include <vector>



        enum class State {Idle, Walking, Mimicking};
        enum class GameState {Playing, Won, Lost};
        const Color Palette[] = {PURPLE,BLUE,GREEN,YELLOW,ORANGE,BROWN};
        const char* PaletteNames [] = {"purple", "blue", "green", "yellow", "orange", "brown"};
        const int SIZE_OF_PALETTE = 6;
        const int SCREEN_WIDTH = 960;
        const int SCREEN_HEIGHT = 540;

        struct Agent
        {
            Vector2 AgentPosition;
            Color AgentColor;
            Vector2 TargetPosition;
            State AgentState;
            float TimeStanding;
            float PauseDuration;
            bool IsMimic;
            int colorIndex;
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
                Agent NewAgent =
                {
                    {(float)GetRandomValue(0, SCREEN_WIDTH), (float)GetRandomValue(0, SCREEN_HEIGHT)},
                    Palette[generatedColorIndex],
                    Vector2Clamp(Vector2{(float)GetRandomValue(0, SCREEN_WIDTH), (float)GetRandomValue(0, SCREEN_HEIGHT)}, Vector2{0.0f,0.0f}, Vector2{SCREEN_WIDTH, SCREEN_HEIGHT}),
                    State::Walking,
                    0.0f,
                    0.0f,
                    false
                };
                NewAgent.colorIndex = generatedColorIndex;
                agents.push_back(NewAgent);
            }

            agents[mimicAgentIndex].IsMimic = true;
            if(GetRandomValue(1, 100) <= truthChance)
            {
                hint = TextFormat("The color of the liar is %s", PaletteNames[agents[mimicAgentIndex].colorIndex]);
            }
            else
            {
               int lieIndex = (agents[mimicAgentIndex].colorIndex + GetRandomValue(1, SIZE_OF_PALETTE-1)) % SIZE_OF_PALETTE;
               hint = TextFormat("The color of the liar is %s", PaletteNames[lieIndex]);
            }
            gameState = GameState::Playing;
        }

        int main()
        {
            std::vector<Vector2> PlayerRecordedDeltaPositions;
            Vector2 PlayerStartingPosition;
            Vector2 PlayerDeltaPosition;
            bool hasHistory;
            Vector2 delayedData;
            bool delayedMoving;
            bool clickedThisAgent;
            int currentRound = 1;
            std::string hint;
            
            InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "CopyCat");
            std::vector<Agent> Agents{};
            GameState gameState;

            SetTargetFPS(60);
            Vector2 position;
            Vector2 direction = Vector2{0.0f,0.0f};
            float speed = 200.0f;
            
            ResetRound(Agents, PlayerRecordedDeltaPositions, gameState, position, currentRound, hint);

            while(!WindowShouldClose())
            {
                if(gameState == GameState::Playing)
                {
                    PlayerStartingPosition = position;
                    direction = Vector2{0.0,0.0};
                    
                    if(IsKeyDown(KEY_UP))
                    {
                        direction.y += -1;
                    }
                    if(IsKeyDown(KEY_DOWN))
                    {
                        direction.y += 1;
                    }
                    if(IsKeyDown(KEY_RIGHT))
                    {
                        direction.x += 1;
                    }
                    if(IsKeyDown(KEY_LEFT))
                    {
                        direction.x += -1;
                    }
                    direction = Vector2Normalize(direction);
                    position = Vector2Add(position, Vector2Scale(direction, speed*GetFrameTime()));

                    PlayerDeltaPosition = Vector2Subtract(position, PlayerStartingPosition);
                    PlayerRecordedDeltaPositions.push_back(PlayerDeltaPosition);
                    hasHistory = PlayerRecordedDeltaPositions.size() >= 90;
                    delayedData = hasHistory ? PlayerRecordedDeltaPositions[PlayerRecordedDeltaPositions.size() - 90] : Vector2 {0,0};
                    delayedMoving = Vector2Length(delayedData) > 0.5f;

                    for (Agent& agent: Agents) 
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
                                    Vector2 WalkingDirection = Vector2Normalize(Vector2Subtract(agent.TargetPosition, agent.AgentPosition));
                                    agent.AgentPosition += Vector2Scale(WalkingDirection, GetFrameTime() * 130);
                                    if(Vector2Distance(agent.AgentPosition, agent.TargetPosition)<= 5.0f)
                                    {
                                        agent.TimeStanding = 0.0f;
                                        agent.TargetPosition = Vector2Clamp(agent.AgentPosition + Vector2{(float)GetRandomValue(-450, 450), (float)GetRandomValue(-250, 250)}, Vector2{0.0f,0.0f}, Vector2{SCREEN_WIDTH, SCREEN_HEIGHT});
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
                                    agent.AgentPosition += delayedData;
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
                                        //gameState = GameState::Won;
                                        ResetRound(Agents, PlayerRecordedDeltaPositions, gameState, position, currentRound, hint);
                                    }
                                    else 
                                    {
                                        gameState = GameState::Lost;
                                    }
                                }
                            }
                        }
                }
                else 
                {
                    if(IsKeyPressed(KEY_R))
                    {   
                        currentRound = 1;
                        ResetRound(Agents, PlayerRecordedDeltaPositions, gameState, position, currentRound, hint);
                    }
                }

                BeginDrawing();
                    ClearBackground(LIGHTGRAY);
                    for (Agent& agent: Agents) 
                    {
                        DrawCircleV(agent.AgentPosition, 20.0f, agent.AgentColor);
                    }
                    //if(gameState == GameState::Won)
                        //{
                            //DrawText("You Won!!", SCREEN_WIDTH/2, SCREEN_HEIGHT/2, 48, GREEN);
                        //}
                    if(gameState == GameState::Lost)
                    {
                        DrawText("You lose, press R to reset", SCREEN_WIDTH/2, SCREEN_HEIGHT/2, 48, RED);
                    }
                    DrawCircleV(position, 20.0f, RED);
                    DrawText(TextFormat("Round %d", currentRound), 12, 12, 48, BLACK);
                    DrawText("Keep pressing H for hint", 12, SCREEN_HEIGHT - 48 , 24, BLACK);
                    if(IsKeyDown(KEY_H))
                    {
                        DrawText(hint.c_str(), SCREEN_WIDTH/2.0f, SCREEN_HEIGHT/2.0f , 24, BLACK);
                    }
                EndDrawing();
            }

            CloseWindow();
                
            return 0;
        }
